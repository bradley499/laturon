#ifndef array_h
#define array_h

#include "variable.h"

typedef struct array_value_t
{
	union
	{
		signed long long int numeric;
		long double floating;
		char *string;
		struct array_value_t *array;
	} value;
	variable_type_t result_type; // Value result type
	struct array_value_t *next;
} array_value_t;

array_value_t *array_new();
void array_delete(array_value_t *array);
array_value_t *array_remove(array_value_t *array, unsigned long long at);
int array_update(array_value_t *array, unsigned long long at, unsigned char type, signed long long int numeric, long double floating, char *string, array_value_t *array_nested);
int array_insert(array_value_t *array, unsigned long long at, unsigned char type, signed long long int numeric, long double floating, char *string, array_value_t *array_nested);
array_value_t *array_get(array_value_t *array, unsigned long long at);
unsigned long long array_length(array_value_t *array);
array_value_t *array_duplicate(array_value_t *array);

#endif