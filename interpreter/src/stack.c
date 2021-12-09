#ifndef stack_c
#define stack_c

#include <stdlib.h>
#include "stack.h"
#include "scope.h"
#include "misc.h"

void stack_parent_push(scope_t *to_update, scope_t *parent)
{
	stack_parent_t *current = xmalloc(sizeof(stack_parent_t));
	current->parent = parent;
	if (to_update->call_stack == NULL)
		to_update->call_stack = current;
	else
	{
		stack_parent_t *previous = to_update->call_stack->previous;
		current->previous = previous;
		to_update->call_stack = current;
	}
}

scope_t * stack_parent_pop(scope_t *to_update)
{
	if (to_update == NULL)
		fatal_error(STACK_REFERENCE_ERROR);
	scope_t *scope = NULL;
	if (to_update->call_stack != NULL)
	{
		scope = to_update->call_stack->parent;
		stack_parent_t *previous = to_update->call_stack->previous;
		free(to_update->call_stack);
		to_update->call_stack = previous;
	}
	if (scope == NULL)
		fatal_error(STACK_REFERENCE_ERROR);
	return scope;
}

void stack_parent_clear(scope_t *to_update)
{
	if (to_update->call_stack == NULL)
		return;
	for (;to_update->call_stack != NULL;)
		stack_parent_pop(to_update);
}

void stack_result_push(struct scope_t *to_update, double result_value, unsigned char result_type, int result_constant)
{
	stack_result_t *current = xmalloc(sizeof(stack_result_t));
	current->result_type = result_type;
	current->result = result_value;
	if (result_constant)
		current->result_type |= STACK_RESULT_TYPE_CONSTANT_BIT;
	if (to_update->result == NULL)
		to_update->result = current;
	else
	{
		stack_result_t *previous = to_update->result->previous;
		current->previous = previous;
		to_update->result = current;
	}
}

stack_result_t stack_result_pop(struct scope_t *to_update)
{
	if (to_update->result == NULL)
		fatal_error(MISSING_COMPOUND_LITERAL);
	stack_result_t value = {
		.result = to_update->result->result,
		.result_type = to_update->result->result_type,
	};
	if (!(value.result_type & STACK_RESULT_TYPE_CONSTANT_BIT))
	{
		stack_result_t *previous = to_update->result->previous;
		free(to_update->result);
		to_update->result = previous;
	}
	value.result_type &= ~STACK_RESULT_TYPE_CONSTANT_BIT;
	return value;
}

unsigned char stack_result_type(struct scope_t *to_update)
{
	if (to_update->result == NULL)
		fatal_error(MISSING_COMPOUND_LITERAL);
	return to_update->result->result_type;
}

unsigned char stack_result_type_is_constant(struct scope_t *to_update)
{
	unsigned char constant = 0;
	if (to_update->result != NULL)
		if ((to_update->result->result_type & STACK_RESULT_TYPE_CONSTANT_BIT))
			constant = 1;
	return constant;
}

unsigned char stack_result_value(struct scope_t *to_update)
{
	if (to_update->result == NULL)
		fatal_error(MISSING_COMPOUND_LITERAL);
	return to_update->result->result;
}

void stack_result_clear(struct scope_t *to_update)
{
	if (to_update->result == NULL)
		return;
	for (;to_update->result != NULL;)
	{
		to_update->result->result_type = 0; // Undefine the result type (resulting in all constant declarations being lost)
		stack_result_pop(to_update);
	}
}

#endif