// TODO: #4 Add support for multiple (0+) arguments support in commands (comma separated or smth) @V13kv

#include "libs/text/include/text.h"
#include "libs/colors/colors.h"

#include "include/asm/assembler.h"

void hint();
char *getFileName(int argc, char **argv, int fileIndex);

int main(int argc, char **argv)
{
    text_t code = {};
    textCtor(&code, getFileName(argc, argv, 1), FILE_MODE::R);

    assembly(&code, getFileName(argc, argv, 2));

    textDtor(&code);
    return 0;
}

void hint()
{
    printf(RED "Incorrect inline argument input!\n" RESET);
    printf("asm.exe <file_name> <output_file_name>\n");
}

char *getFileName(int argc, char *argv[], int fileIndex)
{
    char *file_name = NULL;
    if (argc >= fileIndex + 1)
    {
        file_name = argv[fileIndex];
    }
    else
    {
        hint();
    }

    return file_name;
}
