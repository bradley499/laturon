#ifndef misc_h
#define misc_h

#include "bigfloat.h"

#define FLOAT_MAX_PRECISION                 6	  // Maximum level of precision when representing floating point values
#define FLOAT_PRECISION_COMPENSATE_EXPONENT 1e-7  // Used to offset precision in floating point values to a more precise readable format
#define STRING_MEMORY_MAX_LENGTH            4096  // Maximum length of a string

#if STRING_MEMORY_MAX_LENGTH < (BIGFLOAT_PRECISION + 2)
#error `STRING_MEMORY_MAX_LENGTH` needs to be bigger!
#endif

char *int_to_string(BigFloat *n);
char *float_to_string(BigFloat *n);
char *bool_to_string(BigFloat *n);
void fatal_error(unsigned int error_code);
char *new_string();
char *new_string_size(unsigned short size);

#endif