#ifndef misc_h
#define misc_h

#define FLOAT_MAX_PRECISION                 6	  // Maximum level of precision when representing floating point values
#define FLOAT_PRECISION_COMPENSATE_EXPONENT 1e-7  // Used to offset precision in floating point values to a more precise readable format
#define STRING_MEMORY_MAX_LENGTH            4096  // Maximum length of a string

unsigned char int_to_str(int n, char str[], int padding);
void float_to_string(double n, char *res);
void bool_to_string(double n, char *res);
char *new_string();
char *new_string_size(unsigned short size);

#endif