#include <stdio.h>
#include <string.h>  // for strcpy && memcpy

#include "libs/text/include/text.h"

#include "include/asm/assembler.h"
#include "include/asm/labels.h"

#include "include/asm/settings.h"
#include "include/constants.h"

static EXIT_CODES parseCommand(text_line_t *line, command_t *command, labels_t *unprocCommandArgLabels, const int globalOffset);
static EXIT_CODES normalizeCodeLine(text_line_t *line);
static EXIT_CODES hasArguments(char *mnemonics, bool *hasArgs);
static EXIT_CODES isSpecialInstruction(command_t *command, bool *isSpecInstr);
static EXIT_CODES checkMnemonics(char *mnemonics);
static EXIT_CODES setCommandMnemonics(command_t *command, char *mnemonics);
static EXIT_CODES setCommandOpcode(command_t *command);
static EXIT_CODES parseCommandArguments(command_t *command, text_line_t *line, int argsStart, int argsEnd, labels_t *unprocCommandArgLabels, const int globalOffset);
static EXIT_CODES parseArgument(command_t *command, int *argNumber, text_line_t *line, int *argStart);
static EXIT_CODES checkRegisterForCorrectness(char *reg);
static EXIT_CODES getArgumentsMathOperation(text_line_t *line, int *argStart, char *mathOP);

static EXIT_CODES encodeCommand(command_t *command);
static EXIT_CODES encodeRegisterArgument(command_t *command, char *regStr);
static EXIT_CODES encodeImmediateArgument(command_t *command, char *immStr);
static EXIT_CODES exportEncodedCommand(command_t *command, FILE *fs);

static EXIT_CODES fillUnprocCommandArgLabels(labels_t *unproc, labels_t *labels, FILE *fs);

static EXIT_CODES resetCommand(command_t *command);

/**
 * @brief Main function that translates assembly source code file into bytecode file
 * 
 * @param code 
 * @param outputFileName 
 * @return EXIT_CODES 
 */
EXIT_CODES assembly(text_t *code, char *outputFileName)
{
    // Error check
    if (code == NULL || outputFileName == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Output file
    FILE *fs = fopen(outputFileName, "wb");
    if (fs == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::BAD_STD_FUNC_RESULT);
        return EXIT_CODES::BAD_STD_FUNC_RESULT;
    }

    // General labels (labels declaration)
    labels_t labels = {};
    IS_OK_W_EXIT(labelsCtor(&labels));

    // Command argument labels (labels used in commands)
    labels_t unprocCommandArgLabels = {};
    IS_OK_W_EXIT(labelsCtor(&unprocCommandArgLabels));

    // Assembly
    int globalOffset = 0;  // (local)
    command_t command = {};
    for (int line = 0; line < code->lines_count; ++line)
    {
        IS_OK_W_EXIT(normalizeCodeLine(&code->lines[line]));

        if (code->lines[line].length)
        {    
            // TODO: check for complex instruction, e.g. push <string> (separate into multiple push instructions)  
            if (isLabel(code->lines[line].beginning, LABEL_LINE_FORMAT))
            {
                IS_OK_W_EXIT(initLabel(code->lines[line].beginning, &labels, LABEL_LINE_FORMAT, globalOffset));
            }
            else
            {
                // Parse command
                IS_OK_W_EXIT(parseCommand(&code->lines[line], &command, &unprocCommandArgLabels, globalOffset));

                IS_OK_W_EXIT(encodeCommand(&command));
                IS_OK_W_EXIT(exportEncodedCommand(&command, fs));
                
                // Update global offset (for label's offset identification)
                globalOffset += command.encoded.bytes;

                IS_OK_W_EXIT(resetCommand(&command));  
            }
        }
    }
    
    IS_OK_W_EXIT(fillUnprocCommandArgLabels(&unprocCommandArgLabels, &labels, fs));
    // printf(GREEN "[+] Translation successfully done\n" RESET);

    IS_OK_W_EXIT(labelsDtor(&unprocCommandArgLabels));
    IS_OK_W_EXIT(labelsDtor(&labels));
    fclose(fs);

    return EXIT_CODES::NO_ERRORS;
} 

/**
 * @brief Function that gets the actual string length of a command (i.e. without leading spaces and without comments)
 * 
 * @param line 
 * @return EXIT_CODES 
 */
