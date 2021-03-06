#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "include/settings.h"
#include "include/stack.h"

#ifndef STACK_HASH
    #define STACK_HASH  1
#endif

#ifndef STACK_CANARY
    #define STACK_CANARY  1
#endif

#ifndef stackReallocCoefficient
    #define stackReallocCoefficient 2
#endif

#ifndef POISON
    #define POISON  -663
#endif

#if defined(STACK_HASH) && STACK_HASH == 1
    #include "libs/hash/include/hash.h"
#endif

#ifdef STACK_ELEMENT_DUMP_FMT_STR
    #undef STACK_ELEMENT_DUMP_FMT_STR
#endif
#define STACK_ELEMENT_DUMP_FMT_STR  "%lf"

#if defined(STACK_CANARY) && STACK_CANARY == 1  
    
    EXIT_CODES canaryCtor(stack_t *stack, int stack_capacity)
    {
        // Error check
        if (stack == NULL)
        {
            PRINT_ERROR_TRACING_MESSAGE(STACK_EXIT_CODES::PASSED_STACK_IS_NULLPTR);
            return EXIT_CODES::BAD_OBJECT_PASSED;
        }

        if (stack->capacity != -1 || stack->size != -1)  // Only just created stack object can call canaryCtor
        {
            PRINT_ERROR_TRACING_MESSAGE(STACK_EXIT_CODES::OLD_STACK_PASSED);
            return EXIT_CODES::BAD_OBJECT_PASSED;
        }

        // Canary construction
        int *canaryLeft = (int *) stack->data;
        *canaryLeft = CANARY_VALUE;

        int *canaryRight = (int *) (((char *) stack->data) + sizeof(stack->canaryLeft) + stack_capacity * sizeof(stackElem_t));
        *canaryRight = CANARY_VALUE;

        stack->data = (stackElem_t *) (((char *) stack->data) + sizeof(stack->canaryLeft));

        return EXIT_CODES::NO_ERRORS;
    }

    EXIT_CODES canaryCheck(stack_t *stack, bool *result)
    {
        // Error check
        if (!stackBasicCheck(stack) || result == NULL)
        {
            *result = false;

            PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::BAD_OBJECT_PASSED);
            return EXIT_CODES::BAD_OBJECT_PASSED;
        }

        // Stack structure canary check
        if (stack->canaryLeft != CANARY_VALUE || stack->canaryRight != CANARY_VALUE)
        {
            *result = false;

            PRINT_ERROR_TRACING_MESSAGE(STACK_EXIT_CODES::STACK_STRUCTURE_CANARY_IS_DAMAGED);
            return EXIT_CODES::BAD_OBJECT_PASSED;
        }

        // Stack data canary check
        int *canaryLeft  = (int *) ( ((char *) stack->data) - sizeof(stack->canaryLeft));
        int *canaryRight = (int *) ( ((char *) stack->data) + stack->capacity * sizeof(stackElem_t));
        if ( *canaryLeft != CANARY_VALUE || *canaryRight != CANARY_VALUE)
        {
            *result = false;

            PRINT_ERROR_TRACING_MESSAGE(STACK_EXIT_CODES::STACK_DATA_CANARY_IS_DAMAGED);
            return EXIT_CODES::BAD_OBJECT_PASSED;
        }

        *result = true;
        return EXIT_CODES::NO_ERRORS;
    }
    
#endif

#if defined(STACK_HASH) && STACK_HASH == 1

    EXIT_CODES calculateStackHashSum(stack_t *stack, long long int *hashSum)
    {
        // Error check
        if (!stackBasicCheck(stack))
        {
            PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::BAD_OBJECT_PASSED);
            return EXIT_CODES::BAD_OBJECT_PASSED;
        }

        // Calculation of hash of stack structure
        long long int stack_hash_sum = 0;
        unsigned long long stack_total_bytes = sizeof(stack_t) - sizeof(stack->hashSum);
        IS_OK_WO_EXIT(calculateHashSum(stack, stack_total_bytes, &stack_hash_sum));

        // Calculation of data hash of the stack structure 
        long long int stack_data_hash_sum = 0;

        long long int data_total_bytes = stack->capacity * sizeof(stackElem_t);
        #if defined(STACK_CANARY) && STACK_CANARY == 1
            data_total_bytes += ( sizeof(stack->canaryLeft) + sizeof(stack->canaryRight) );

            char *data_p = ((char *) stack->data) - sizeof(stack->canaryLeft);
        #else
            char *data_p = (char *) stack->data;
        #endif

        IS_OK_WO_EXIT(calculateHashSum(data_p, data_total_bytes, &stack_data_hash_sum));
        
        // Get entire stack + stack->data hash sum
        *hashSum = stack_hash_sum + stack_data_hash_sum;

        return EXIT_CODES::NO_ERRORS;
    }

    EXIT_CODES stackHashCheck(stack_t *stack, bool *result)
    {
        // Error check
        if (!stackBasicCheck(stack) || result == NULL)
        {
            *result = false;

            PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::BAD_OBJECT_PASSED);
            return EXIT_CODES::BAD_OBJECT_PASSED;
        }

        // Hash sum check
        long long int currentHashSum = 0;
        IS_OK_WO_EXIT(calculateStackHashSum(stack, &currentHashSum));

        if (stack->hashSum != currentHashSum)
        {
            *result = false;

            PRINT_ERROR_TRACING_MESSAGE(STACK_EXIT_CODES::STACK_HASH_SUM_IS_DAMAGED);
            return EXIT_CODES::BAD_OBJECT_PASSED;
        }

        *result = true;
        return EXIT_CODES::NO_ERRORS;
    }

