#include <math.h> // for fabs

#include "libs/colors/colors.h"
#include "libs/stack/include/stack.h"
#include "libs/text/include/text.h"

#include "include/processor/processor.h"
#include "include/processor/settings.h"

EXIT_CODES cpuCtor(cpu_t *CPU)
{
    // Error check
    if (CPU == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Stack init
    IS_ERROR(stackCtor(&CPU->stack))
    {
        PRINT_ERROR_TRACING_MESSAGE(STACK_EXIT_CODES::BAD_STACK_PASSED);
        return EXIT_CODES::BAD_OBJECT_PASSED;
    }

    // RAM init
    CPU->RAM = (double *) calloc(MAX_RAM_SIZE, sizeof(double));
    CHECK_CALLOC_RESULT(CPU->RAM);

    // VRAM init
    CPU->VRAM = (byte *) calloc(MAX_VRAM_SIZE, sizeof(byte));
    CHECK_CALLOC_RESULT(CPU->VRAM);

    return EXIT_CODES::NO_ERRORS;
}

EXIT_CODES cpuDtor(cpu_t *CPU)
{
    // Error check
    if (CPU == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Stack destruction
    IS_ERROR(stackDtor(&CPU->stack))
    {
        PRINT_ERROR_TRACING_MESSAGE(STACK_EXIT_CODES::BAD_STACK_PASSED);
        return EXIT_CODES::BAD_OBJECT_PASSED;
    }

    // RAM destruction
    free(CPU->RAM);

    // VRAM destruction
    free(CPU->VRAM);

    return EXIT_CODES::NO_ERRORS;
}

EXIT_CODES cpuDump(cpu_t *CPU, text_t *byteCode)
{
    // Error check
    if (CPU == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Dump registers
    for (int reg = 0; reg < MAX_REGS_COUNT; ++reg)
    {
        printf(GREEN "$%cX" RESET ": %lf\n", 'A' + reg, CPU->commonRegs[reg]);
    }
    printf(RED "$ip" RESET ": %d -> 0x%x\n\n", CPU->ip, (byte) byteCode->data[CPU->ip]);

    // Dump stack
    IS_ERROR(stackDump(&CPU->stack))
    {
        PRINT_ERROR_TRACING_MESSAGE(PROCESSOR_EXIT_CODES::ERROR_DUMPING_PROCESSOR_STACK);
        return EXIT_CODES::BAD_OBJECT_PASSED;
    }
    putchar('\n');

    // Dump RAM
    for (int cell = 0; cell < RAM_CELLS_TO_DUMP; ++cell)
    {
        printf("[%lf]", CPU->RAM[cell]);
    }
    printf("\n\n");

    return EXIT_CODES::NO_ERRORS;
}

static void cpuExit(cpu_t *CPU, text_t *byteCode, int exitCode)
{
    // Error check
    if (CPU == NULL || byteCode == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        exit(EXIT_FAILURE);
    }

    // Exit
    textDtor(byteCode);
    IS_ERROR(cpuDtor(CPU))
    {
        PRINT_ERROR_TRACING_MESSAGE(PROCESSOR_EXIT_CODES::FAIL_DURING_CPU_DTOR_IN_HALT);
        exit(EXIT_FAILURE);
    }

    exit(exitCode);
}

static EXIT_CODES _getRegisterValue(cpu_t *CPU, text_t *byteCode, double *result)
{
    // Error check
    if (CPU == NULL || byteCode == NULL || result == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Get register value
    byte regCode = (byte) byteCode->data[CPU->ip];
    *result = CPU->commonRegs[regCode];

    return EXIT_CODES::NO_ERRORS;
}

static double getRegisterValue(cpu_t *CPU, text_t *byteCode)
{
    // Error check
    if (CPU == NULL || byteCode == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return BAD_DOUBLE_VALUE;
    }

    // Get register value
    double result = 0;
    IS_ERROR(_getRegisterValue(CPU, byteCode, &result))
    {
        PRINT_ERROR_TRACING_MESSAGE(PROCESSOR_EXIT_CODES::FAIL_DURING_TAKING_REGISTER_VALUE);
        return BAD_DOUBLE_VALUE;
    }

    return result;
}

static EXIT_CODES _getImmediateValue(cpu_t *CPU, text_t *byteCode, double *result)
{
    // Error check
    if (CPU == NULL || byteCode == NULL || result == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Get immediate (double) value
    *result = *((double *) &byteCode->data[CPU->ip]);

    return EXIT_CODES::NO_ERRORS;
}

static double getImmediateValue(cpu_t *CPU, text_t *byteCode)
{
    // Error check
    if (CPU == NULL || byteCode == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return BAD_DOUBLE_VALUE;
    }

    // Get immediate (double) value
    double result = 0;
    IS_ERROR(_getImmediateValue(CPU, byteCode, &result))
    {
        PRINT_ERROR_TRACING_MESSAGE(PROCESSOR_EXIT_CODES::FAIL_DURING_TAKING_IMMEDIATE_VALUE);
        return BAD_DOUBLE_VALUE;
    }

    return result;
}

static EXIT_CODES __cpuCountInternalExpressionValue(cpu_t *CPU, text_t *byteCode, size_t argc, double *result)
{
    // Error check
    if (CPU == NULL || byteCode == NULL || result == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Count expression value
    *result = 0;
    for (size_t arg = 0; arg < argc; ++arg)
    {
        // Get bytecode double value
        if (MRI_IS_REGISTER(byteCode->data[CPU->ip]))  // CPU->ip is pointing to byte after globalMRI
        {
            ++CPU->ip;   

            *result += getRegisterValue(CPU, byteCode);

            CPU->ip += sizeof(byte);
        }
        else if (MRI_IS_IMMEDIATE(byteCode->data[CPU->ip]))
        {
            ++CPU->ip;
            
            *result += getImmediateValue(CPU, byteCode);

            CPU->ip += sizeof(double);
        }
        else
        {
            PRINT_ERROR_TRACING_MESSAGE(PROCESSOR_EXIT_CODES::UNKNOWN_INSTRUCTION_TYPE_ARG);
            return EXIT_CODES::BAD_OBJECT_PASSED;
        }
    }

    return EXIT_CODES::NO_ERRORS;
}

static EXIT_CODES _cpuGetBytecodeValue(cpu_t *CPU, text_t *byteCode, double *result)
{
    // Error check
    if (CPU == NULL || byteCode == NULL || result == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Get metainfo
    size_t argc     = (size_t)  GET_TOTAL_ARGS(byteCode->data[CPU->ip]);
    int globalMRI   = (int)     GET_GLOBAL_MRI(byteCode->data[CPU->ip++]);
    IS_ERROR(__cpuCountInternalExpressionValue(CPU, byteCode, argc, result))
    {
        PRINT_ERROR_TRACING_MESSAGE(PROCESSOR_EXIT_CODES::ERROR_COUNTING_INTERNAL_EXPRESSION_VALUE);
        return EXIT_CODES::BAD_OBJECT_PASSED;
    }

    // Check global MRI <-> Memory, Register, Immediate (actually for globalMRI using only memory)
    if (MRI_IS_MEMORY(globalMRI))
    {
        // TODO: check all RAM index etc. && maybe do separate function
        if (fabs(*result - fabs((int) *result)) < EPS)
        {
            *result = CPU->RAM[(int) *result];  
        }
        else
        {
            PRINT_ERROR_TRACING_MESSAGE(PROCESSOR_EXIT_CODES::BAD_TO_ACCESS_RAM_INDEX);
            return EXIT_CODES::BAD_OBJECT_PASSED;
        }
    }

    return EXIT_CODES::NO_ERRORS;
}

static double cpuGetBytecodeValue(cpu_t *CPU, text_t *byteCode)
{
    // Error check
    if (CPU == NULL || byteCode == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return BAD_DOUBLE_VALUE;
    }

    //TODO: EXIT_PANIC
    // Get bytecode double value (calculate expression if there is one)
    double result = DEFAULT_DOUBLE_VALUE;
    IS_ERROR(_cpuGetBytecodeValue(CPU, byteCode, &result))
    {
        // TODO: processor error flag that determines the stay of processor
        PRINT_ERROR_TRACING_MESSAGE(PROCESSOR_EXIT_CODES::ERROR_READING_BYTECODE_DOUBLE_VALUE);
        return BAD_DOUBLE_VALUE;
    }

    return result;
}

static EXIT_CODES _cpuPop(cpu_t *CPU, double *result)
{
    // Error check
    if (CPU == NULL || result == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Pop
    IS_ERROR(stackPop(&CPU->stack, result))
    {
        PRINT_ERROR_TRACING_MESSAGE(PROCESSOR_EXIT_CODES::ERROR_POPPING_VALUE_FROM_STACK);
        return EXIT_CODES::BAD_OBJECT_PASSED;
    }

    return EXIT_CODES::NO_ERRORS;
}

static double cpuPop(cpu_t *CPU)
{
    // Error check
    if (CPU == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return BAD_DOUBLE_VALUE;
    }

    // Pop
    double result = DEFAULT_DOUBLE_VALUE;
    IS_ERROR(_cpuPop(CPU, &result))
    {
        // TODO: processor error flag that determines the stay of processor
        PRINT_ERROR_TRACING_MESSAGE(PROCESSOR_EXIT_CODES::ERROR_READING_DOUBLE_FROM_STACK);
        return BAD_DOUBLE_VALUE;
    }
    return result;
}

static EXIT_CODES cpuPush(cpu_t *CPU, double value)
{
    // Error check
    if (CPU == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Push
    IS_ERROR(stackPush(&CPU->stack, value))
    {
        PRINT_ERROR_TRACING_MESSAGE(PROCESSOR_EXIT_CODES::ERROR_PUSHING_VALUE_TO_STACK);
        return EXIT_CODES::BAD_OBJECT_PASSED;
    }   

    return EXIT_CODES::NO_ERRORS;
}

static EXIT_CODES _cpuGetBytecodeOffset(cpu_t *CPU, text_t *byteCode, offset *result)
{
    // Error check
    if (CPU == NULL || byteCode == NULL || result == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Get offset
    *result = *((offset *) &byteCode->data[CPU->ip]);

    return EXIT_CODES::NO_ERRORS;
}

static offset cpuGetBytecodeOffset(cpu_t *CPU, text_t *byteCode)
{
    // Error check
    if (CPU == NULL || byteCode == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        cpuExit(CPU, byteCode, EXIT_FAILURE);
    }

    // Get offset
    offset displacement = 0;
    IS_ERROR(_cpuGetBytecodeOffset(CPU, byteCode, &displacement))
    {
        PRINT_ERROR_TRACING_MESSAGE(PROCESSOR_EXIT_CODES::FAIL_DURING_TAKING_OFFSET);
        cpuExit(CPU, byteCode, EXIT_FAILURE);
    }

    return displacement;
}

static EXIT_CODES cpuOut(cpu_t *CPU)
{
    // Error check
    if (CPU == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Out
    printf("%lf", cpuPop(CPU));

    return EXIT_CODES::NO_ERRORS;
}

static EXIT_CODES cpuIn(cpu_t *CPU)
{
    // Error check
    if (CPU == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Get input
    double input = 0;
    scanf("%lf", &input);

    // Push
    IS_ERROR(cpuPush(CPU, input))
    {
        PRINT_ERROR_TRACING_MESSAGE(PROCESSOR_EXIT_CODES::ERROR_PUSHING_INPUTED_VALUE_TO_STACK);
        return EXIT_CODES::BAD_OBJECT_PASSED;
    }

    return EXIT_CODES::NO_ERRORS;
}

static EXIT_CODES cpuOutc(cpu_t *CPU)
{
    // Error check
    if (CPU == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    printf("%c", (int) cpuPop(CPU));

    return EXIT_CODES::NO_ERRORS;
}

static EXIT_CODES cpuMoveValue(cpu_t *CPU, text_t *byteCode, double value)
{
    // Error check
    if (CPU == NULL || byteCode == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Move value
    size_t argc     = (size_t)  GET_TOTAL_ARGS(byteCode->data[CPU->ip]);
    int globalMRI   = (int)     GET_GLOBAL_MRI(byteCode->data[CPU->ip++]);
    if (MRI_IS_MEMORY(globalMRI))
    {
        double result = 0;
        IS_ERROR(__cpuCountInternalExpressionValue(CPU, byteCode, argc, &result))
        {
            PRINT_ERROR_TRACING_MESSAGE(PROCESSOR_EXIT_CODES::ERROR_COUNTING_INTERNAL_EXPRESSION_VALUE);
            return EXIT_CODES::BAD_OBJECT_PASSED;
        }

        // TODO: check all RAM index etc. && maybe do separate function
        // TODO: copy-paste do function or something
        if (fabs(result - fabs((int) result)) < EPS)
        {
            CPU->RAM[(int) result] = value;
        }
        else
        {
            PRINT_ERROR_TRACING_MESSAGE(PROCESSOR_EXIT_CODES::BAD_TO_ACCESS_RAM_INDEX);
            return EXIT_CODES::BAD_OBJECT_PASSED;
        }       
    }
    else
    {
        if (MRI_IS_REGISTER(byteCode->data[CPU->ip]))
        {
            // Move value into register
            ++CPU->ip;

            byte regCode = (byte) byteCode->data[CPU->ip];
            CPU->commonRegs[regCode] = value;

            CPU->ip += sizeof(byte);
        }
        else
        {
            PRINT_ERROR_TRACING_MESSAGE(PROCESSOR_EXIT_CODES::CANT_POP_VALUE_INTO_IMMEDIATE);
            return EXIT_CODES::BAD_OBJECT_PASSED;
        }
    }

    return EXIT_CODES::NO_ERRORS;
}

// FIXME: CHECK
static EXIT_CODES cpuGOut(cpu_t *CPU, double indexToOut)
{
    // Error check
    if (CPU == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Putchar from VRAM
    printf("%c", CPU->VRAM[(int) indexToOut]);

    return EXIT_CODES::NO_ERRORS;
}

#define OPDEF(unused, opcode, argc, code, ...)   \
    case ((byte) opcode): { code }; break; //printf("mnemonics now: %s\n", #unused); 

static EXIT_CODES cpuExecuteCommand(cpu_t *CPU, text_t *byteCode)
{
    // Error check
    if (CPU == NULL || byteCode == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Execution
    double val              = DEFAULT_DOUBLE_VALUE;
    double val1             = DEFAULT_DOUBLE_VALUE;
    double val2             = DEFAULT_DOUBLE_VALUE;
    offset displacement     = 0;

    byte opcode = (byte) byteCode->data[CPU->ip++];    
    switch(opcode)
    {
        #include "include/opdefs.h"

        default:
            PRINT_ERROR_TRACING_MESSAGE(PROCESSOR_EXIT_CODES::UNKNOWN_OPCODE_BYTE);
            return EXIT_CODES::BAD_OBJECT_PASSED;
    } 

    return EXIT_CODES::NO_ERRORS;
}

#undef OPDEF

EXIT_CODES cpuExecuteBytecode(text_t *byteCode, cpu_t *CPU)
{
    // Error check
    if (byteCode == NULL || CPU == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }
    
    // Execution
    // char s;
    while ((size_t) CPU->ip < byteCode->size)
    {
        // cpuDump(CPU, byteCode);
        IS_ERROR(cpuExecuteCommand(CPU, byteCode))
        {
            PRINT_ERROR_TRACING_MESSAGE(PROCESSOR_EXIT_CODES::BAD_BYTECODE_PASSED);
            return EXIT_CODES::BAD_OBJECT_PASSED;
        }
        // cpuDump(CPU, byteCode);

        // printf("Command executed!\n");
        // scanf("%c", &s);
    }

    // TODO: IP increase problem
    printf("WHILE END!\n");
    printf("byteCode->size: %d\n", byteCode->size);

    return EXIT_CODES::NO_ERRORS;
}