static EXIT_CODES normalizeCodeLine(text_line_t *line)
{
    // Error check
    if (line == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Beautifying
    char *r = line->beginning;
    char *w = line->beginning;  // pointer to line->beginning to be overwritten
    char previous_char = ' ';
    while (*r != ';' && *r != '\0')
    {
        if (*r != ' ' || previous_char != ' ')
        {
            *w = *r;
            ++w;
        }

        previous_char = *r;
        ++r;
    }

    if (w != line->beginning)
    {
        if (*(w - 1) == ' ')
        {
            *(w - 1) = '\0';
        }
        else
        {
            *w = '\0';
        }
    }
    else
    {
        *w = '\0';
    }
    line->length = strlen(line->beginning);

    return EXIT_CODES::NO_ERRORS;
}

/**
 * @brief Function that parses an entire command
 * 
 * @param line 
 * @param command 
 * @param unprocCommandArgLabels 
 * @param globalOffset 
 * @return EXIT_CODES 
 */
static EXIT_CODES parseCommand(text_line_t *line, command_t *command, labels_t *unprocCommandArgLabels, const int globalOffset)
{
    // Error check
    if (line == NULL || command == NULL || unprocCommandArgLabels == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Get mnemonics && arguments
    int mnemonicsEnd    = 0;
    int argsStart       = 0;
    int argsEnd         = 0;
    char mnemonics[MAX_MNEMONICS_STR_LENGTH] = {};
    char arguments[MAX_ARGUMENTS_PER_COMMAND * MAX_ARGUMENT_STR_LENGTH] = {};

    int ret = sscanf(line->beginning, COMMAND_FORMAT, mnemonics, &mnemonicsEnd, &argsStart, arguments, &argsEnd);
    CHECK_SSCANF_RESULT(ret);
    if (ret == 0)
    {
        PRINT_ERROR_TRACING_MESSAGE(ASM_EXIT_CODES::BAD_COMMAND_FORMAT);
        return EXIT_CODES::BAD_OBJECT_PASSED;
    }

    // Parse mnemonics
    // TODO: посчитать один раз индекс нахождения в массиве
    IS_OK_W_EXIT(checkMnemonics(mnemonics)); 
    IS_OK_W_EXIT(setCommandMnemonics(command, mnemonics));
    IS_OK_W_EXIT(setCommandOpcode(command));

    // Check mnemonics for arguments existence
    bool hasArgs = true;
    IS_OK_W_EXIT(hasArguments(command->mnemonics, &hasArgs));

    if (hasArgs)
    {
        // Check for whitespace between mnemonics and arguments
        if (mnemonicsEnd == argsStart)
        {
            PRINT_ERROR_TRACING_MESSAGE(ASM_EXIT_CODES::BAD_COMMAND_FORMAT);
            return EXIT_CODES::BAD_OBJECT_PASSED;
        }

        // Parse arguments
        IS_OK_W_EXIT(parseCommandArguments(command, line, argsStart, argsEnd, unprocCommandArgLabels, globalOffset));
    }

    return EXIT_CODES::NO_ERRORS;
}

/**
 * @brief Function that determines whether the current command has any arguments (based on command mnemonics)
 * 
 * @param mnemonics 
 * @param hasArgs 
 * @return EXIT_CODES 
 */
static EXIT_CODES hasArguments(char *mnemonics, bool *hasArgs)
{
    // Erorr check
    if (mnemonics == NULL || hasArgs == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Get argument count
    for (size_t instrIndx = 0; instrIndx < MNEMONICS_TABLE_LENGTH; ++instrIndx)
    {
        if (!strcmp(mnemonics, MNEMONICS_TABLE[instrIndx]))
        {
            *hasArgs = !!OPERATION_ARGS_COUNT_TABLE[instrIndx];
            return EXIT_CODES::NO_ERRORS;
        }
    }

    PRINT_ERROR_TRACING_MESSAGE(ASM_EXIT_CODES::UNKNOWN_MNEMONICS);
    return EXIT_CODES::BAD_OBJECT_PASSED;
}

static EXIT_CODES isSpecialInstruction(command_t *command, bool *isSpecInstr)
{
    // Error check
    if (command == NULL || isSpecInstr == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Check
    // if (command->mnemonics[0] != 'j' && command->mnemonics[0] != 'c')
    // {
    //     *isSpecInstr = false;
    //     return EXIT_CODES::NO_ERRORS;
    // }
    // else
    // {
    // }
    if (!strcmp(command->mnemonics, "jmp"))
    {
        *isSpecInstr = true;
    }
    else if (!strcmp(command->mnemonics, "je"))
    {
        *isSpecInstr = true;
    }
    else if (!strcmp(command->mnemonics, "jne"))
    {
        *isSpecInstr = true;
    }
    else if (!strcmp(command->mnemonics, "jl"))
    {
        *isSpecInstr = true;
    }
    else if (!strcmp(command->mnemonics, "jg"))
    {
        *isSpecInstr = true;
    }
    else if (!strcmp(command->mnemonics, "call"))
    {
        *isSpecInstr = true;
    }
    else
    {
        *isSpecInstr = false;
    }

    return EXIT_CODES::NO_ERRORS;
}

/**
 * @brief Function that checks current command mnemonics for existence
 * 
 * @param mnemonics 
 * @return EXIT_CODES 
 */
static EXIT_CODES checkMnemonics(char *mnemonics)
{
    // Error check
    if (mnemonics == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Cheking
    for (size_t instrIndx = 0; instrIndx < MNEMONICS_TABLE_LENGTH; ++instrIndx)
    {
        if (!strcmp(MNEMONICS_TABLE[instrIndx], mnemonics))
        {
            return EXIT_CODES::NO_ERRORS;  
        }
    }

    PRINT_ERROR_TRACING_MESSAGE(ASM_EXIT_CODES::UNKNOWN_MNEMONICS);
    return EXIT_CODES::BAD_OBJECT_PASSED;
}

/**
 * @brief Set the Command Mnemonics in the `command` data structure
 * 
 * @param command 
 * @param mnemonics 
 * @return EXIT_CODES 
 */
static EXIT_CODES setCommandMnemonics(command_t *command, char *mnemonics)
{
    // Error check
    if (command == NULL || mnemonics == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Set command mnemonics
    strcpy(command->mnemonics, mnemonics);

    return EXIT_CODES::NO_ERRORS;
}

/**
 * @brief Set the Command Opcode in the `command` data structure
 * 
 * @param command 
 * @return EXIT_CODES 
 */
static EXIT_CODES setCommandOpcode(command_t *command)
{
    // Error check
    if (command == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Set command mnemonics opcode
    for (size_t mnemonics = 0; mnemonics < MNEMONICS_TABLE_LENGTH; ++mnemonics)
    {
        if (!strcmp(MNEMONICS_TABLE[mnemonics], command->mnemonics))
        {
            command->opcode = OPERATION_OPCODES_TABLE[mnemonics];
            return EXIT_CODES::NO_ERRORS;
        }
    }

    PRINT_ERROR_TRACING_MESSAGE(ASM_EXIT_CODES::UNKNOWN_MNEMONICS);
    return EXIT_CODES::BAD_OBJECT_PASSED;
}

/**
 * @brief Function that parses all command arguments
 * 
 * @param command 
 * @param line 
 * @param argsStart 
 * @param argsEnd 
 * @param unprocCommandArgLabels 
 * @param globalOffset 
 * @return EXIT_CODES 
 */
static EXIT_CODES parseCommandArguments(command_t *command, text_line_t *line, int argsStart, int argsEnd, labels_t *unprocCommandArgLabels, const int globalOffset)
{
    // Error check
    if (command == NULL || line == NULL || unprocCommandArgLabels == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Check for special instructions (instructions that use labels as command args or it is label itself)
    bool isSpecInstr = true;
    IS_OK_W_EXIT(isSpecialInstruction(command, &isSpecInstr));
    if (isSpecInstr)
    {
        if (isLabel(&line->beginning[argsStart], LABEL_ARG_FORMAT))
        {
            IS_OK_W_EXIT(initLabel(&line->beginning[argsStart], unprocCommandArgLabels, LABEL_ARG_FORMAT, globalOffset));
            
            command->isSpecialCommand   = true;
            command->argumentsCount     = ONE_ARGUMENT;
        }
        else
        {
            PRINT_ERROR_TRACING_MESSAGE(ASM_EXIT_CODES::BAD_LABEL_FORMAT);
            return EXIT_CODES::BAD_OBJECT_PASSED;
        }
    }
    else
    {
        // If there are memory brackets, then syntax-check them for correctness
        if  ((line->beginning[argsStart] == '[' && line->beginning[argsEnd - 1] != ']') ||
             (line->beginning[argsStart] != '[' && line->beginning[argsEnd - 1] == ']'))
        {
            PRINT_ERROR_TRACING_MESSAGE(ASM_EXIT_CODES::BAD_COMMAND_MEMORY_BRACKETS_USE);
            return EXIT_CODES::BAD_OBJECT_PASSED;
        }

        // Set command MRI
        if (line->beginning[argsStart] == '[')
        {
            ++argsStart;
            --argsEnd;

            SET_MRI_MEMORY(command->MRI);
        }

        // Parse command arguments
        int argNumber = 0;
        char op = '\0';
        int argStart = argsStart;
        while (argStart < argsEnd)
        {
            IS_OK_W_EXIT(parseArgument(command, &argNumber, line, &argStart));
            if (argStart < argsEnd)
            {
                // TODO: #6 #5 Support of `-`, etc (*additional all math ops as functions, like +(ax, 123) etc) @V13kv
                IS_OK_W_EXIT(getArgumentsMathOperation(line, &argStart, &op)); // FIXME: #12 possible bug, the result of this function is not used anywhere?!?!? (maybe it is needed to parse complex argument such as "1 + 2 - 3", or "1 * 2 * 4 - 3 + 1" and etc) @V13kv
            }
        }
        command->argumentsCount = argNumber;
    }
    
    return EXIT_CODES::NO_ERRORS;
}

/**
 * @brief Function that parses one argument of a command
 * 
 * @param command 
 * @param argNumber 
 * @param line 
 * @param argStart 
 * @return EXIT_CODES 
 */
static EXIT_CODES parseArgument(command_t *command, int *argNumber, text_line_t *line, int *argStart)
{
    // Error check
    if (command == NULL || argNumber == NULL || line == NULL || argStart == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Parse one argument
    int newArgStart = 0;
    char reg[MAX_REGISTER_STR_LENGTH] = {};
    if (sscanf(&line->beginning[*argStart], REGISTER_FORMAT, reg, &newArgStart) == 1)
    {
        IS_OK_W_EXIT(checkRegisterForCorrectness(reg));
        strcpy(command->arguments[*argNumber], reg);

        SET_MRI_REGISTER(command->argsMRI[*argNumber]);
    }
    else
    {
        double imm = 0;
        if (sscanf(&line->beginning[*argStart], IMMEDIATE_VALUE_FORMAT, &imm, &newArgStart) == 1)
        {
            memcpy(command->arguments[*argNumber], &imm, sizeof(double));

            SET_MRI_IMMEDIATE(command->argsMRI[*argNumber]);
        }
        else
        {
            PRINT_ERROR_TRACING_MESSAGE(ASM_EXIT_CODES::BAD_COMMAND_ARGUMENTS);
            return EXIT_CODES::BAD_OBJECT_PASSED;
        }
    }

    *argStart += newArgStart;
    ++(*argNumber);

    return EXIT_CODES::NO_ERRORS;
}

/**
 * @brief Function that check whether the register mentioned in a command argument exists
 * 
 * @param reg 
 * @return EXIT_CODES 
 */
static EXIT_CODES checkRegisterForCorrectness(char *reg)
{
    // Error check
    if (reg == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Check
    if (reg[1] != 'x' || reg[0] > 'd')
    {
        PRINT_ERROR_TRACING_MESSAGE(ASM_EXIT_CODES::UNKNOWN_COMMAND_REGISTER);
        return EXIT_CODES::BAD_OBJECT_PASSED;
    }

    return EXIT_CODES::NO_ERRORS;
}

// TODO: #6 Support of -, etc (*additional all math ops as functions, like +(ax, 123) etc)
static EXIT_CODES getArgumentsMathOperation(text_line_t *line, int *argStart, char *mathOP)
{
    // Error check
    if (mathOP == NULL || line == NULL || argStart == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Get math operation
    switch (line->beginning[*argStart])
    {
        case '+':
            *mathOP = '+';
            break;
        case '-':
            *mathOP = '-';
            break;
        default:
            *mathOP = '\0';
            PRINT_ERROR_TRACING_MESSAGE(ASM_EXIT_CODES::BAD_MATH_OPERATION_IN_COMMAND_ARG);
            return EXIT_CODES::BAD_OBJECT_PASSED;
    }
    ++(*argStart);

    return EXIT_CODES::NO_ERRORS;
}

/**
 * @brief Function that encodes an entire parsed command to the bytecode
 * 
 * @param command 
 * @return EXIT_CODES 
 */
static EXIT_CODES encodeCommand(command_t *command)
{
    // Error check
    if (command == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Encode metadata
    command->encoded.byteData[command->encoded.bytes++] = (byte) command->opcode;

    // Special instructions encoding
    if (command->isSpecialCommand)
    {
        command->encoded.bytes += sizeof(offset);
    }
    // Common (not zero arg) instructions encoding
    else if (command->argumentsCount != NO_ARGUMENTS)
    {
        // Encode metadata
        command->encoded.byteData[command->encoded.bytes] = (byte) ENCODE_COMMAND_ARGS_COUNT(command->argumentsCount);
        command->encoded.byteData[command->encoded.bytes++] |= (byte) ENCODE_COMMAND_MRI(command->MRI);

        // Encode arguments
        for (size_t arg = 0; arg < command->argumentsCount; ++arg)
        {
            // Encode arg's MRI
            command->encoded.byteData[command->encoded.bytes++] = (byte) command->argsMRI[arg];
            
            // Properly encode argument
            if (ARG_IS_REGISTER(command->argsMRI[arg]))
            {
                IS_OK_W_EXIT(encodeRegisterArgument(command, command->arguments[arg]));
                command->encoded.bytes += sizeof(byte);
            }
            else
            {
                IS_OK_W_EXIT(encodeImmediateArgument(command, command->arguments[arg]));
                command->encoded.bytes += sizeof(double);
            }
        }
    }

    return EXIT_CODES::NO_ERRORS;
}

/**
 * @brief Function that encodes the register (to the bytecode)
 * 
 * @param command 
 * @param regStr 
 * @return EXIT_CODES 
 */
static EXIT_CODES encodeRegisterArgument(command_t *command, char *regStr)
{
    // Error check
    if (command == NULL || regStr == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Encode
    if (regStr[1] != 'x' || regStr[0] > 'd')
    {
        PRINT_ERROR_TRACING_MESSAGE(ASM_EXIT_CODES::UNKNOWN_COMMAND_REGISTER);
        return EXIT_CODES::BAD_OBJECT_PASSED;
    }

    command->encoded.byteData[command->encoded.bytes] = (byte) (regStr[0] - 'a');  // ax opcode

    return EXIT_CODES::NO_ERRORS;
}

/**
 * @brief Function that encodes an immediate argument value (to the bytecode)
 * 
 * @param command 
 * @param immStr 
 * @return EXIT_CODES 
 */
static EXIT_CODES encodeImmediateArgument(command_t *command, char *immStr)
{
    // Error check
    if (command == NULL || immStr == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Encode
    memcpy(&command->encoded.byteData[command->encoded.bytes], immStr, sizeof(double));

    return EXIT_CODES::NO_ERRORS;
}

/**
 * @brief Function that exports one entirely encoded command to the file
 * 
 * @param command 
 * @param fs 
 * @return EXIT_CODES 
 */
static EXIT_CODES exportEncodedCommand(command_t *command, FILE *fs)
{
    // Error check
    if (command == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }
    
    // Export
    fwrite(command->encoded.byteData, sizeof(byte), command->encoded.bytes, fs);

    return EXIT_CODES::NO_ERRORS;
}

/**
 * @brief Function that fills offsets for all unprocessed labels (that were occured after the command referenced to them) in the export file
 * 
 * @param unproc 
 * @param labels 
 * @param fs 
 * @return EXIT_CODES 
 */
static EXIT_CODES fillUnprocCommandArgLabels(labels_t *unproc, labels_t *labels, FILE *fs)
{
    // Error check
    if (unproc == NULL || labels == NULL || fs == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Fill unprocessed argument labels of commands
    for (int argLabel = 0; argLabel < unproc->totalLabels; ++argLabel)
    {
        for (int label = 0; label < labels->totalLabels; ++label)
        {
            if (!strcmp(unproc->labels[argLabel].name, labels->labels[label].name))
            {
                CHECK_FSEEK_RESULT(fseek(fs, unproc->labels[argLabel].offset + 1, SEEK_SET));

                fwrite(&labels->labels[label].offset, sizeof(offset), 1, fs);
            }
        }
    }

    return EXIT_CODES::NO_ERRORS;
}

/**
 * @brief Function that wipes all the previous data contained in `command` data structure
 * 
 * @param command 
 * @return EXIT_CODES 
 */
static EXIT_CODES resetCommand(command_t *command)
{
    // Erorr check
    if (command == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Reset
    memset(command->mnemonics, 0, sizeof(command->mnemonics));
    memset(command->arguments, 0, sizeof(command->arguments));
    memset(command->argsMRI, 0, sizeof(command->argsMRI));
    memset(command->encoded.byteData, 0, sizeof(command->encoded.byteData));

    command->opcode             = 0;
    command->argumentsCount     = 0;
    command->MRI                = 0;
    command->isSpecialCommand   = 0;
    command->encoded.bytes      = 0;

    return EXIT_CODES::NO_ERRORS;
}
