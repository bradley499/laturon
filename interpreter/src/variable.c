#ifndef variable_c
#define variable_c

#include <stdlib.h>

#include "variable.h"
#include "misc.h"

variable_t variables[VARIABLE_MAX_TOTAL];
static unsigned short total_variables = 0;

void variable_initialisation()
{
	variable_cleanup(0); // Remove all existing variables and prepare for new initialisation
	total_variables = 0; // Set total variables in use to 0
}

unsigned int new_variable(unsigned int execution_scope, unsigned int function_scope, unsigned int variable_id)
{
	if (total_variables >= VARIABLE_MAX_TOTAL_UNRESERVED)								// Too many variables have been declared in memory
		if (variable_id != VARIABLE_TEMPORARY || total_variables == VARIABLE_MAX_TOTAL) // Is not temporary variable or all variable space is used up
			return VARIABLE_UNASSIGNED;													// No variable is avaliable to be set
	short new_variable_position = 0;
	for (; new_variable_position < VARIABLE_MAX_TOTAL; new_variable_position++)
		if (variables[new_variable_position].function_scope == VARIABLE_UNASSIGNED)
			break;
	if (new_variable_position >= VARIABLE_MAX_TOTAL_UNRESERVED)							// No variable spaces avaliable
		if (variable_id != VARIABLE_TEMPORARY || total_variables == VARIABLE_MAX_TOTAL) // Is not temporary variable or all variable space is used up
			return VARIABLE_UNASSIGNED;
	variables[new_variable_position].value.type = VARIABLE_IS_UNASSIGNED;
	total_variables += 1;												// Increase total variables count
	variables[new_variable_position].execution_scope = execution_scope; // Set scope size
	variables[new_variable_position].function_scope = function_scope;	// Set function scope size
	variables[new_variable_position].variable_id = variable_id;			// Set variable hash value
	return VARIABLE_ASSIGNED;
}

void variable_refresh_scope(unsigned int variable_id, unsigned int function_scope, unsigned int execution_scope)
{
	for (unsigned short i = 0; i < VARIABLE_MAX_TOTAL; i++)
		if (variables[i].function_scope == function_scope)
			if (variables[i].variable_id == variable_id)
			{
				variables[i].execution_scope = execution_scope; // Refresh scope size
				return;
			}
}

void variable_delete(variable_id id)
{
	variables[id].function_scope = VARIABLE_UNASSIGNED;	 // Unassign variable function scope
	variables[id].execution_scope = VARIABLE_UNASSIGNED; // Unassign the execution scope
	variables[id].variable_id = VARIABLE_UNASSIGNED;	 // Unassign the variable hash scope
	if (variables[id].value.type == VARIABLE_STRING)	 // If has allocated value
		free(variables[id].value.contents.string);		 // Remove allocated value
	total_variables -= 1;								 // Decrease total variable count
}

variable_value_t variable_get_value(unsigned int variable_id, unsigned int function_scope)
{
	for (unsigned short i = 0; i < VARIABLE_MAX_TOTAL; i++)
		if (variables[i].function_scope == function_scope || variables[i].function_scope == VARIABLE_SCOPE_GLOBAL)
			if (variables[i].variable_id == variable_id)
				return variables[i].value;
	fatal_error(INVALID_SYNTAX_GENERIC);
}

void variable_cleanup(unsigned int execution_scope)
{
	for (unsigned short i = 0; i < VARIABLE_MAX_TOTAL; i++)
		if (((execution_scope == VARIABLE_SCOPE_GLOBAL) ? 1 : variables[i].execution_scope > execution_scope)) // If variable scope is destroyed
			variable_delete(i);																				   // Delete variable
}

int variable_name_valid(char *name)
{
	char character = '\0';
	if (!((name[0] >= 'A' && name[0] <= 'Z') || (name[0] >= 'a' && name[0] <= 'z')))
		return 0;
	for (int i = 1; (character = name[i]) != '\0'; i++)
	{
		if (!((character >= 'A' && character <= 'Z') || (character >= 'a' && character <= 'z') || (character >= '0' && character <= '9') || character == '_'))
			return 0;
	}
	return 1;
}

#endif