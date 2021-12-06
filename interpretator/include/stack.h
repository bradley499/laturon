#ifndef stack_h
#define stack_h

#include "scope.h"

struct stack_t
{
	struct scope_t *parent;
	struct stack_t *previous;
	unsigned long long stack_position;
} __attribute__((__packed__));

void stack_add(struct scope_t *to_update, struct scope_t *parent);
void stack_clear(struct scope_t *to_update);
void stack_reset(struct scope_t *tree_root);

#endif