#endif

bool stackBasicCheck(stack_t *stack)
{
    return  stack != NULL && stack->data != NULL &&
            stack->size >= 0 && stack->size <= stack->capacity;
}

bool stackOk(stack_t *stack)
{
    bool result = stackBasicCheck(stack);

    #if defined(STACK_CANARY) && STACK_CANARY == 1  
        bool canary_is_ok = true;
        IS_OK_WO_EXIT(canaryCheck(stack, &canary_is_ok));

        result = result && canary_is_ok;
    #endif 

    #if defined(STACK_HASH) && STACK_HASH == 1  
        bool hash_is_ok = true;
        IS_OK_WO_EXIT(stackHashCheck(stack, &hash_is_ok));

        result = result && hash_is_ok;
    #endif

    return result;
}

EXIT_CODES sprayPoisonOnData(stack_t *stack)
{
    // Error check
    if (!stackBasicCheck(stack))
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::BAD_OBJECT_PASSED);
        return EXIT_CODES::BAD_OBJECT_PASSED;
    }

    for (int i = stack->size; i < stack->capacity; ++i)
    {
        stack->data[i] = POISON;
    }

    return EXIT_CODES::NO_ERRORS;
}

#if STACK_CANARY == 1 || STACK_HASH == 1

    EXIT_CODES stackCapacityIncrease(stack_t *stack, int *add_bytes)
    {
        // Error check
        if (stack == NULL || add_bytes == NULL)
        {
            PRINT_ERROR_TRACING_MESSAGE(STACK_EXIT_CODES::PASSED_STACK_IS_NULLPTR);
            return EXIT_CODES::BAD_OBJECT_PASSED;
        }

        // Protection bytes for hashSum field
        #if defined(STACK_HASH) && STACK_HASH == 1
            (*add_bytes) += sizeof(stack->hashSum);
        #endif
        
        // Protection bytes for canary fields
        #if defined(STACK_CANARY) && STACK_CANARY == 1
            (*add_bytes) += ( sizeof(stack->canaryLeft) + sizeof(stack->canaryRight) );
        #endif

        return EXIT_CODES::NO_ERRORS;
    }

#endif

// TODO: change struct default values to macroses
EXIT_CODES stackCtor(stack_t *stack, int stack_capacity)
{
    // Error check
    if (stack == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(STACK_EXIT_CODES::PASSED_STACK_IS_NULLPTR);
        return EXIT_CODES::BAD_OBJECT_PASSED;
    }

    if (stack->data != NULL || stack->capacity != -1 || stack->size != -1)
    {
        PRINT_ERROR_TRACING_MESSAGE(STACK_EXIT_CODES::OLD_STACK_PASSED);
        return EXIT_CODES::BAD_OBJECT_PASSED;
    }

    // Count additional bytes needed for security fields
    int stack_capacity_increase = 0;
    IS_OK_W_EXIT(stackCapacityIncrease(stack, &stack_capacity_increase));
    
    // Memory allocation (for stack elements)
    stack->data = (stackElem_t *) calloc(1, stack_capacity * sizeof(stackElem_t) + stack_capacity_increase);
    if (stack->data == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::BAD_STD_FUNC_RESULT);
        return EXIT_CODES::BAD_STD_FUNC_RESULT;
    }

    // Construct canary fields
    #if defined(STACK_CANARY) && STACK_CANARY == 1
        IS_OK_W_EXIT(canaryCtor(stack, stack_capacity));
    #endif

    // Fill stack structure
    stack->capacity = stack_capacity;
    stack->size = 0;

    // Poison allocated data fields
    IS_OK_W_EXIT(sprayPoisonOnData(stack));

    // Construct hasSum field
    #if defined(STACK_HASH) && STACK_HASH == 1
        IS_OK_W_EXIT(calculateStackHashSum(stack, &stack->hashSum));
    #endif

    return EXIT_CODES::NO_ERRORS;
}

