#ifndef LABELS_H
#define LABELS_H

#include <stdbool.h>

#define DEBUG_LEVEL 1
#include "libs/debug/debug.h"

#define LABEL_LINE_FORMAT               "%[a-zA-Z0-9_]:%lln"
#define LABEL_ARG_FORMAT                "%[a-zA-Z0-9_]%lln"
const int MIN_LABELS_COUNT              = 2;
const int ALLOC_INC_COEF                = 2;
const int MAX_LABEL_STR_LENGTH          = 50;

/**
 * @brief An enum class used to track the return function results of functions represented in `assembler` project.
 * 
 */
enum class LABELS_EXIT_CODES
{
    BAD_LABEL_NAME,
    BAD_LABEL_FORMAT,
};

typedef long long int ll;

/**
 * @brief Structure that represents a single label
 * 
 */
struct label_t
{
    char name[MAX_LABEL_STR_LENGTH] = {};
    ll length                       = 0;
    long int offset                 = 0;
};

/**
 * @brief Structure that stores all labels and some other meta-information
 * 
 */
struct labels_t
{
    label_t *labels         = NULL;
    ll currAllocatedLabels  = 0;
    int totalLabels         = 0;
};

/**
 * @brief Construction of `labels` data structure
 * 
 * @param labels 
 * @return EXIT_CODES 
 */
EXIT_CODES labelsCtor(labels_t *labels);

/**
 * @brief Destruction of `labels` data structure
 * 
 * @param labels 
 * @return EXIT_CODES 
 */
EXIT_CODES labelsDtor(labels_t *labels);

/**
 * @brief Function that determines whether the current line is a label
 * 
 * @param data 
 * @param LABEL_FORMAT 
 * @return true 
 * @return false 
 */
bool isLabel(char *data, const char *LABEL_FORMAT);

/**
 * @brief Function that initializes (constructs) one label and puts it into `labels` data structure
 * 
 * @param data 
 * @param labels 
 * @param LABEL_FORMAT 
 * @param globalOffset 
 * @return EXIT_CODES 
 */
EXIT_CODES initLabel(char *data, labels_t *labels, const char *LABEL_FORMAT, const int globalOffset);

/**
 * @brief Function that increases the number of labels that can be contained in the `labels` data structure
 * 
 * @param labels 
 * @return EXIT_CODES 
 */
EXIT_CODES expandLabelsArray(labels_t *labels);


#endif  // LABELS_H
