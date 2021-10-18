#ifndef misc_c
#define misc_c

#include <stdlib.h>
#include <string.h>
#include "misc.h"
#include "run.h"
#include "scope.h"
#include "bigfloat.h"

char * int_to_string(BigFloat *n)
{
	return toString(n, 0);
}

char * float_to_string(BigFloat *n)
{
	return toString(n, 1);
}

char * bool_to_string(BigFloat *n)
{
	char *res = NULL;
	if (equals(n, SCOPE_BOOLEAN_BIGFLOAT_TRUE))
	{
		res = malloc(sizeof(char) * 5);
		if (res == NULL)
			fatal_error(RUN_MEMORY_ALLOCATION_ERROR);
		strcpy(res, "True\0");
	}
	else
	{
		res = malloc(sizeof(char) * 6);
		if (res == NULL)
			fatal_error(RUN_MEMORY_ALLOCATION_ERROR);
		strcpy(res, "False\0");
	}
	return res;
}

char *new_string() {
	return malloc(STRING_MEMORY_MAX_LENGTH * sizeof(char));
}

void fatal_error(unsigned int error_code)
{
	exit(error_code);
}

char *new_string_size(unsigned short size) {
	if (size == 0 || size > STRING_MEMORY_MAX_LENGTH)
		return NULL;
	return malloc((sizeof(char) * size));
}

#endif