EXIT_CODES getNewReallocationCapacity(stack_t *stack, REALLOC_MODES mode, int *new_capacity)
{
    // Error check
    OBJECT_VERIFY(stack, stack);

    if (new_capacity == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::BAD_OBJECT_PASSED);
        return EXIT_CODES::BAD_OBJECT_PASSED;
    }

    switch (mode)
    {
        case REALLOC_MODES::DECREASE:
            *new_capacity = stack->capacity / stackReallocCoefficient;
            break;
        case REALLOC_MODES::INCREASE:
            *new_capacity = stack->capacity * stackReallocCoefficient;
            break;
        default:
            return EXIT_CODES::BAD_OBJECT_PASSED;
    }

    // Error check
    OBJECT_VERIFY(stack, stack);

    return EXIT_CODES::NO_ERRORS;
}

EXIT_CODES stackReallocation(stack_t *stack, REALLOC_MODES mode)
{
    // Error check
    OBJECT_VERIFY(stack, stack);

    // Get new reallocation capacity
    int new_capacity = 0;
    IS_OK_W_EXIT(getNewReallocationCapacity(stack, mode, &new_capacity));

    int stack_capacity_increase = 0;
    #if defined(STACK_CANARY) && STACK_CANARY == 1
        stack_capacity_increase += ( sizeof(stack->canaryLeft) + sizeof(stack->canaryRight) );
    #endif

    #if defined(STACK_CANARY) && STACK_CANARY == 1
        stackElem_t *temp = (stackElem_t *) realloc( ((char *) stack->data) - sizeof(stack->canaryLeft),  // because we want to free canaries (left and right)
                                                    sizeof(stack->canaryLeft) + new_capacity * sizeof(stackElem_t) + sizeof(stack->canaryRight));
    #else
        stackElem_t *temp = (stackElem_t *) realloc(stack->data, new_capacity * sizeof(stackElem_t));
    #endif
    
    if (temp == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::BAD_STD_FUNC_RESULT);
        return EXIT_CODES::BAD_STD_FUNC_RESULT;
    }

    #if defined(STACK_CANARY) && STACK_CANARY == 1
        // Set canaries
        int *canaryLeft = (int *) temp;
        *canaryLeft = CANARY_VALUE;

        int *canaryRight = (int *) (((char *) temp) + sizeof(stack->canaryLeft) + new_capacity * sizeof(stackElem_t));
        *canaryRight = CANARY_VALUE;

        temp = (stackElem_t *) (((char *) temp) + sizeof(stack->canaryLeft));
    #endif

    // Update structure variables
    stack->data = temp;
    stack->capacity = new_capacity;

    // Poison new fields
    IS_OK_W_EXIT(sprayPoisonOnData(stack));

    // Update hash sum
    #if defined(STACK_HASH) && STACK_HASH == 1
        IS_OK_W_EXIT(calculateStackHashSum(stack, &stack->hashSum));
    #endif

    // Error check
    OBJECT_VERIFY(stack, stack);

    return EXIT_CODES::NO_ERRORS;
}

EXIT_CODES stackPush(stack_t *stack, stackElem_t value)
{
    // Error check
    OBJECT_VERIFY(stack, stack);

    if (stack->size == stack->capacity)
    {
        IS_OK_W_EXIT(stackReallocation(stack, REALLOC_MODES::INCREASE));
    }

    stack->data[stack->size++] = value;

    // Update hash sum
    #if defined(STACK_HASH) && STACK_HASH == 1
        IS_OK_W_EXIT(calculateStackHashSum(stack, &stack->hashSum));
    #endif

    // Error check
    OBJECT_VERIFY(stack, stack);

    return EXIT_CODES::NO_ERRORS;
}

EXIT_CODES stackPop(stack_t *stack, stackElem_t *popTo)
{
    // Error check
    OBJECT_VERIFY(stack, stack);

    if (stack->size < 1)
    {
        PRINT_ERROR_TRACING_MESSAGE(STACK_EXIT_CODES::BAD_STACK_SIZE);
        return EXIT_CODES::BAD_OBJECT_PASSED;
    }

    if (popTo == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::BAD_OBJECT_PASSED);
        return EXIT_CODES::BAD_OBJECT_PASSED;
    }

    // printf("stackReallocCoefficient * stack->size: %d\n", stackReallocCoefficient * stack->size);
    // printf("stack->capacity: %d\n", stack->capacity);
    if (stack->size > 8 && stackReallocCoefficient * stack->size < stack->capacity)
    {
        IS_OK_W_EXIT(stackReallocation(stack, REALLOC_MODES::DECREASE));
    }

    if (popTo == DEFAULT_POPTO_VALUE)
    {
        --stack->size; 
    }
    else
    {
        *popTo = stack->data[--stack->size];
    }

    stack->data[stack->size] = POISON;

    // Update hash sum
    #if defined(STACK_HASH) && STACK_HASH == 1
        IS_OK_W_EXIT(calculateStackHashSum(stack, &stack->hashSum));
    #endif

    // Error check
    OBJECT_VERIFY(stack, stack);

    return EXIT_CODES::NO_ERRORS;
}

