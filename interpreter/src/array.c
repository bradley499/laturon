#ifndef stack_c
#define stack_c

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "array.h"
#include "variable.h"
#include "misc.h"

array_value_t *array_new()
{
	array_value_t *array = xmalloc(sizeof(array_value_t));
	array->next = NULL;
	return array;
}

void array_delete(array_value_t *array)
{
	if (array == NULL)
		fatal_error(ARRAY_TYPE_EXPECTED_ERROR);
	typedef struct array_references
	{
		array_value_t *array;
		struct array_references *next;
	} array_references;
	array_references *arrays = xmalloc(sizeof(array_references));
	arrays->array = array;
	arrays->next = NULL;
	for (;;)
	{
		if (arrays == NULL)
			break;
		array_value_t *current = arrays->array;
		arrays = arrays->next;
		for (; current != NULL;)
		{
			if (current->result_type == VARIABLE_ARRAY)
			{
				if (arrays == NULL)
				{
					arrays = xmalloc(sizeof(array_references));
					arrays->array = current->value.array;
					arrays->next = NULL;
				}
				else
				{
					array_references *array_current = arrays;
					for (; array_current->next != NULL;)
						array_current = array_current->next;
					array_references *array_reference = xmalloc(sizeof(array_references));
					array_reference->array = current->value.array;
					array_reference->next = NULL;
					array_current->next = array_reference;
					current = current->next;
					array_current->next->next = NULL;
					continue;
				}
			}
			array_value_t *current_array_item = current;
			current = current->next;
			if (current_array_item->result_type == VARIABLE_STRING)
				free(current_array_item->value.string);
			free(current_array_item);
		}
	}
}

array_value_t *array_remove(array_value_t *array, unsigned long long at)
{
	array_value_t *current = array;
	if (current == NULL)
		fatal_error(ARRAY_TYPE_EXPECTED_ERROR);
	if (current->next == NULL)
	{
		if (at != 0)
			fatal_error(ARRAY_LOCATION_ERROR);
		array_delete(current);
		return NULL;
	}
	array_value_t *previous = NULL;
	unsigned int size = 0;
	do
	{
		if (size == at)
		{
			if (current->next != NULL)
			{
				if (current->result_type == VARIABLE_STRING)
					free(current->value.string);
				else if (current->result_type == VARIABLE_ARRAY)
					array_delete(current->value.array);
				current->value = current->next->value;
				current->result_type = current->next->result_type;
				struct array_value_t *next = current->next->next;
				free(current->next);
				current->next = next;
			}
			else
			{
				previous->next = current->next;
				if (current->result_type == VARIABLE_STRING)
					free(current->value.string);
				else if (current->result_type == VARIABLE_ARRAY)
					array_delete(current->value.array);
				free(current);
			}
			return array;
		}
		size++;
		previous = current;
		current = current->next;
	} while (size <= at && current != NULL);
	fatal_error(ARRAY_LOCATION_ERROR);
	return NULL;
}

int array_update(array_value_t *array, unsigned long long at, unsigned char type, signed long long int numeric, long double floating, char *string, array_value_t *array_nested)
{
	array_value_t *current = array;
	if (current == NULL)
		fatal_error(ARRAY_TYPE_EXPECTED_ERROR);
	unsigned int size = 0;
	do
	{
		if (size == at)
		{
			if (current->result_type == VARIABLE_STRING)
				free(current->value.string);
			else if (current->result_type == VARIABLE_ARRAY)
				array_delete(current->value.array);
			current->result_type = type;
			switch (current->result_type)
			{
			case VARIABLE_INT:
			case VARIABLE_BOOLEAN:
			case VARIABLE_NULL:
				current->value.numeric = numeric;
				break;
			case VARIABLE_DOUBLE:			
				current->value.floating = floating;
				break;
			case VARIABLE_STRING:
			{
				current->value.string = xmalloc(strlen(string) + 1);
				if (!copy_string(current->value.string, string))
					fatal_error(MEMORY_ALLOCATION_ERROR);
				break;
			}
			case VARIABLE_ARRAY:
				current->value.array = array;
				break;
			default:
				fatal_error(UNKNOWN_ERROR);
				break;
			}
			return BOOLEAN_TRUE;
		}
		size++;
		current = current->next;
	} while (size <= at && current != NULL);
	return BOOLEAN_FALSE;
}

