#ifndef stack_c
#define stack_c

#include <stdlib.h>
#include "stack.h"
#include "scope.h"
#include "misc.h"

static unsigned long long stack_position = 0;

void stack_push(struct scope_t *to_update, struct scope_t *parent)
{
	struct stack_t *current = xmalloc(sizeof(struct stack_t));
	current->parent = parent;
	if (to_update->call_stack == NULL)
		to_update->call_stack = current;
	else
	{
		struct stack_t *previous = to_update->call_stack->previous;
		current->previous = previous;
		to_update->call_stack = current;
	}
}

struct scope_t * stack_pop(struct scope_t *to_update)
{
	if (to_update == NULL)
		fatal_error(STACK_REFERENCE_ERROR);
	struct scope_t *scope = NULL;
	if (to_update->call_stack != NULL)
	{
		scope = to_update->call_stack->parent;
		struct stack_t *previous = to_update->call_stack->previous;
		free(to_update->call_stack);
		to_update->call_stack = previous;
	}
	if (scope == NULL)
		fatal_error(STACK_REFERENCE_ERROR);
	return scope;
}

void stack_clear(struct scope_t *to_update)
{
	if (to_update->call_stack == NULL)
		return;
	for (;to_update->call_stack != NULL;)
		stack_pop(to_update);
}

void stack_reset(struct scope_t *tree_root)
{
	stack_position = 0;
	stack_clear(tree_root);
}

#endif