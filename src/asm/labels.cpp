#include <stdbool.h>
#include <stdio.h>  // for sscanf
#include <stdlib.h>  // for calloc && realloc && free 

#include "include/asm/labels.h"

/**
 * @brief Construction of `labels` data structure
 * 
 * @param labels 
 * @return EXIT_CODES 
 */
EXIT_CODES labelsCtor(labels_t *labels)
{
    // Error check
    if (labels == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Construction
    labels->labels = (label_t *) calloc(MIN_LABELS_COUNT, sizeof(label_t));
    if (labels->labels == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::BAD_STD_FUNC_RESULT);
        return EXIT_CODES::BAD_STD_FUNC_RESULT;
    }

    labels->currAllocatedLabels = MIN_LABELS_COUNT;

    return EXIT_CODES::NO_ERRORS;
}

/**
 * @brief Destruction of `labels` data structure
 * 
 * @param labels 
 * @return EXIT_CODES 
 */
EXIT_CODES labelsDtor(labels_t *labels)
{
    // Error check
    if (labels == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Destruction
    free(labels->labels);
    labels->labels                  = NULL;
    labels->currAllocatedLabels     = 0;
    labels->totalLabels             = 0;

    return EXIT_CODES::NO_ERRORS;
}

/**
 * @brief Function that increases the number of labels that can be contained in the `labels` data structure
 * 
 * @param labels 
 * @return EXIT_CODES 
 */
EXIT_CODES expandLabelsArray(labels_t *labels)
{
    // Error check
    if (labels == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Reallocation
    label_t *newLabels = (label_t *) realloc(labels->labels, ALLOC_INC_COEF * labels->currAllocatedLabels * sizeof(label_t));
    if (newLabels == NULL)
    {
        IS_OK_W_EXIT(labelsDtor(labels));

        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::BAD_STD_FUNC_RESULT);
        return EXIT_CODES::BAD_STD_FUNC_RESULT;
    }

    labels->labels = newLabels;
    labels->currAllocatedLabels = ALLOC_INC_COEF * labels->currAllocatedLabels;

    return EXIT_CODES::NO_ERRORS;
}

// ******TODO: label strip function

/**
 * @brief Function that initializes (constructs) one label and puts it into `labels` data structure
 * 
 * @param data 
 * @param labels 
 * @param LABEL_FORMAT 
 * @param globalOffset 
 * @return EXIT_CODES 
 */
EXIT_CODES initLabel(char *data, labels_t *labels, const char *LABEL_FORMAT, const int globalOffset)
{
    // Error check
    if (data == NULL || labels == NULL || LABEL_FORMAT == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        return EXIT_CODES::PASSED_OBJECT_IS_NULLPTR;
    }

    // Initialization
    label_t *currentLabel = &labels->labels[labels->totalLabels];
    int ret = sscanf(data, LABEL_FORMAT, currentLabel->name, &currentLabel->length);

    // Error check + format check
    CHECK_SSCANF_RESULT(ret);
    if (ret != 1)
    {
        PRINT_ERROR_TRACING_MESSAGE(LABELS_EXIT_CODES::BAD_LABEL_NAME);
        return EXIT_CODES::BAD_OBJECT_PASSED;
    }

    // Update `labels` fiels
    currentLabel->offset = globalOffset;
    ++labels->totalLabels;

    // Check for reallocation
    if (labels->totalLabels >= labels->currAllocatedLabels)
    {
        IS_OK_W_EXIT(expandLabelsArray(labels));
    }

    return EXIT_CODES::NO_ERRORS;
}

/**
 * @brief Function that determines whether the current line is a label
 * 
 * @param data 
 * @param LABEL_FORMAT 
 * @return true 
 * @return false 
 */
bool isLabel(char *data, const char *LABEL_FORMAT)
{
    // Error check
    if (data == NULL || LABEL_FORMAT == NULL)
    {
        PRINT_ERROR_TRACING_MESSAGE(EXIT_CODES::PASSED_OBJECT_IS_NULLPTR);
        exit(EXIT_FAILURE);
    }

    // Check
    label_t temp = {};
    int ret = sscanf(data, LABEL_FORMAT, temp.name, &temp.length);
    if (ret != 1 || ret == EOF)
    {
        PRINT_ERROR_TRACING_MESSAGE(LABELS_EXIT_CODES::BAD_LABEL_FORMAT);
        exit(EXIT_FAILURE);
    }

    if (temp.length == 0)
    {
        return false;
    }

    return true;
}
