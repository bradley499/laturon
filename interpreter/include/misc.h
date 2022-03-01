#ifndef misc_h
#define misc_h

#include "array.h"

#define FLOAT_MAX_PRECISION 6					 // Maximum level of precision when representing floating point values
#define FLOAT_PRECISION_COMPENSATE_EXPONENT 1e-7 // Used to offset precision in floating point values to a more precise readable format
#define STRING_MEMORY_MAX_LENGTH 4096			 // Maximum length of a string

typedef enum error_codes
{
	UNKNOWN_ERROR,			   // An unknown error
	MEMORY_ALLOCATION_ERROR,   // Failed to allocate memory
	LOGIC_ERROR,			   // Logical operation failed
	CONVERSION_ERROR,		   // Failure to convert to another type
	CLEANUP_ERROR,			   // Failed to cleanup variables outside of current scope
	EXECUTION_ERROR,		   // Failed to execute operation
	STACK_REFERENCE_ERROR,	   // An invalid reference to a call stack scope occurred
	STRING_INPUT_ERROR,		   // Failed to correctly read in user input string
	MISSING_COMPOUND_LITERAL,  // A reference to a compound literal does not exist
	ARRAY_TYPE_EXPECTED_ERROR, // An array routine was not given an array to operate on
	ARRAY_LOCATION_ERROR,	   // A reference to an item within an array that is out of range
	IO_ERROR,				   // Failed to perform an operation on source file
} error_codes;

// Blocking malloc which will exit program execution on failure
void *xmalloc(unsigned int size);
// Convert integer to string
unsigned char int_to_str(int n, char str[], int padding);
// Convert float to string
void float_to_string(double n, char *res);
// Convert boolean to string
void bool_to_string(double n, char *res);
// Exit program with error code
void fatal_error(error_codes code);
// Allocate a new string
array_value_t *string_new(char *str);

#endif