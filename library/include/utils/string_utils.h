#ifndef __STRING_UTILS_H__
#define __STRING_UTILS_H__

#include <stdint.h>

char* clone_str(const char* str);

char* str_nclone(const char* str, uint64_t n);

#endif // __STRING_UTILS_H__