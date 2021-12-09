#ifndef scope_c
#define scope_c

#include <stdlib.h>
#include <stdbool.h>

#include "scope.h"
#include "misc.h"

unsigned char scope_current_result_type = SCOPE_DEFAULT_VALUE;

scope_t *scope_new()
{
	scope_t *scope = (scope_t *)xmalloc(sizeof(scope_t)); // Attempts to request for memory
	scope->left = NULL;									  // Set scope left branch as undefined
	scope->right = NULL;								  // Set scope right branch as undefined
	scope->type = SCOPE_UNDEFINED;						  // Set scope type as undefined
	scope->result = NULL;								  // Set scope result as undefined
	scope->await = SCOPE_BOOLEAN_FALSE;					  // Set scope await to false
	return scope;
}

int scope_set_type(scope_t *scope, int set_type)
{
	if (scope->type != SCOPE_UNDEFINED)
		return SCOPE_FAILURE; // Return error if scope already is defined
	scope->type = set_type;	  // Set scope type
	return SCOPE_SUCCESS;
}

int scope_get_type(scope_t *scope)
{
	return scope->type;
}

void scope_set_result(scope_t *scope, double set_result)
{
	stack_result_push(scope, set_result, scope_current_result_type, (int)SCOPE_BOOLEAN_FALSE); // Set scope result
}

void scope_set_result_constant(scope_t *scope, double set_result)
{
	stack_result_push(scope, set_result, scope_current_result_type, (int)SCOPE_BOOLEAN_TRUE); // Set scope result
}

void scope_set_result_type(unsigned char set_result_type)
{
	scope_current_result_type = set_result_type;
}

double scope_get_result(scope_t *scope)
{
	return stack_result_pop(scope).result;
}

int scope_get_result_type(scope_t *scope)
{
	return stack_result_type(scope);
}

void scope_clear_result(scope_t *scope)
{
	switch (scope->type)
	{
	case SCOPE_TYPE_INT:
	case SCOPE_TYPE_BOOL:
	case SCOPE_TYPE_DOUBLE:
		break; // Do not reset for standard datatypes
	case SCOPE_TYPE_ARRAY:
		break;
	default:
		scope->result = 0; // Clear scope result;
	}
}

int scope_await(scope_t *scope)
{
	switch (scope->type)
	{
	case SCOPE_TYPE_INT:
	case SCOPE_TYPE_BOOL:
	case SCOPE_TYPE_DOUBLE:
	case SCOPE_VARIABLE:
	{
		scope->await = SCOPE_AWAIT_NO; // Is a constant value
		break;
	}
	default:
	{
		if (scope->await == SCOPE_AWAITING)
		{
			if (scope->right != NULL)
				scope->await = SCOPE_AWAITING_CONCLUSION;
			else
				scope->await = SCOPE_AWAIT_FINISHED;
		}
		else if (scope->await == SCOPE_AWAIT_FINISHED)
			scope->await = SCOPE_AWAITING; // Reset for new invocation
		else
			scope->await = SCOPE_AWAIT_FINISHED; // Has child operations
		break;
	}
	}
	if (scope->await == SCOPE_AWAITING_CONCLUSION && (scope->type == SCOPE_TYPE_FUNCTION_CALL || scope->type == SCOPE_TYPE_CONTAINER || scope->type == SCOPE_VARIABLE_UPDATE)) // Is a function call or a new container or variable update
		scope->await = SCOPE_AWAIT_FINISHED;																																   // Cancel scope value retension and call function instead
	return scope->await;
}

int scope_array_append(scope_t *scope_destination, scope_t *scope_source)
{
	scope_t *current_scope = scope_destination;
	scope_t *current_source_scope = scope_source;
	unsigned char appending_array = SCOPE_BOOLEAN_FALSE;
	if (scope_get_result_type(current_source_scope) == SCOPE_TYPE_ARRAY) // If appending an array
	{
		appending_array = SCOPE_BOOLEAN_TRUE;
		current_source_scope = current_source_scope->left; // Get first element within array
	}
	while (current_scope->left != NULL) // Repeat until the end of the array has been reached
		current_scope = current_scope->left;
	while (current_source_scope->left != NULL) // Repeat until the end of the array has been reached
	{
		scope_t *new_scope = scope_new();
		if (new_scope == NULL)
			return SCOPE_CREATION_FAILURE;
		scope_set_result_type(stack_result_type(current_source_scope));
		double value = stack_result_value(current_source_scope);
		scope_set_result(new_scope, value);
		scope_set_left(current_scope, new_scope);
		if (appending_array == SCOPE_BOOLEAN_FALSE)
			break;
		current_source_scope = current_source_scope->left;
		current_scope = new_scope;
	}
	return SCOPE_SUCCESS;
}

void scope_set_left(scope_t *scope, scope_t *set_scope)
{
	if (scope->left != NULL)		// If leftwards child node is ste
		scope_destroy(scope->left); // Remove leftwards child node
	scope->left = set_scope;		// Set leftwards child node
}

scope_t *scope_get_left(scope_t *scope)
{
	return scope->left; // Get leftwards child node
}

void scope_set_right(scope_t *scope, scope_t *set_scope)
{
	if (scope->right != NULL)		 // If rightwards child node is set
		scope_destroy(scope->right); // Remove rightwards child node
	scope->right = set_scope;		 // Set rightwards child node
}

scope_t *scope_get_right(scope_t *scope)
{
	return scope->right; // Get rightwards child node
}

void scope_destroy(scope_t *scope)
{
	if (scope == NULL) // Scope is empty
		return;
	scope_t *current_scope = scope;
	scope_t *previous_scope = NULL;
	for (;;)
	{
		scope_t *current_scope_left_child = current_scope->left;
		if (current_scope->await != SCOPE_AWAITING_DELETION)
		{
			current_scope->left = previous_scope;			// Store reference to parent scope
			current_scope->await = SCOPE_AWAITING_DELETION; // Declare scope to be deleted
			if (current_scope_left_child != NULL)
			{
				previous_scope = current_scope;
				current_scope = current_scope_left_child; // Set current scope to child scope
				continue;
			}
		}
		if (current_scope->right != NULL)
		{
			previous_scope = current_scope;
			current_scope = current_scope->right; // Set current scope to child scope
			previous_scope->right = NULL;		  // Remove reference to current branch
		}
		else
		{
			if (current_scope == scope) // Root scope has no children
				break;
			current_scope_left_child = current_scope->left;
			current_scope->left = NULL;	 // Removes reference to current scope's pointer
			current_scope->right = NULL; // Removes reference to current scope's pointer
			free(current_scope);		 // Free up child scope memory
			current_scope = current_scope_left_child;
			previous_scope = current_scope->left;
		}
	}
	free(scope); // Free up root scope memory
}

#endif