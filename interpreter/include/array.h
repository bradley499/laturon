#ifndef array_h
#define array_h

#include "stack.h"

typedef struct array_value_t
{
	union
	{
		double result_double;
		signed long long int result_int;
	} result;				   // Value result
	unsigned char result_type; // Value result type
	struct array_value_t *next;
} array_value_t;

array_value_t *array_new();
void array_delete(array_value_t *array);
array_value_t *array_remove(array_value_t *array, int at);
int array_update(array_value_t *array, int at, unsigned char value_type, double value_double, signed long long int value_int);
int array_insert(array_value_t *array, int at, unsigned char value_type, double value_double, signed long long int value_int);
array_value_t *array_get(array_value_t *array, int at);
unsigned int array_length(array_value_t *array);
array_value_t *array_duplicate(array_value_t *array);

#endif