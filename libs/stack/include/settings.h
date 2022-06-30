/**
 * @file settings.h
 * @author Vladislav Skvortsov (vladislavskvo@gmail.com)
 * @brief File containing configuration parameters for stack operation
 * @version 0.1
 * @date 2022-06-30
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef STACK_SETTINGS_H
#define STACK_SETTINGS_H


#pragma region STACK_INTERNALS
    #define stackElem_t                 double  // Stack is consists of stackElem_t type variables only
    #define stackReallocCoefficient     2       // Determines how many times the stack increases/decreases
    #define POISON                      -663    // Poison for identifying unused values  
#pragma endregion STACK_INTERNALS


#pragma region STACK_SECURITY
    #define STACK_CANARY    1  // 0 - disable canaries, 1 - enable canaries
    #define STACK_HASH      1  // 0 - disable checksum (hashsum), 1 - enable checksum (hashsum)

    #if defined(STACK_CANARY) && STACK_CANARY == 1
        #define CANARY_VALUE    0xBABE  // Default canary value
    #endif
#pragma endregion STACK_SECURITY


#endif  // STACK_SETTINGS_H
