#ifndef run_h
#define run_h

#include <limits.h>
#include "parse.h"
#include "variable.h"

typedef struct stack_value_t
{
	variable_value_t *value;
	char operation_type;
	token_t *token;
} stack_value_t;

#define EXECUTION_STACK_MAX_MEMORY_SIZE 30000
#define EXECUTION_STACK_SIZE (EXECUTION_STACK_MAX_MEMORY_SIZE / sizeof(stack_value_t))

// Reset the execution stack
void run_stack_reset();
// Execute the program
int run(parsed_function_scope_t **functions);

#endif