#ifndef variable_c
#define variable_c

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "variable.h"
#include "scope.h"

variable variables[VARIABLE_MAX_TOTAL];
static unsigned short total_variables = 0;

void variable_initialisation()
{
	for (variable_id i = 0; i < VARIABLE_MAX_TOTAL; i++)
	{
		if (variables[i].function_scope == VARIABLE_UNASSIGNED && variables[i].execution_scope == VARIABLE_UNASSIGNED && variables[i].scope == NULL)
			delete_variable(i);
		variables[i].function_scope = VARIABLE_UNASSIGNED;	// Unassign variable function scope
		variables[i].execution_scope = VARIABLE_UNASSIGNED; // Unassign the execution scope
		variables[i].scope = NULL;							// Declare an unassigned scope
	}
}

variable_id new_variable(unsigned int scope, unsigned int function_scope)
{
	if (total_variables == VARIABLE_MAX_TOTAL) // Too many variables have been declared in memory
		return VARIABLE_UNASSIGNED;			   // No variable is avaliable to be set
	short new_variable_position = 0;
	for (; new_variable_position < VARIABLE_MAX_TOTAL; new_variable_position++)
		if (variables[new_variable_position].function_scope == VARIABLE_UNASSIGNED)
			break;
	if (new_variable_position == VARIABLE_MAX_TOTAL) // No variable spaces avaliable
		return VARIABLE_UNASSIGNED;
	variables[new_variable_position].scope = scope_new();
	if (variables[new_variable_position].scope == NULL)
		return VARIABLE_UNASSIGNED;
	total_variables += 1;											  // Increase total variables count
	variables[new_variable_position].execution_scope = scope;		  // Set scope size
	variables[new_variable_position].function_scope = function_scope; // Set function scope size
	return (new_variable_position + 1);
}

void refresh_variable_scope(variable_id id, unsigned int scope)
{
	if (id-- == VARIABLE_UNASSIGNED)
		return;
	if (variables[id].function_scope != VARIABLE_UNASSIGNED)
		variables[id].execution_scope = scope; // Refresh scope size
}

void delete_variable(variable_id id)
{
	if (id-- == VARIABLE_UNASSIGNED)
		return;
	variables[id].function_scope = VARIABLE_UNASSIGNED; // Unassign variable function scope
	variables[id].execution_scope = VARIABLE_UNASSIGNED;
	if (variables[id].scope != NULL)
		scope_destroy(variables[id].scope);
	variables[id].scope = NULL;
	total_variables -= 1; // Decrease total variable count
}

struct scope_t *variable_get_scope(variable_id id)
{
	if (id-- == VARIABLE_UNASSIGNED)
		return NULL;
	return variables[id].scope;
}

void cleanup(unsigned int scope)
{
	for (variable_id i = 0; i < VARIABLE_MAX_TOTAL; i++)
		if (((scope == 0) ? 1 : variables[i].execution_scope > scope)) // If variable scope is destroyed
			delete_variable(i);										   // Delete variable
}

#endif