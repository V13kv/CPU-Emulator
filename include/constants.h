#include <stddef.h>  // for size_t

#define OPDEF(opName, ...) #opName,

    const char *MNEMONICS_TABLE[] = {
        #include "opdefs.h"
    };

    const size_t MNEMONICS_TABLE_LENGTH = sizeof(MNEMONICS_TABLE) / sizeof(MNEMONICS_TABLE[0]);

#undef OPDEF

// TODO: rename OPCODES_TABLE to OPERATION_OPCODES_TABLE
#define OPDEF(unused, opcode, ...) opcode,

    const int OPCODES_TABLE[] = {
        #include "opdefs.h"
    };

#undef OPDEF

// TODO: rename ARGS_COUNT_TABLE to OPERATION_ARGS_COUNT_TABLE
#define OPDEF(unused1, unused2, opArgsCount, ...) opArgsCount,

    const int ARGS_COUNT_TABLE[] = {
        #include "opdefs.h"
    };

#undef OPDEF
