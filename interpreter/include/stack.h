#ifndef stack_h
#define stack_h

#include <limits.h>

#include "scope.h"

struct stack_parent_t
{
	struct scope_t *parent;
	struct stack_parent_t *previous;
};

typedef struct stack_result_t
{
	double result;				// Value result
	unsigned char result_type;	// Value result type
	struct stack_result_t *previous;
} stack_result_t;

#define STACK_RESULT_TYPE_CONSTANT_BIT (1 << (__CHAR_BIT__ - 1))

typedef struct stack_parent_t stack_parent_t;

// Push scope parental reference to scope parent stack
void stack_parent_push(struct scope_t *to_update, struct scope_t *parent);
// Pop scope parental reference from scope parent stack
struct scope_t *stack_parent_pop(struct scope_t *to_update);
// Clear scope parent stack
void stack_parent_clear(struct scope_t *to_update);
// Push scope result to scope result stack
void stack_result_push(struct scope_t *to_update, double result_value, unsigned char result_type, int result_constant);
// Pop scope result from scope result stack
stack_result_t stack_result_pop(struct scope_t *to_update);
// Get type from the most recent result stack value
unsigned char stack_result_type(struct scope_t *to_update);
// Get state of result stack being a constant value
unsigned char stack_result_type_is_constant(struct scope_t *to_update);
// Get value from the most recent result stack value
unsigned char stack_result_value(struct scope_t *to_update);
// Get value from the most recent result stack value
unsigned char stack_result_value(struct scope_t *to_update);
// Clear scope result stack
void stack_result_clear(struct scope_t *to_update);

#endif