#if defined(DEBUG_LEVEL) && DEBUG_LEVEL == 2
    
    EXIT_CODES stackDump(stack_t *stack)
    {
        if (stack == NULL)
        {
            PRINT_ERROR_TRACING_MESSAGE(STACK_EXIT_CODES::PASSED_STACK_IS_NULLPTR);
            return EXIT_CODES::BAD_OBJECT_PASSED;
        }

        fprintf(DEFAULT_ERROR_TRACING_STREAM, YELLOW"[DEBUG_INFO, %s %s]"RESET":\n", __DATE__, __TIME__);
        fprintf(DEFAULT_ERROR_TRACING_STREAM, "stack<%s> [0x%p] %s ", STRINGIFY(stackElem_t), stack, STRINGIFY(stack));
        fprintf(DEFAULT_ERROR_TRACING_STREAM, "from %s(%d), %s():\n", __FILE__, __LINE__, __func__);

        fprintf(DEFAULT_ERROR_TRACING_STREAM, "{\n");

        #if defined(STACK_CANARY) && STACK_CANARY == 1
            fprintf(DEFAULT_ERROR_TRACING_STREAM, "\t\tCANARY = %d\n", stack->canaryLeft);
        #endif

        fprintf(DEFAULT_ERROR_TRACING_STREAM, "\t\tcapacity = %d (%s)\n", stack->capacity, VALUE_CODE_TO_STR(stack->capacity >= 0));
        fprintf(DEFAULT_ERROR_TRACING_STREAM, "\t\tsize = %d (%s)\n",
                stack->size, VALUE_CODE_TO_STR(stack->size <= stack->capacity && stack->size >= 0));
        fprintf(DEFAULT_ERROR_TRACING_STREAM, "\t\tdata[0x%p]\n", stack->data);

        if (stack->size > 0)
        {
            fprintf(DEFAULT_ERROR_TRACING_STREAM, "\t\t{\n");

            #if defined(STACK_CANARY) && STACK_CANARY == 1   
                fprintf(DEFAULT_ERROR_TRACING_STREAM, "\t\t\t [-1] = %d (CANARY)\n", *((int * ) (((char *) stack->data) - sizeof(stack->canaryLeft))));
            #endif

            for (int element = 0; element < 12; ++element)
            {
                if ( fabs((double) stack->data[element] - POISON) > __DBL_EPSILON__)
                {
                    fprintf(DEFAULT_ERROR_TRACING_STREAM, "\t\t\t *[%d] = " STACK_ELEMENT_DUMP_FMT_STR "\n", element, (double) stack->data[element]);
                }
                else
                {
                    fprintf(DEFAULT_ERROR_TRACING_STREAM, "\t\t\t [%d] = " STACK_ELEMENT_DUMP_FMT_STR " (NOT USED)\n", element, (double) stack->data[element]);
                }
            }
            fprintf(DEFAULT_ERROR_TRACING_STREAM, "\t\t\t ...\n");

            #if defined(STACK_CANARY) && STACK_CANARY == 1
                fprintf(DEFAULT_ERROR_TRACING_STREAM, "\t\t\t [%d] = %d (CANARY)\n", stack->capacity, *((int *) (((char *) stack->data) + stack->capacity * sizeof(stackElem_t))));//stack->data[stack->capacity]);
            #endif

            fprintf(DEFAULT_ERROR_TRACING_STREAM, "\t\t}\n");
        }

        #if defined(STACK_CANARY) && STACK_CANARY == 1
            fprintf(DEFAULT_ERROR_TRACING_STREAM, "\t\tCANARY = %d \n", stack->canaryRight);
        #endif

        #if defined(STACK_HASH) && STACK_HASH == 1
            fprintf(DEFAULT_ERROR_TRACING_STREAM, "\t\tHASH_SUM = %lld \n", stack->hashSum);
        #endif

        fprintf(DEFAULT_ERROR_TRACING_STREAM, "}\n");

        return EXIT_CODES::NO_ERRORS;
    }
    
#endif

EXIT_CODES stackDtor(stack_t *stack)
{
    // Error check
    OBJECT_VERIFY(stack, stack);

    #if defined(STACK_CANARY) && STACK_CANARY == 1
        free( (((char *) stack->data) - sizeof(stack->canaryLeft)) );
    #else
        free(stack->data);
    #endif

    stack->data = NULL;
    stack->capacity = -1;
    stack->size = -1;

    return EXIT_CODES::NO_ERRORS;
}
