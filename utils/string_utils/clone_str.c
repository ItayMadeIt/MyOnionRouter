#include "string_utils.h"

#include <string.h>
#include <stdlib.h>

char* clone_str(const char* str)
{
    if (!str) return NULL;

    uint64_t size = strlen(str) + 1; // null terminator

    char* result = (char*)malloc(sizeof(char) * size);

    if (!result) return NULL;

    memcpy(result, str, size);

    return result;
}