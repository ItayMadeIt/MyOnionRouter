#include "string_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* str_nclone(const char* str, uint64_t n)
{
    if (!str) return NULL;

    // limit length
    uint64_t actual_len = strlen(str);
    if (n > actual_len) n = actual_len;

    char* result = (char*)malloc(n+1);
    if (!result) return NULL;

    memcpy(result, str, n);
    result[n] = '\0';

    return result;
}