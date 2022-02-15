#ifndef misc_c
#define misc_c

#include <stdlib.h>
#include <string.h>
#include "misc.h"
#include "array.h"
#include "run.h"
#include "scope.h"

void *xmalloc(unsigned int size)
{
	void *result = malloc(size);
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
	if (n == SCOPE_BOOLEAN_TRUE)
		strcpy(res, "True\0");
	else
		strcpy(res, "False\0");
}

array_value_t *string_new(char *str)
{
	array_value_t *array_root = array_new();
	array_root->result_type = SCOPE_TYPE_STRING;
	for (unsigned int i = 0; i < strlen(str); i++)
		array_insert(array_root, i, SCOPE_TYPE_INT, 0, (signed long long int)str[i]);
	return array_root;
}

void fatal_error(error_codes code)
{
	// TODO: Implement cleanup routine
	exit((int)code); // Terminate program
}

#endif