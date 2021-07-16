#ifndef variable_h
#define variable_h

#include "scope.h"

#define VARIABLE_MAX_TOTAL       9999  // Total number of variables that is allowed to be used
#define VARIABLE_UNASSIGNED      0     // Variable is unassigned
#define VARIABLE_ASSIGN_SUCCESS  1     // Variable has been assigned

#define variable_get_result(a) scope_get_result(variable_get_scope(a))
#define variable_get_result_string(a) scope_get_result_string(variable_get_scope(a))
#define variable_set_result(a, b) scope_set_result(variable_get_scope(a), b)
#define variable_set_result_string(a, b) scope_set_result_string(variable_get_scope(a), b)
#define variable_get_result_type(a) scope_get_result_type(variable_get_scope(a))
#define variable_set_result_type(a, b) scope_set_result_type(variable_get_scope(a), b)

typedef struct variable
{
	unsigned int execution_scope;
	unsigned int function_scope;
	struct scope_t *scope;
} variable;

typedef unsigned short variable_id;

void variable_initialisation();
variable_id new_variable(unsigned int scope, unsigned int function_scope);
void refresh_variable_scope(variable_id id, unsigned int scope);
void delete_variable(variable_id id);
struct scope_t *variable_get_scope(variable_id id);
void cleanup(unsigned int scope);

#endif