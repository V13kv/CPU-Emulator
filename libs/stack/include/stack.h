#ifndef STACK_H
#define STACK_H

#include <cstddef>
#include <stdbool.h>

#define DEBUG_LEVEL 2
#include "libs/debug/debug.h"
#include "settings.h"

#ifndef STACK_CANARY
    #define STACK_CANARY 0
#endif

#ifndef STACK_HASH
    #define STACK_HASH 0
#endif

#ifndef DEBUG_LEVEL
    #define DEBUG_LEVEL 0
#endif

enum class STACK_EXIT_CODES
{
    OLD_STACK_PASSED,
    BAD_STACK_SIZE,
    PASSED_STACK_IS_NULLPTR,
    BAD_REALLOC_MODE_PASSED,
    STACK_DATA_CANARY_IS_DAMAGED,
    STACK_STRUCTURE_CANARY_IS_DAMAGED,
    STACK_HASH_SUM_IS_DAMAGED,
    BAD_STACK_PASSED,
};

enum class REALLOC_MODES
{
    INCREASE,
    DECREASE
};

/**
 * @brief Structure represents stack data structure.
 * 
 */
struct stack_t
{
    #if STACK_CANARY == 1
        const int canaryLeft = CANARY_VALUE;    // Left canary value to signal out-of-bounds calls (overflow/underflow) (global canaries)
    #endif

    stackElem_t *data = NULL;                   // All stack elements are located here (+ left and right canaries to monitor violation of stack bounds) (local canaries)
    int capacity = -1;                          // Current capacity of stack
    int size = -1;                              // Current size of stack

    #if STACK_CANARY == 1
        const int canaryRight = CANARY_VALUE;   // Right canary value to signal out-of-bounds calls (overflow/underflow)
    #endif

    #if STACK_HASH == 1
        long long int hashSum = 0;              // Checksum (hashsum) to signal arbitrary write to stack structure (global canaries), contains info about all stack structure 
    #endif
};

#if DEBUG_LEVEL == 2
    /**
     * @brief Dump of all stack structure and its fields
     * 
     * @param stack 
     * @return EXIT_CODES 
     */
    EXIT_CODES stackDump(stack_t *stack);
#else
    #define stackDump(stack) EXIT_CODES::NO_ERRORS;
#endif

#if STACK_CANARY == 1  
    /**
     * @brief Check stack canary values (both global and local canaries)
     * 
     * @param stack 
     * @param result 
     * @return EXIT_CODES 
     */
    EXIT_CODES canaryCheck(stack_t *stack, bool *result);

    /**
     * @brief Construct local canaries
     * 
     * @param stack 
     * @param stack_capacity 
     * @return EXIT_CODES 
     */
    EXIT_CODES canaryCtor(stack_t *stack, int stack_capacity);
#endif

#if STACK_HASH == 1
    /**
     * @brief Calculate hash sum, that contains information about all stack structure
     * 
     * @param stack 
     * @param hashSum 
     * @return EXIT_CODES 
     */
    EXIT_CODES calculateStackHashSum(stack_t *stack, long long int *hashSum);

    /**
     * @brief Check current stack structure hashsum and previous one
     * 
     * @param stack 
     * @param result 
     * @return EXIT_CODES 
     */
    EXIT_CODES stackHashCheck(stack_t *stack, bool *result);
#endif

#if STACK_CANARY == 1 || STACK_HASH == 1
    /**
     * @brief Determine the capacity increase for security fields of the stack
     * 
     * @param stack 
     * @param add_bytes 
     * @return EXIT_CODES 
     */
    EXIT_CODES stackCapacityIncrease(stack_t *stack, int *add_bytes);
#else
    #define stackCapacityIncrease(stack, add_bytes)  EXIT_CODES::NO_ERRORS;
#endif

bool stackBasicCheck(stack_t *stack);
bool stackOk(stack_t *stack);

/**
 * @brief Spraying poison on all allocated uninitialized stack data
 * 
 * @param stack 
 * @return EXIT_CODES 
 */
EXIT_CODES sprayPoisonOnData(stack_t *stack);

/**
 * @brief Construction of stack data structure
 * 
 * @param stack 
 * @param stack_capacity 
 * @return EXIT_CODES 
 */
EXIT_CODES stackCtor(stack_t *stack, int stack_capacity = 12);

/**
 * @brief Deconstruction of stack data structure
 * 
 * @param stack 
 * @return EXIT_CODES 
 */
EXIT_CODES stackDtor(stack_t *stack);

/**
 * @brief Get the New Reallocation Capacity object
 * 
 * @param stack 
 * @param mode 
 * @param new_capacity 
 * @return EXIT_CODES 
 */
EXIT_CODES getNewReallocationCapacity(stack_t *stack, REALLOC_MODES mode, int *new_capacity);

/**
 * @brief Reallocate stack (increase its capacity)
 * 
 * @param stack 
 * @param mode 
 * @return EXIT_CODES 
 */
EXIT_CODES stackReallocation(stack_t *stack, REALLOC_MODES mode);

/**
 * @brief Push element into stack
 * 
 * @param stack 
 * @param value 
 * @return EXIT_CODES 
 */
EXIT_CODES stackPush(stack_t *stack, stackElem_t value);

#define DEFAULT_POPTO_VALUE (stackElem_t *) 1
/**
 * @brief Pop element from the stack
 * 
 * @param stack 
 * @param popTo 
 * @return EXIT_CODES 
 */
EXIT_CODES stackPop(stack_t *stack, stackElem_t *popTo = DEFAULT_POPTO_VALUE);

#endif  // STACK_H
