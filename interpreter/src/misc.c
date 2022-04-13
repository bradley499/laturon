#ifndef misc_c
#define misc_c

#include <stdlib.h>
#include <string.h>
#include "misc.h"
#include "array.h"
#include "interact.h"

void *xmalloc(unsigned int size)
{
	void *result = calloc(size, sizeof(void));
	if (result == NULL)
		fatal_error(MEMORY_ALLOCATION_ERROR);
	return result;
}

unsigned char int_to_str(int n, char str[], int padding)
{
	if (n == 0 && padding == 0)
		padding = 1;
	int i = 0;
	if (n < 0)
		str[i++] = '-';
	while (n)
	{
		str[i++] = (n % 10) + '0'; // Convert last digit to ascii char
		n = n / 10;
	}

	for (unsigned char ii = 0; ii < padding; ii++)
		str[i++] = '0'; // Leftwards 0 padding
	char str_tmp[i];
	for (int ii = 0; ii < i; ii++)
		str_tmp[ii] = str[(i - 1 - ii)];
	for (int ii = 0; ii < i; ii++)
		str[ii] = str_tmp[ii]; // Reverse char array
	str[i] = '\0';
	return i;
}

void float_to_string(double n, char *res)
{
	int whole_number = (int)n;					// Extract whole integer from double
	double decimals = n - (double)whole_number; // Extract floating point decimal from double
	unsigned char i = int_to_str(whole_number, res, (whole_number == 0));
	unsigned char lessen[2] = {0, 0};
	for (unsigned ii = 0; ii < FLOAT_MAX_PRECISION; ii++)
	{
		decimals *= 10;
		if ((int)(decimals + FLOAT_PRECISION_COMPENSATE_EXPONENT) % 10 == 0 && lessen[1] == 0)
			lessen[0]++; // Increase leftwards padding
		else
			lessen[1] = 1; // Decimal value is not completely 0
	}
	res[i++] = '.';
	if (lessen[1] != 0)
	{
		while ((int)decimals % 10 == 0)
			decimals /= 10;
		decimals += FLOAT_PRECISION_COMPENSATE_EXPONENT; // Compensat for double precision inaccuracies
		i += (int_to_str((int)decimals, (res + i), lessen[0]) - 1);
		for (int ii = 0; ii < FLOAT_MAX_PRECISION; ii++)
		{
			if (res[i--] == '0')	 // If is trailing 0
				res[(i + 1)] = '\0'; // Remove trailing 0
			else
			{
				i += 2;
				break;
			}
			if (ii == (FLOAT_MAX_PRECISION - 1))
				i++;
		}
	}
	else
		res[i++] = '0';
	res[i] = '\0';
}

void bool_to_string(double n, char *res)
{
	if (n == BOOLEAN_TRUE)
		strcpy(res, "True\0");
	else
		strcpy(res, "False\0");
}

void fatal_error(error_codes code)
{
	// TODO: Implement cleanup routine
	error_code((int)code);
}

