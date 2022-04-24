#ifndef variable_c
#define variable_c

#include <stdlib.h>
#include <string.h>
#include "variable.h"
#include "misc.h"

variable_t variables[VARIABLE_MAX_TOTAL];
static unsigned short total_variables = 0;

void variable_initialisation()
{
	variable_cleanup(0);																			 // Remove all existing variables and prepare for new initialisation
	total_variables = 0;																			 // Set total variables in use to 0
	variable_set_value(new_variable(0, 0, BOOLEAN_FALSE), VARIABLE_BOOLEAN, BOOLEAN_FALSE, 0, 0, 0); // Create boolean false variable
	variable_set_value(new_variable(0, 0, BOOLEAN_TRUE), VARIABLE_BOOLEAN, BOOLEAN_TRUE, 0, 0, 0);	 // Create boolean true variable
	variable_set_value(new_variable(0, 0, VARIABLE_NULL), VARIABLE_NULL, BOOLEAN_FALSE, 0, 0, 0);	 // Create null variable
}

unsigned short new_variable(unsigned int execution_scope, unsigned int function_scope, variable_id id)
{
	if (total_variables >= VARIABLE_MAX_TOTAL) // Too many variables have been declared in memory
		fatal_error(TOO_BIG_NUMERIC);
	unsigned int new_variable_position = 0;
	for (; new_variable_position < VARIABLE_MAX_TOTAL; new_variable_position++)
		if (variables[new_variable_position].function_scope == VARIABLE_UNASSIGNED)
			break;
	if (new_variable_position >= VARIABLE_MAX_TOTAL) // No variable spaces avaliable
		fatal_error(TOO_BIG_NUMERIC);
	variables[new_variable_position].value.type = VARIABLE_IS_UNASSIGNED;
	variables[new_variable_position].value.variable_id_reference[VARIABLE_ID_TYPE_REFERENCE] = id;
	variables[new_variable_position].value.variable_id_reference[VARIABLE_ID_TYPE_POSITIONAL] = new_variable_position;
	total_variables += 1;																		  // Increase total variables count
	variables[new_variable_position].execution_scope = execution_scope;							  // Set scope size
	variables[new_variable_position].function_scope = function_scope;							  // Set function scope size
	variables[new_variable_position].variable_id = (new_variable_position | VARIABLE_UNASSIGNED); // Set variable id reference value
	return new_variable_position;
}

void variable_refresh_scope(variable_id id, unsigned int function_scope, unsigned int execution_scope)
{
	for (unsigned int i = 0; i < VARIABLE_MAX_TOTAL; i++)
		if (variables[i].function_scope == function_scope)
			if (variables[i].variable_id == id)
			{
				variables[i].execution_scope = execution_scope; // Refresh scope size
				return;
			}
}

void variable_delete(unsigned short variable_position)
{
	variables[variable_position].function_scope = VARIABLE_UNASSIGNED;															  // Unassign variable function scope
	variables[variable_position].execution_scope = VARIABLE_UNASSIGNED;															  // Unassign the execution scope
	variables[variable_position].variable_id = VARIABLE_UNASSIGNED;																  // Unassign the variable hash scope
	if (variables[variable_position].value.type == VARIABLE_STRING && variables[variable_position].value.contents.string != NULL) // If has allocated value
		free(variables[variable_position].value.contents.string);																  // Remove allocated value
	total_variables -= 1;																										  // Decrease total variable count
}

variable_value_t variable_get_value(variable_id id, unsigned int function_scope, unsigned int execution_scope)
{
	for (unsigned int i = 0; i < VARIABLE_MAX_TOTAL; i++)
		if (variables[i].function_scope == function_scope || variables[i].function_scope == VARIABLE_SCOPE_GLOBAL)
			if (variables[i].value.variable_id_reference[VARIABLE_ID_TYPE_REFERENCE] == id)
				return variables[i].value;
	return variables[new_variable(execution_scope, function_scope, id)].value; // Create new and return value if not exists
}

void variable_set_value(unsigned short variable_position, char type, signed long long int numeric, long double floating, char *string, struct array_value_t *array)
{
	if (variable_position >= VARIABLE_MAX_TOTAL)
		fatal_error(STACK_REFERENCE_ERROR);
	if (variables[variable_position].value.type == VARIABLE_STRING && variables[variable_position].value.contents.string != NULL)
		free(variables[variable_position].value.contents.string);
	switch (type)
	{
	case VARIABLE_INT:
	case VARIABLE_BOOLEAN:
	case VARIABLE_NULL:
		variables[variable_position].value.contents.numeric = numeric;
		break;
	case VARIABLE_DOUBLE:
		variables[variable_position].value.contents.floating = floating;
		break;
	case VARIABLE_STRING:
	{
		variables[variable_position].value.contents.string = xmalloc(strlen(string) + 1);
		if (!copy_string(variables[variable_position].value.contents.string, string))
			fatal_error(MEMORY_ALLOCATION_ERROR);
		break;
	}
	case VARIABLE_ARRAY:
		variables[variable_position].value.contents.array = array;
		break;
	default:
		fatal_error(UNKNOWN_ERROR);
		break;
	}
	variables[variable_position].value.type = type;
	variables[variable_position].variable_id = variable_value_assigned(variables[variable_position].variable_id, 1); // Declare variable as assigned
}

variable_id variable_value_assigned(variable_id id, int declare_as_defined)
{
	if (declare_as_defined == 1)
		return (id & ~VARIABLE_UNASSIGNED); // Remove unassigned bit from variable name
	else
		return ((id & VARIABLE_UNASSIGNED) != VARIABLE_UNASSIGNED); // Variable is not unassigned
}

variable_id variable_get_host_id(unsigned short variable_position)
{
	return variables[variable_position].variable_id;
}

void variable_cleanup(unsigned int execution_scope)
{
	for (unsigned int i = 0; i < VARIABLE_MAX_TOTAL; i++)
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