#ifndef parameter_h
#define parameter_h

#include "scope.h"

#define PARAMETER_PUSH_SUCCESS 0        // Successfully pushed data onto the parameter stack
#define PARAMETER_PUSH_FAILURE 1        // Failed to push data onto the parameter stack
#define PARAMETER_STACK_MAX_SIZE 10000  // The maximum size of the parameter stack

typedef struct parameter_t
{
	unsigned int function_scope_level;
	scope_t *parameter_scope;
} parameter_t;

// Removes all values from parameter stack
void clear_parameter_stack();
// Removes all values that are beyond the current scope level
void parameter_stack_parent_pop(unsigned short scope_level);
// Adds a value to the parameter stack
unsigned int parameter_stack_parent_push(unsigned short scope_level, scope_t *parameter);

#endif