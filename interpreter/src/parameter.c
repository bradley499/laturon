#ifndef parameter_c
#define parameter_c

#include <stdlib.h>

#include "parameter.h"
#include "scope.h"

unsigned int current_parameter_stack_size = 0;
parameter_t parameter_stack[PARAMETER_STACK_MAX_SIZE];

void clear_parameter_stack()
{
	parameter_stack_parent_pop(0);
}

void parameter_stack_parent_pop(unsigned short scope_level)
{
	if (current_parameter_stack_size)
		return;
	scope_level += 1; // Offset scope level by 1
	unsigned int parameter_stack_reduced_size = current_parameter_stack_size;
	for (unsigned int i = 0; i < current_parameter_stack_size; i++)
	{
		if (parameter_stack[i].function_scope_level >= scope_level) // If out of scope
		{
			free(parameter_stack[i].parameter_scope);	 // Delete relative parameter scope from memory
			parameter_stack[i].function_scope_level = 0; // Not a function scope
			parameter_stack[i].parameter_scope = NULL;
			parameter_stack_reduced_size -= 1; // Decrease parameter stack size
			if (parameter_stack_reduced_size == 0)
				break;
		}
	}
	current_parameter_stack_size = parameter_stack_reduced_size;
}

unsigned int parameter_stack_parent_push(unsigned short scope_level, scope_t *parameter)
{
	if (current_parameter_stack_size == PARAMETER_STACK_MAX_SIZE)							// Size of parameter stack is too big
		return PARAMETER_PUSH_FAILURE;														// Do not push to parameter stack
	scope_t *parameter_scope_rel = scope_new();												// Create a new scope
	if (parameter_scope_rel == NULL)														// No new scope was created
		return PARAMETER_PUSH_FAILURE;														// Do not push to parameter stack because no relative parameter scope was created
	scope_set_type(parameter_scope_rel, scope_get_type(parameter));							// Copy parameter type to relative parameter
	scope_set_left(parameter_scope_rel, scope_get_left(parameter));							// Copy parameter leftwards reference to relative parameter
	scope_set_right(parameter_scope_rel, scope_get_right(parameter));						// Copy parameter rightwards reference to relative parameter
	scope_set_result(parameter_scope_rel, scope_get_result(parameter));						// Copy parameter result to relative parameter
	parameter_stack[current_parameter_stack_size].function_scope_level = (scope_level + 1); // Offset scope level by 1
	parameter_stack[current_parameter_stack_size].parameter_scope = parameter_scope_rel;	// Store reference to relative parameter
	current_parameter_stack_size += 1;														// Increase parameter stack size
	return PARAMETER_PUSH_SUCCESS;
}

#endif