#ifndef parameter_h
#define parameter_h

#include "scope.h"

#define PARAMETER_PUSH_SUCCESS 0        // Successfully pushed data onto the parameter stack
#define PARAMETER_PUSH_FAILURE 1        // Failed to push data onto the parameter stack
#define PARAMETER_STACK_MAX_SIZE 10000  // The maximum size of the parameter stack

struct parameter_t
{
	unsigned int function_scope_level;
	struct scope_t *parameter_scope;
};

void clear_parameter_stack();															  // Removes all values from parameter stack
void pop_parameter_stack(unsigned short scope_level);									  // Removes all values that are beyond the current scope level
unsigned int push_parameter_stack(unsigned short scope_level, struct scope_t *parameter); // Adds a value to the parameter stack

#endif