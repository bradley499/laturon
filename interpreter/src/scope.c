#ifndef scope_c
#define scope_c

#include <stdlib.h>
#include <string.h>

#include "scope.h"
#include "misc.h"

struct scope_t *scope_new()
{
	struct scope_t *scope = NULL;
	scope = (struct scope_t *)malloc(sizeof(struct scope_t)); // Attempts to request for memory
	if (scope != NULL)										  // No memory was allocated
	{
		scope->left = NULL;							 // Set scope left branch as undefined
		scope->right = NULL;						 // Set scope right branch as undefined
		scope->type = SCOPE_UNDEFINED;				 // Set scope type as undefined
		scope->result_type = SCOPE_UNDEFINED;		 // Set scope result type as undefined
		scope->result.numeric = SCOPE_DEFAULT_VALUE; // Set scope result as undefined
		scope->await = SCOPE_BOOLEAN_FALSE;			 // Set scope await to false
	}
	return scope;
}

int scope_set_type(struct scope_t *scope, int set_type)
{
	if (scope->type != SCOPE_UNDEFINED)
		return SCOPE_FAILURE; // Return error if scope already is defined
	scope->type = set_type;	  // Set scope type
	return SCOPE_SUCCESS;
}

int scope_get_type(struct scope_t *scope)
{
	return scope->type;
}

void scope_set_result(struct scope_t *scope, double set_result)
{
	scope_set_result_type(scope, SCOPE_TYPE_DOUBLE);
	scope->result.numeric = set_result; // Set scope result
}

unsigned char scope_set_result_string(struct scope_t *scope, char *str)
{
	unsigned long string_length = strlen(str);
	scope_set_result_type(scope, SCOPE_TYPE_STRING);
	scope->result.string = NULL;
	scope->result.string = new_string_size((string_length + 1)); // Allocate memory for new string
	if (scope->result.string == NULL)							 // Failed to allocate memory for new string
	{
		scope->result_type = SCOPE_UNDEFINED;
		return SCOPE_CREATION_FAILURE;
	}
	scope->result.string[string_length] = '\0';
	memcpy(scope->result.string, str, string_length); // Set scope value to char array pointer
	return SCOPE_SUCCESS;
}

void scope_set_result_type(struct scope_t *scope, int set_result_type)
{
	if (scope->result_type == set_result_type)
		return;
	if (scope->result_type == SCOPE_TYPE_STRING)
	{
		if (scope->result.string != NULL)
			free(scope->result.string); // Free up memory for current string
		scope->result.string = NULL;
	}
	scope->result_type = set_result_type; // Set scope result type
}

double scope_get_result(struct scope_t *scope)
{
	if (scope->result_type != SCOPE_TYPE_STRING || scope->type != SCOPE_UNDEFINED)
		return SCOPE_DEFAULT_VALUE;
	return scope->result.numeric; // Get scope result
}

char *scope_get_result_string(struct scope_t *scope)
{
	if (scope->result_type != SCOPE_TYPE_STRING)
		return NULL;
	return scope->result.string; // Get scope string result
}

int scope_get_result_type(struct scope_t *scope)
{
	return scope->result_type; // Get scope result type
}

void scope_clear_result(struct scope_t *scope)
{
	switch (scope->type)
	{
	case SCOPE_TYPE_INT:
	case SCOPE_TYPE_BOOL:
	case SCOPE_TYPE_DOUBLE:
	case SCOPE_TYPE_STRING:
		break; // Do not reset for standard datatypes
	case SCOPE_TYPE_ARRAY:
		break;
	default:
		scope->result.numeric = 0; // Clear scope result;
	}
}

int scope_await(struct scope_t *scope)
{
	switch (scope->type)
	{
	case SCOPE_TYPE_INT:
	case SCOPE_TYPE_BOOL:
	case SCOPE_TYPE_DOUBLE:
	case SCOPE_TYPE_STRING:
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

int scope_array_append(struct scope_t *scope_destination, struct scope_t *scope_source)
{
	struct scope_t *current_scope = scope_destination;
	struct scope_t *current_source_scope = scope_source;
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
		struct scope_t *new_scope = scope_new();
		if (new_scope == NULL)
			return SCOPE_CREATION_FAILURE;
		scope_set_result_type(new_scope, scope_get_result_type(current_source_scope));
		scope_set_result(new_scope, scope_get_result(current_source_scope));
		scope_set_left(current_scope, new_scope);
		if (appending_array == SCOPE_BOOLEAN_FALSE)
			break;
		current_source_scope = current_source_scope->left;
		current_scope = new_scope;
	}
	return SCOPE_SUCCESS;
}

void scope_set_left(struct scope_t *scope, struct scope_t *set_scope)
{
	if (scope->left != NULL)		// If leftwards child node is ste
		scope_destroy(scope->left); // Remove leftwards child node
	scope->left = set_scope;		// Set leftwards child node
}

struct scope_t *scope_get_left(struct scope_t *scope)
{
	return scope->left; // Get leftwards child node
}

void scope_set_right(struct scope_t *scope, struct scope_t *set_scope)
{
	if (scope->right != NULL)		 // If rightwards child node is set
		scope_destroy(scope->right); // Remove rightwards child node
	scope->right = set_scope;		 // Set rightwards child node
}

struct scope_t *scope_get_right(struct scope_t *scope)
{
	return scope->right; // Get rightwards child node
}

void scope_destroy(struct scope_t *scope)
{
	if (scope == NULL) // Scope is empty
		return;
	struct scope_t *current_scope = scope;
	struct scope_t *previous_scope = NULL;
	for (;;)
	{
		struct scope_t *current_scope_left_child = current_scope->left;
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
			if (current_scope->result_type == SCOPE_TYPE_STRING)
			{
				if (current_scope->result.string != NULL)
					free(current_scope->result.string); // Free up memory for current string
			}
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