#ifndef variable_h
#define variable_h

#include <limits.h>

typedef long long variable_id;

#define VARIABLE_MAX_TOTAL USHRT_MAX												   // Total number of variables that is allowed to be used
#define VARIABLE_UNASSIGNED ((variable_id)1 << ((sizeof(variable_id) * CHAR_BIT) - 2)) // Variable is unassigned
#define VARIABLE_ASSIGNED 0															   // Variable has been assigned
#define VARIABLE_TEMPORARY 0														   // A temporary variable defintion
#define VARIABLE_SCOPE_GLOBAL 0														   // Variable's scope is global

#define VARIABLE_NUMERIC_REFERENCE_START TOTAL_DEFAULT_VALUES

typedef enum variable_type_t
{
	VARIABLE_IS_UNASSIGNED = 0,
	VARIABLE_INT,
	VARIABLE_DOUBLE,
	VARIABLE_STRING,
	VARIABLE_BOOLEAN,
	VARIABLE_ARRAY,
	VARIABLE_NULL,
	VARIABLE_END,
} variable_type_t;

#include "misc.h"
#include "array.h"

enum variable_id_type
{
	VARIABLE_ID_TYPE_REFERENCE = 0,
	VARIABLE_ID_TYPE_POSITIONAL = 1,
};

typedef struct variable_value_t
{
	variable_type_t type;
	union
	{
		signed long long int numeric;
		long double floating;
		char *string;
		struct array_value_t *array;
	} contents;
	variable_id variable_id_reference[2];
} variable_value_t;

typedef struct variable_t
{
	unsigned long long execution_scope;
	unsigned long long function_scope;
	variable_id variable_id;
	variable_value_t value;
} variable_t;

// Initialise variable array
void variable_initialisation();
// Create a new variable with referenced execution scope and function scope
unsigned short new_variable(unsigned long long execution_scope, unsigned long long function_scope, variable_id id);
// Delete variable reference
void variable_delete(unsigned short variable_position);
// Get value of variable
variable_value_t variable_get_value(variable_id id, unsigned long long function_scope, unsigned long long execution_scope);
// Set value of variable
void variable_set_value(unsigned short variable_position, char type, signed long long int numeric, long double floating, char *string, struct array_value_t *array);
// Check and declare variable assigned state
variable_id variable_value_assigned(variable_id id, int declare_as_defined);
// Get the variable host ID from position
variable_id variable_get_host_id(unsigned short variable_position);
// Clean up all variables within execution scope
void variable_cleanup(unsigned long long execution_scope);
// Validates if a name is a valid name for a variable or function
int variable_name_valid(char *name);

#endif