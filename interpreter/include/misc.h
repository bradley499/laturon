#ifndef misc_h
#define misc_h

#include "array.h"

#define FLOAT_MAX_PRECISION 6					 // Maximum level of precision when representing floating point values
#define FLOAT_PRECISION_COMPENSATE_EXPONENT 1e-7 // Used to offset precision in floating point values to a more precise readable format
#define STRING_MEMORY_MAX_LENGTH 4096			 // Maximum length of a string

#define BOOLEAN_FALSE 0.0
#define BOOLEAN_TRUE 1.0
#define VALUE_NULL (int)(BOOLEAN_TRUE + 1)
#define TOTAL_DEFAULT_VALUES 3

typedef enum error_codes
{
	UNKNOWN_ERROR,				// An unknown error occurred
	MEMORY_ALLOCATION_ERROR,	// Failed to allocate memory
	LOGIC_ERROR,				// Logical operation failed
	CONVERSION_ERROR,			// Failure to convert to another type
	CLEANUP_ERROR,				// Failed to cleanup variables outside of current scope
	EXECUTION_ERROR,			// Failed to execute operation
	STACK_REFERENCE_ERROR,		// An invalid reference to a call stack scope occurred
	STRING_INPUT_ERROR,			// Failed to correctly read in user input string
	MISSING_COMPOUND_LITERAL,	// A reference to a compound literal does not exist
	ARRAY_TYPE_EXPECTED_ERROR,	// An array routine was not given an array to operate on
	ARRAY_LOCATION_ERROR,		// A reference to an item within an array that is out of range
	IO_ERROR,					// Failed to perform an operation on source file
	INVALID_SYNTAX_GENERIC,		// The source provided has invalid syntax
	STACK_MEMORY_LIMIT_REACHED, // The total amount of stack memory available to execute your program has been reached
	TOO_BIG_NUMERIC,			// Too many functions or variables are declared within your program to be handled within memory
	VALUE_NOT_SET,				// Attempted to use a variable which has not been set yet
	ZERO_DIVISION_ERROR,		// Zero division error
	UNIMPLEMENTED_YET,			// You are attempting to use syntax that has not been finalised
} error_codes;

typedef enum syntax_errors
{
	INVALID_SYNTAX,				   // Invalid syntax
	NO_FUNCTION_DEFINITION,		   // No function definition or reference was given
	NO_FUNCTION_REFERENCE,		   // Unable to call a function that has no reference
	NO_VARIABLE_DEFINITION,		   // No variable definition or reference was given
	SCOPE_OPEN_WITHIN_EXPRESSION,  // Attempting to open a scope within an expression
	SCOPE_CLOSE_UNABLE,			   // Closing a scope that has not been opened
	PARENTHESES_CLOSE_UNABLE,	   // Closing a parentheses that has not been opened
	BRACKET_CLOSE_UNABLE,		   // Closing a bracket that has not been opened
	INVALID_FUNCTION_DEFINITION,   // Invalid function definition
	PARAMETER_ABUSED,			   // Too many parameters were declared to be handled within a function
	INVALID_VARIABLE_DEFINITION,   // Invalid variable definition
	FUNCTION_DEFINITION_RECURSIVE, // Unable to define a function with a recursive function definition
	VARIABLE_DEFINITION_RECURSIVE, // Unable to define a variable with a recursive variable definition
	LINE_ENDED_INCORRECTLY,		   // The line ended abruptly
	UNUSED_VARIABLE,			   // A variable is referenced but not utilised
	INVALID_VARIABLE_NAME,		   // A variable has a name that is not supported
	INVALID_FUNCTION_NAME,		   // A function has a name that is not supported
	INVALID_FUNCTION_PARAMETERS,   // The total amount of parameters used in a function call does not match the function definition
	INVALID_PARAMETER_NAME_GLOBAL, // A parameter has the same name as a variable that is declared within the global scope
	INVALID_OPERATION,			   // An invalid operation was defined within the syntax
	INVALID_NUMERIC,			   // An invalid numerical value was given
	RETURN_WITHOUT_FUNCTION,	   // Unable to return when not use within a function
	EMPTY_RETURN,				   // Attempting to return where there is nothing given
	STATEMENT_AFTER_RETURN,		   // A statement was declared after a return without closing a function scope
	INVALID_REMOVE,				   // Attempting to remove list element where list is not present
	INVALID_BREAK,				   // A break statement has been invalidly given additional parameters
	INVALID_WHILE,				   // A while loop was declared incorrectly
	NUMERIC_CONVERSION_FAILED,	   // Unable to convert numeric value to a numeric type
	DUPLICATE_FUNCTION_DEFINITION, // A function was defined that already exists
	INVALID_VARIABLE_REFERENCE,	   // A reference to a variable could not be established due to an invalid format
	INVALID_PARAMETERS,			   // A functions parameters were incorrectly defined
	INVALID_LITERAL_TERMINATION,   // A string was incorrectly terminated
	INVALID_ARRAY_TERMINATION,	   // A list was not closed off
	INVALID_SCOPE_TERMINATION,	   // An open scope was not closed
	DANGLING_VARIABLE_REFERENCE,   // A dangling variable reference occurred prior to an operation
} syntax_errors;

// Blocking malloc which will exit program execution on failure
void *xmalloc(unsigned int size);
// Convert integer to string
unsigned char int_to_str(signed long long n, char str[], int padding);
// Convert string to integer
signed long long str_to_int(char *str);
// Convert float to string
void float_to_string(long double n, char *res);
// Convert string to float
long double str_to_float(char *str);
// Convert boolean to string
void bool_to_string(double n, char *res);
// Copy a string and verify a successful copy
int copy_string(char *destination, char *source);
// Warning of numeric convertion of large number
void convertion_numeric_warning();
// Warning of floating point convertion of large number
void convertion_floating_warning();
// Exit program with error code
void fatal_error(error_codes code);
// Exit program with error code with line
void fatal_error_lined(error_codes code, unsigned long long line);
// Output a syntax error with its associative line number then exits the program
void syntax_error(syntax_errors error, unsigned long long line);

#endif