void syntax_error(syntax_errors error, unsigned int line)
{
#define SYNTAX_ERROR_BUFFER_SIZE 140
	char buffer[SYNTAX_ERROR_BUFFER_SIZE];
	switch (error)
	{
	case NO_FUNCTION_DEFINITION:
		snprintf(buffer, SYNTAX_ERROR_BUFFER_SIZE, "No function definition or reference was given. On line: %d", line);
		break;
	case NO_FUNCTION_REFERENCE:
		snprintf(buffer, SYNTAX_ERROR_BUFFER_SIZE, "Unable to call a function that has no reference. On line: %d", line);
		break;
	case NO_VARIABLE_DEFINITION:
		snprintf(buffer, SYNTAX_ERROR_BUFFER_SIZE, "No variable definition or reference was given. On line: %d", line);
		break;
	case SCOPE_OPEN_WITHIN_EXPRESSION:
		snprintf(buffer, SYNTAX_ERROR_BUFFER_SIZE, "Attempting to open a scope within an expression. On line: %d", line);
		break;
	case SCOPE_CLOSE_UNABLE:
		snprintf(buffer, SYNTAX_ERROR_BUFFER_SIZE, "Closing a scope that has not been opened. On line: %d", line);
		break;
	case PARENTHESES_CLOSE_UNABLE:
		snprintf(buffer, SYNTAX_ERROR_BUFFER_SIZE, "Closing a parentheses that has not been opened. On line: %d", line);
		break;
	case BRACKET_CLOSE_UNABLE:
		snprintf(buffer, SYNTAX_ERROR_BUFFER_SIZE, "Closing a bracket that has not been opened. On line: %d", line);
		break;
	case INVALID_FUNCTION_DEFINITION:
		snprintf(buffer, SYNTAX_ERROR_BUFFER_SIZE, "Invalid function definition. On line: %d", line);
		break;
	case INVALID_VARIABLE_DEFINITION:
		snprintf(buffer, SYNTAX_ERROR_BUFFER_SIZE, "Invalid variable definition. On line: %d", line);
		break;
	case FUNCTION_DEFINITION_RECURSIVE:
		snprintf(buffer, SYNTAX_ERROR_BUFFER_SIZE, "Unable to define a function with a recursive function definition. On line: %d", line);
		break;
	case VARIABLE_DEFINITION_RECURSIVE:
		snprintf(buffer, SYNTAX_ERROR_BUFFER_SIZE, "Unable to define a variable with a recursive variable definition. On line: %d", line);
		break;
	case INVALID_OPERATION:
		snprintf(buffer, SYNTAX_ERROR_BUFFER_SIZE, "An invalid operation was defined within the syntax. On line: %d", line);
		break;
	case UNUSED_VARIABLE:
		snprintf(buffer, SYNTAX_ERROR_BUFFER_SIZE, "A variable is referenced but not utilised. On line: %d", line);
		break;
	case INVALID_VARIABLE_NAME:
		snprintf(buffer, SYNTAX_ERROR_BUFFER_SIZE, "A variable has a name that is not supported. On line: %d", line);
		break;
	case INVALID_FUNCTION_NAME:
		snprintf(buffer, SYNTAX_ERROR_BUFFER_SIZE, "A function has a name that is not supported. On line: %d", line);
		break;
	case INVALID_FUNCTION_PARAMETERS:
		snprintf(buffer, SYNTAX_ERROR_BUFFER_SIZE, "The total amount of parameters used in a function call does not match the function definition. On line: %d", line);
		break;
	case INVALID_NUMERIC:
		snprintf(buffer, SYNTAX_ERROR_BUFFER_SIZE, "An invalid numerical value was given. On line: %d", line);
		break;
	case EMPTY_RETURN:
		snprintf(buffer, SYNTAX_ERROR_BUFFER_SIZE, "Attempting to return where there is nothing given. On line: %d", line);
		break;
	case STATEMENT_AFTER_RETURN:
		snprintf(buffer, SYNTAX_ERROR_BUFFER_SIZE, "A statement was declared after a return without closing a function scope. On line: %d", line);
		break;
	case INVALID_REMOVE:
		snprintf(buffer, SYNTAX_ERROR_BUFFER_SIZE, "Attempting to remove list element where list is not present. On line: %d", line);
		break;
	case INVALID_BREAK:
		snprintf(buffer, SYNTAX_ERROR_BUFFER_SIZE, "A break statement has been invalidly given additional parameters. On line: %d", line);
		break;
	case INVALID_WHILE:
		snprintf(buffer, SYNTAX_ERROR_BUFFER_SIZE, "A while loop was declared incorrectly. On line: %d", line);
	case LINE_ENDED_INCORRECTLY:
		snprintf(buffer, SYNTAX_ERROR_BUFFER_SIZE, "The line ended abruptly. On line: %d", line);
		break;
	case DUPLICATE_FUNCTION_DEFINITION:
		snprintf(buffer, SYNTAX_ERROR_BUFFER_SIZE, "A function was defined that already exists. On line: %d", line);
		break;
	case NUMERIC_CONVERSION_FAILED:
		snprintf(buffer, SYNTAX_ERROR_BUFFER_SIZE, "Unable to convert numeric value to a numeric type. On line: %d", line);
		break;
	case INVALID_VARIABLE_REFERENCE:
		snprintf(buffer, SYNTAX_ERROR_BUFFER_SIZE, "A reference to a variable could not be established due to an invalid format. On line: %d", line);
		break;
	case INVALID_PARAMETERS:
		snprintf(buffer, SYNTAX_ERROR_BUFFER_SIZE, "A functions parameters were incorrectly defined. On line: %d", line);
		break;
	case INVALID_SYNTAX:
	default:
		snprintf(buffer, SYNTAX_ERROR_BUFFER_SIZE, "Invalid syntax. On line: %d", line);
		break;
	}
	output(buffer, OUTPUT_ERROR);
	exit(EXIT_FAILURE);
#undef SYNTAX_ERROR_BUFFER_SIZE
}

#endif
