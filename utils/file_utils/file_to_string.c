#include "file_utils.h"

#include <string.h>
#include <stdlib.h>

char* file_to_string(FILE* fd)
{
    fseek(fd, 0L, SEEK_END);
    int length = ftell(fd);

    char* str = malloc(length + 1);
    str[length] = '\0'; 

    fseek(fd, 0, SEEK_SET);

    int bytes = 0;
    if ((bytes = fread(str, sizeof(char), length, fd)) != length)
    {
        fprintf(stderr, "Bytes was not sufficient in it's mission to read bytes.\n");
        return NULL;
    }

    return str;
}