int array_insert(array_value_t *array, unsigned long long at, unsigned char type, signed long long int numeric, long double floating, char *string, array_value_t *array_nested)
{
	array_value_t *current = array;
	array_value_t *previous = NULL;
	if (current == NULL)
		fatal_error(ARRAY_TYPE_EXPECTED_ERROR);
	unsigned int size = 0;
	do
	{
		if (size == at)
		{
			array_value_t *array_insert = (array_value_t *)array_new();
			array_insert->result_type = type;
			if (size == 0)
			{
				unsigned char result_type = array_insert->result_type;
				array_insert->result_type = current->result_type;
				array_insert->value = current->value;
				array_insert->next = current->next;
				current->result_type = result_type;
				switch (array_insert->result_type)
				{
				case VARIABLE_INT:
				case VARIABLE_BOOLEAN:
				case VARIABLE_NULL:
					array_insert->value.numeric = numeric;
					break;
				case VARIABLE_DOUBLE:			
					array_insert->value.floating = floating;
					break;
				case VARIABLE_STRING:
				{
					array_insert->value.string = xmalloc(strlen(string) + 1);
					if (!copy_string(array_insert->value.string, string))
						fatal_error(MEMORY_ALLOCATION_ERROR);
					break;
				}
				case VARIABLE_ARRAY:
					array_insert->value.array = array;
					break;
				default:
					fatal_error(UNKNOWN_ERROR);
					break;
				}
				current->next = array_insert;
			}
			else
			{
				switch (array_insert->result_type)
				{
				case VARIABLE_INT:
				case VARIABLE_BOOLEAN:
				case VARIABLE_NULL:
					array_insert->value.numeric = numeric;
					break;
				case VARIABLE_DOUBLE:			
					array_insert->value.floating = floating;
					break;
				case VARIABLE_STRING:
				{
					array_insert->value.string = xmalloc(strlen(string) + 1);
					if (!copy_string(array_insert->value.string, string))
						fatal_error(MEMORY_ALLOCATION_ERROR);
					break;
				}
				case VARIABLE_ARRAY:
					array_insert->value.array = array;
					break;
				default:
					fatal_error(UNKNOWN_ERROR);
					break;
				}
				array_insert->next = current;
				previous->next = array_insert;
			}
			return BOOLEAN_TRUE;
		}
		size++;
		if (current->next == NULL && size == at)
		{
			array_value_t *array_insert = (array_value_t *)array_new();
			array_insert->result_type = type;
			switch (current->result_type)
			{
			case VARIABLE_INT:
			case VARIABLE_BOOLEAN:
			case VARIABLE_NULL:
				array_insert->value.numeric = numeric;
				break;
			case VARIABLE_DOUBLE:			
				array_insert->value.floating = floating;
				break;
			case VARIABLE_STRING:
			{
				array_insert->value.string = xmalloc(strlen(string) + 1);
				if (!copy_string(array_insert->value.string, string))
					fatal_error(MEMORY_ALLOCATION_ERROR);
				break;
			}
			case VARIABLE_ARRAY:
				array_insert->value.array = array;
				break;
			default:
				fatal_error(UNKNOWN_ERROR);
				break;
			}
			current->next = array_insert;
			return BOOLEAN_TRUE;
		}
		previous = current;
		current = current->next;
	} while (current != NULL);
	return BOOLEAN_FALSE;
}

array_value_t *array_get(array_value_t *array, unsigned long long at)
{
	array_value_t *current = array;
	if (current == NULL)
		fatal_error(ARRAY_TYPE_EXPECTED_ERROR);
	unsigned int size = 0;
	do
	{
		if (size == at)
			return current;
		size++;
		current = current->next;
	} while (size <= at && current != NULL);
	return NULL;
}

unsigned long long array_length(array_value_t *array)
{
	array_value_t *current = array;
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

array_value_t *array_duplicate(array_value_t *array)
{
	if (array == NULL)
		fatal_error(ARRAY_TYPE_EXPECTED_ERROR);
	array_value_t *array_copy = NULL;
	// typedef struct array_references
	// {
	// 	array_value_t *array;
	// 	array_value_t *parent;
	// 	struct array_references *next;
	// } array_references;
	// array_references *arrays = xmalloc(sizeof(array_references));
	// arrays->array = array;
	// arrays->next = NULL;
	// arrays->parent = NULL;
	// for (; arrays != NULL;)
	// {
	// 	array_references *current = arrays;
	// 	int is_nested = BOOLEAN_FALSE;
	// 	for (;;)
	// 	{
	// 		struct array_value_t *new_value = xmalloc(sizeof(struct array_value_t));
	// 		new_value->result_type = current->array->result_type;
	// 		// new_value->result_type = current->array->result_type;
	// 		new_value->next = NULL;
	// 		if (current->parent == NULL) // Top level array element
	// 		{
	// 			if (array_copy == NULL)
	// 				array_copy = new_value;
	// 			else
	// 			{
	// 				array_value_t *array_current = array_copy;
	// 				for (; array_current->next != NULL;)
	// 					array_current = array_current->next;
	// 				array_current->next = new_value;
	// 			}
	// 		}
	// 		else // Nested array
	// 		{
	// 			array_value_t *array_current = current->parent;
	// 			if (is_nested)
	// 			{
	// 				// struct array_value_t *array_nested = (array_value_t *)array_current->result.result_int;
	// 				// for (; array_nested->next != NULL;)
	// 				// 	array_nested = array_nested->next;
	// 				// array_nested->next = new_value;
	// 			}
	// 			else
	// 			{
	// 				// array_current->result.result_int = (long long int)&new_value;
	// 				is_nested = BOOLEAN_TRUE;
	// 			}
	// 		}
	// 		if (new_value->result_type == VARIABLE_ARRAY)
	// 		{
	// 			array_references *array_reference = xmalloc(sizeof(array_references));
	// 			// array_reference->array = (array_value_t *)new_value->result.result_int;
	// 			array_reference->parent = new_value;
	// 			array_references *arrays_appender = arrays;
	// 			for (; arrays_appender->next != NULL;)
	// 				arrays_appender = arrays_appender->next;
	// 			arrays_appender->next = array_reference;
	// 		}
	// 		current->array = current->array->next;
	// 		if (current->array == NULL) // End of current array
	// 			break;
	// 	}
	// 	arrays = arrays->next;
	// 	free(current);
	// }
	return array_copy;
}

#endif