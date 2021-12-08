#ifndef variable_h
#define variable_h

#include "scope.h"

#define VARIABLE_MAX_TOTAL            9999                                                // Total number of variables that is allowed to be used
#define VARIABLE_MAX_TOTAL_RESERVED   99                                                  // Total number of variables that is reserved for temporary variables
#define VARIABLE_MAX_TOTAL_UNRESERVED (VARIABLE_MAX_TOTAL - VARIABLE_MAX_TOTAL_RESERVED)  // Total number of variables that is not reserved for temporary variables
#define VARIABLE_UNASSIGNED           0                                                   // Variable is unassigned
#define VARIABLE_ASSIGNED             1                                                   // Variable has been assigned
#define VARIABLE_TEMPORARY            0                                                   // A temporary variable defintion
#define VARIABLE_SCOPE_GLOBAL         0                                                   // Variable's scope is global

#define variable_get_result(variable_hash, function_scope) scope_get_result(variable_get_scope(variable_hash, function_scope))
#define variable_get_result_string(variable_hash, function_scope) scope_get_result_string(variable_get_scope(variable_hash, function_scope))
#define variable_set_result(variable_hash, function_scope, set_result) scope_set_result(variable_get_scope(variable_hash, function_scope), set_result)
#define variable_set_result_string(variable_hash, function_scope, set_result_string) scope_set_result_string(variable_get_scope(variable_hash, function_scope), set_result_string)
#define variable_get_result_type(variable_hash, function_scope) scope_get_result_type(variable_get_scope(variable_hash, function_scope))
#define variable_set_result_type(variable_hash, function_scope, set_result_type) scope_set_result_type(variable_get_scope(variable_hash, function_scope), set_result_type)

typedef struct variable
{
	unsigned int execution_scope;
	unsigned int function_scope;
	unsigned int variable_hash;
	struct scope_t *scope;
} variable;

typedef unsigned short variable_id;

// Initialise variable array
void variable_initialisation();
// Create a new variable with referenced execution scope and function scope
unsigned int new_variable(unsigned int execution_scope, unsigned int function_scope, unsigned int variable_hash);
// Change the scope of a variable
void refresh_variable_scope(unsigned int variable_hash, unsigned int function_scope, unsigned int execution_scope);
// Delete variable reference
void delete_variable(variable_id id);
// Get scope of variable
struct scope_t *variable_get_scope(unsigned int variable_hash, unsigned int function_scope);
// Clean up all variables within execution scope
void cleanup(unsigned int execution_scope);

#endif