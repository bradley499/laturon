#ifndef stack_h
#define stack_h

#include "scope.h"

struct stack_t
{
	struct scope_t *parent;
	struct stack_t *previous;
} __attribute__((__packed__));

// Push scope parental reference to scope stack
void stack_push(struct scope_t *to_update, struct scope_t *parent);
// Pop scope parental reference from scope stack
struct scope_t *stack_pop(struct scope_t *to_update);
// Clear scope stack
void stack_clear(struct scope_t *to_update);
// Reset scope from root
void stack_reset(struct scope_t *tree_root);

#endif