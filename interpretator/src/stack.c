#ifndef stack_c
#define stack_c

#include "stack.h"

static unsigned long long stack_position = 0;

void stack_add(struct scope_t *to_update, struct scope_t *parent)
{

}

void stack_clear(struct scope_t *to_update)
{

}

void stack_reset(struct scope_t *tree_root)
{
	stack_position = 0;
	stack_clear(tree_root);
}

#endif