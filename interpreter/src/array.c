#ifndef stack_c
#define stack_c

#include <stddef.h>
#include <stdlib.h>
#include "array.h"
#include "misc.h"
#include "scope.h"

array_value_t* array_new()
{
	array_value_t* array = xmalloc(sizeof(array_value_t));
	array->next = NULL;
	return array;
}

void array_delete(array_value_t *array)
{
	if (array == NULL)
		fatal_error(ARRAY_TYPE_EXPECTED_ERROR);
	typedef struct array_references
	{
		array_value_t* array;
		struct array_references* next;
	} array_references;
	array_references* arrays = xmalloc(sizeof(array_references));
	arrays->array = array;
	arrays->next = NULL;
	for (;;)
	{
		if (arrays == NULL)
			break;
		array_value_t* current = arrays->array;
		arrays = arrays->next;
		for (;current != NULL;)
		{
			if (current->result_type == SCOPE_TYPE_ARRAY)
			{
				if (arrays == NULL)
				{
					arrays = xmalloc(sizeof(array_references));
					arrays->array = (array_value_t *)current->result.result_int;
					arrays->next = NULL;
				}
				else
				{
					array_references* array_current = arrays;
					for (;array_current->next != NULL;)
						array_current = array_current->next;
					array_references* array_reference = xmalloc(sizeof(array_references));
					array_reference->array = (array_value_t *)current->result.result_int;
					array_reference->next = NULL;
					array_current->next = array_reference;
					current = current->next;
					array_current->next->next = NULL;
					continue;
				}
			}
			array_value_t* next = current->next;
			array_value_t* current_array_item = current;
			current = next;
			free(current_array_item);
		}
	}
}

int array_remove(array_value_t *array, int at)
{
	array_value_t* current = array;
	if (current == NULL)
		fatal_error(ARRAY_TYPE_EXPECTED_ERROR);
	if (current->next == NULL)
	{
		array_delete(current);
		return SCOPE_BOOLEAN_TRUE;
	}
	array_value_t* previous = NULL;
	unsigned int size = 0;
	do
	{
		if (size == at)
		{
			if (current->next != NULL)
			{
				current->result = current->next->result;
				current->result_type = current->next->result_type;
				struct array_value_t *next = current->next->next;
				free(current->next);
				current->next = next;
			}
			else
			{
				previous->next = current->next;
				free(current);
			}
			return SCOPE_BOOLEAN_TRUE;
		}
		size++;
		previous = current;
		current = current->next;
	} while (size <= at && current != NULL);
	return SCOPE_BOOLEAN_FALSE;
}

int array_update(array_value_t *array, int at, unsigned char value_type, double value_double, signed long long int value_int)
{
	array_value_t* current = array;
	if (current == NULL)
		fatal_error(ARRAY_TYPE_EXPECTED_ERROR);
	unsigned int size = 0;
	do
	{
		if (size == at)
		{
			current->result_type = value_type;
			if (current->result_type == SCOPE_TYPE_DOUBLE)
				current->result.result_double = value_double;
			else
				current->result.result_int = value_int;
			return SCOPE_BOOLEAN_TRUE;
		}
		size++;
		current = current->next;
	} while (size <= at && current != NULL);
	return SCOPE_BOOLEAN_FALSE;
}

int array_insert(array_value_t *array, int at, unsigned char value_type, double value_double, signed long long int value_int)
{
	array_value_t* current = array;
	if (current == NULL)
		fatal_error(ARRAY_TYPE_EXPECTED_ERROR);
	unsigned int size = 0;
	do
	{
		if (size == at)
		{
			array_value_t *array_insert = (array_value_t *)array_new();
			array_insert->result_type = value_type;
			if (array_insert->result_type == SCOPE_TYPE_DOUBLE)
				array_insert->result.result_double = value_double;
			else
				array_insert->result.result_int = value_int;
			if (size == 0)
			{
				unsigned char result_type = array_insert->result_type;
				array_insert->result_type = current->result_type;
				array_insert->result = current->result;
				array_insert->next = current->next;
				current->result_type = result_type;
				if (result_type == SCOPE_TYPE_DOUBLE)
					current->result.result_double = value_double;
				else
					current->result.result_int = value_int;
				current->next = array_insert;
			}
			else
				array_insert->next = current;
			return SCOPE_BOOLEAN_TRUE;
		}
		size++;
		current = current->next;
	} while (size <= at && current != NULL);
	return SCOPE_BOOLEAN_FALSE;
}

array_value_t* array_get(array_value_t *array, int at)
{
	array_value_t* current = array;
	if (current == NULL)
		fatal_error(ARRAY_TYPE_EXPECTED_ERROR);
	unsigned int size = 0;
	do
	{
		if (size == at)
			return current;
		size++;
		current = current->next;
	} while (size <= at);
	return NULL;
}

unsigned int array_length(array_value_t *array)
{
	array_value_t* current = array;
	if (current == NULL)
		fatal_error(ARRAY_TYPE_EXPECTED_ERROR);
	unsigned int size = 0;
	while (current->next != NULL)
	{
		size++;
		current = current->next;
	}
	return size;	
}

array_value_t* array_duplicate(array_value_t *array)
{
	if (array == NULL)
		fatal_error(ARRAY_TYPE_EXPECTED_ERROR);
	array_value_t* array_copy = NULL;
	typedef struct array_references
	{
		array_value_t* array;
		array_value_t* parent;
		struct array_references* next;
	} array_references;
	array_references* arrays = xmalloc(sizeof(array_references));
	arrays->array = array;
	arrays->next = NULL;
	arrays->parent = NULL;
	for (;arrays != NULL;)
	{
		array_references* current = arrays;
		int is_nested = SCOPE_BOOLEAN_FALSE;
		for (;;)
		{
			struct array_value_t* new_value = xmalloc(sizeof(struct array_value_t));
			new_value->result = current->array->result;
			new_value->result_type = current->array->result_type;
			new_value->next = NULL;
			if (current->parent == NULL) // Top level array element
			{
				if (array_copy == NULL)
					array_copy = new_value;
				else
				{
					array_value_t * array_current = array_copy;
					for (;array_current->next != NULL;)
						array_current = array_current->next;
					array_current->next = new_value;
				}
			}
			else // Nested array
			{
				array_value_t* array_current = current->parent;
				if (is_nested)
				{
					struct array_value_t* array_nested = (array_value_t *)array_current->result.result_int;
					for (;array_nested->next != NULL;)
						array_nested = array_nested->next;
					array_nested->next = new_value;
				}
				else
				{
					array_current->result.result_int = (long long int)&new_value;
					is_nested = SCOPE_BOOLEAN_TRUE;
				}
			}
			if (new_value->result_type == SCOPE_TYPE_ARRAY)
			{
				array_references* array_reference = xmalloc(sizeof(array_references));
				array_reference->array = (array_value_t *)new_value->result.result_int;
				array_reference->parent = new_value;
				array_references* arrays_appender = arrays;
				for (;arrays_appender->next != NULL;)
					arrays_appender = arrays_appender->next;
				arrays_appender->next = array_reference;
			}
			current->array = current->array->next;
			if (current->array == NULL) // End of current array
				break;
		}
		arrays = arrays->next;
		free(current);
	}
	return array_copy;
}

#endif