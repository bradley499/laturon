#ifndef variable_c
#define variable_c

#include <stddef.h>

#include "variable.h"
#include "scope.h"

variable variables[VARIABLE_MAX_TOTAL];
static unsigned short total_variables = 0;

void variable_initialisation()
{
	cleanup(0);			 // Remove all existing variables and prepare for new initialisation
	total_variables = 0; // Set total variables in use to 0
}

unsigned int new_variable(unsigned int execution_scope, unsigned int function_scope, unsigned int variable_hash)
{
	if (total_variables >= VARIABLE_MAX_TOTAL_UNRESERVED)								  // Too many variables have been declared in memory
		if (variable_hash != VARIABLE_TEMPORARY || total_variables == VARIABLE_MAX_TOTAL) // Is not temporary variable or all variable space is used up
			return VARIABLE_UNASSIGNED;													  // No variable is avaliable to be set
	short new_variable_position = 0;
	for (; new_variable_position < VARIABLE_MAX_TOTAL; new_variable_position++)
		if (variables[new_variable_position].function_scope == VARIABLE_UNASSIGNED)
			break;
	if (new_variable_position >= VARIABLE_MAX_TOTAL_UNRESERVED)							  // No variable spaces avaliable
		if (variable_hash != VARIABLE_TEMPORARY || total_variables == VARIABLE_MAX_TOTAL) // Is not temporary variable or all variable space is used up
			return VARIABLE_UNASSIGNED;
	variables[new_variable_position].scope = scope_new();
	if (variables[new_variable_position].scope == NULL)
		return VARIABLE_UNASSIGNED;
	total_variables += 1;												// Increase total variables count
	variables[new_variable_position].execution_scope = execution_scope; // Set scope size
	variables[new_variable_position].function_scope = function_scope;	// Set function scope size
	variables[new_variable_position].variable_hash = variable_hash;		// Set variable hash value
	return VARIABLE_ASSIGNED;
}

void refresh_variable_scope(unsigned int variable_hash, unsigned int function_scope, unsigned int execution_scope)
{
	for (unsigned short i = 0; i < VARIABLE_MAX_TOTAL; i++)
		if (variables[i].function_scope == function_scope)
			if (variables[i].variable_hash == variable_hash)
			{
				variables[i].execution_scope = execution_scope; // Refresh scope size
				return;
			}
}

void delete_variable(variable_id id)
{
	variables[id].function_scope = VARIABLE_UNASSIGNED;	 // Unassign variable function scope
	variables[id].execution_scope = VARIABLE_UNASSIGNED; // Unassign the execution scope
	variables[id].variable_hash = VARIABLE_UNASSIGNED;	 // Unassign the variable hash scope
	if (variables[id].scope != NULL)					 // If points to a scope
		scope_destroy(variables[id].scope);				 // Remove scope pointer
	variables[id].scope = NULL;							 // Declare an unassigned scope
	total_variables -= 1;								 // Decrease total variable count
}

struct scope_t *variable_get_scope(unsigned int variable_hash, unsigned int function_scope)
{
	for (unsigned short i = 0; i < VARIABLE_MAX_TOTAL; i++)
		if (variables[i].function_scope == function_scope || variables[i].function_scope == VARIABLE_SCOPE_GLOBAL)
			if (variables[i].variable_hash == variable_hash)
				return variables[i].scope;
	return NULL;
}

void cleanup(unsigned int execution_scope)
{
	for (unsigned short i = 0; i < VARIABLE_MAX_TOTAL; i++)
		if (((execution_scope == VARIABLE_SCOPE_GLOBAL) ? 1 : variables[i].execution_scope > execution_scope)) // If variable scope is destroyed
			delete_variable(i);																				   // Delete variable
}

#endif