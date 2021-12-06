#ifndef misc_h
#define misc_h

#define FLOAT_MAX_PRECISION                 6	  // Maximum level of precision when representing floating point values
#define FLOAT_PRECISION_COMPENSATE_EXPONENT 1e-7  // Used to offset precision in floating point values to a more precise readable format
#define STRING_MEMORY_MAX_LENGTH            4096  // Maximum length of a string

enum error_codes {
	UNKNOWN_ERROR, // An unknown error
	MEMORY_ALLOCATION_ERROR, // Failed to allocate memory
	LOGIC_ERROR, // Logical operation failed
	CONVERSION_ERROR, // Failure to convert to another type
	CLEANUP_ERROR, // Failed to cleanup variables outside of current scope
	EXECUTION_ERROR, // Failure to execute operation
};

void *xmalloc(unsigned int size);
unsigned char int_to_str(int n, char str[], int padding);
void float_to_string(double n, char *res);
void bool_to_string(double n, char *res);
void fatal_error(unsigned int error_code);
char *new_string();
char *new_string_size(unsigned short size);

#endif