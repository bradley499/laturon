#ifndef run_c
#define run_c

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "scope.h"
#include "run.h"
#include "misc.h"
#include "parameter.h"
#include "variable.h"
#include "stack.h"

struct position *position_tree = NULL;
unsigned int execution_scope = 0;
unsigned int function_scope = 0;
unsigned char function_scope_parameter = RUN_PARAMETER_STACK_CLOSED;

int new_tree(int branch_position)
{
	struct position *position_tree_new_node = new_branch(); // Request a new branch
	if (position_tree_new_node == NULL)						// No branch was given
		return RUN_POSITION_TREE_FAILURE;
	switch (branch_position)
	{
	case RUN_POSITION_BRANCH_LEFT: // Branch position left
		position_tree_new_node->branch = RUN_POSITION_BRANCH_LEFT;
		break;
	case RUN_POSITION_BRANCH_RIGHT: // Branch position right
		position_tree_new_node->branch = RUN_POSITION_BRANCH_RIGHT;
		break;
	default: // Branch position both
		position_tree_new_node->branch = RUN_POSITION_BRANCH_NULL;
		break;
	}
	if (position_tree == NULL) // If there is no root node
	{
		position_tree = position_tree_new_node; // Set root node
		return RUN_SUCCESS;
	}
	struct position *position_tree_current_node = position_tree;
	do
	{
		position_tree_current_node = position_tree_current_node->next; // Get child node
	} while (position_tree_current_node->next != NULL);				   // If child node does exist
	position_tree_current_node->next = position_tree_new_node;		   // Add new node to end of current node
	return RUN_SUCCESS;
}

struct position *new_branch()
{
	struct position *position_tree_node = (struct position *)malloc(sizeof(struct position)); // Allocate a new position node
	if (position_tree_node != NULL)															  // If a position node was allocated
	{
		position_tree_node->branch = RUN_POSITION_BRANCH_NULL; // Set branch direction to null
		position_tree_node->next = NULL;					   // Set branch child to null
	}
	return position_tree_node;
}

void delete_tree(struct position *position_tree_node)
{
	if (position_tree_node->next != NULL)	   // If has children
		delete_tree(position_tree_node->next); // Delete children
	free(position_tree_node);				   // Free up memory
}

void decrease_tree(unsigned int size)
{
	if (position_tree == NULL)
		return;
	struct position *position_tree_previous_node = NULL;
	struct position *position_tree_current_node = position_tree;
	do
	{
		position_tree_previous_node = position_tree_current_node;	   // Set parent node to current node
		position_tree_current_node = position_tree_current_node->next; // Get child node
	} while (position_tree_current_node != NULL);					   // If child node does exist
	free(position_tree_current_node);								   // Free up memory
	position_tree_previous_node->next = NULL;						   // Set child node to null
	if (size > 0)
		decrease_tree((size - 1)); // Delete parent
}

int step(struct scope_t *scope)
{
	if (scope == NULL)
		return RUN_STEP_FINISHED;
	int scope_status = scope_await(scope);
	if (scope_status == SCOPE_AWAITING_DELETION)
		return RUN_FAILURE_CLEANING_UP;
	else if (scope_status != SCOPE_AWAIT_NO)
	{
		if (scope_status == SCOPE_AWAITING)
		{
			new_tree(RUN_POSITION_BRANCH_LEFT);		// Calculate leftwards value
			if (scope->type == SCOPE_TYPE_FUNCTION) // Entering a new function scope
				function_scope += 1;				// Increment function scope
			return RUN_SWITCH_NODE;
		}
		else if (scope_status == SCOPE_AWAITING_CONCLUSION)
		{
			new_tree(RUN_POSITION_BRANCH_RIGHT); // Calculate rightwards value
			return RUN_SWITCH_NODE;
		}
		else if (scope_status == SCOPE_AWAIT_FINISHED)
		{
			switch (scope->type)
			{
			case SCOPE_LOGIC_AND:
			{
				scope_set_result(scope, (scope_get_result(scope_get_left(scope)) == SCOPE_BOOLEAN_TRUE && scope_get_result(scope_get_right(scope)) == SCOPE_BOOLEAN_TRUE));
				break;
			}
			case SCOPE_LOGIC_NOT:
			{
				if (scope_get_result_type(scope_get_left(scope)) == SCOPE_TYPE_STRING)
				{
					if (strlen(scope_get_result_string(variable_get_scope(scope_get_result(scope_get_left(scope)), function_scope))) == 0)
						scope_set_result(scope, SCOPE_BOOLEAN_TRUE);
					else
						scope_set_result(scope, SCOPE_BOOLEAN_FALSE);
				}
				else if (scope_get_result(scope_get_left(scope)) == SCOPE_BOOLEAN_TRUE)
					scope_set_result(scope, SCOPE_BOOLEAN_FALSE);
				else
					scope_set_result(scope, SCOPE_BOOLEAN_TRUE);
				break;
			}
			case SCOPE_LOGIC_EQUAL:
			{
				if (scope_get_result_type(scope_get_left(scope)) == SCOPE_TYPE_STRING && scope_get_result_type(scope_get_right(scope)) == SCOPE_TYPE_STRING)
				{
					if (strcmp(scope_get_result_string(variable_get_scope(scope_get_result(scope_get_left(scope)), function_scope)), scope_get_result_string(variable_get_scope(scope_get_result(scope_get_left(scope)), function_scope))) == 0)
						scope_set_result(scope, SCOPE_BOOLEAN_TRUE);
					else
						scope_set_result(scope, SCOPE_BOOLEAN_FALSE);
				}
				else if ((scope_get_result_type(scope_get_left(scope)) == SCOPE_TYPE_STRING && scope_get_result_type(scope_get_right(scope)) != SCOPE_TYPE_STRING) || (scope_get_result_type(scope_get_left(scope)) != SCOPE_TYPE_STRING && scope_get_result_type(scope_get_right(scope)) == SCOPE_TYPE_STRING))
					scope_set_result(scope, SCOPE_BOOLEAN_FALSE);
				else if (scope_get_result(scope_get_left(scope)) == scope_get_result(scope_get_right(scope)))
					scope_set_result(scope, SCOPE_BOOLEAN_TRUE);
				else
					scope_set_result(scope, SCOPE_BOOLEAN_FALSE);
				break;
			}
			case SCOPE_LOGIC_LESS:
			{
				if (scope_get_result_type(scope_get_left(scope)) == SCOPE_TYPE_STRING || scope_get_result_type(scope_get_right(scope)) == SCOPE_TYPE_STRING)
					return RUN_STRING_LOGIC_ERROR;
				else if (scope_get_type(scope_get_left(scope)) == SCOPE_TYPE_ARRAY || scope_get_type(scope_get_right(scope)) == SCOPE_TYPE_ARRAY)
					return RUN_ARRAY_LOGIC_ERROR;
				if (scope_get_result(scope_get_left(scope)) < scope_get_result(scope_get_right(scope)))
					scope_set_result(scope, SCOPE_BOOLEAN_TRUE);
				else
					scope_set_result(scope, SCOPE_BOOLEAN_FALSE);
				break;
			}
			case SCOPE_LOGIC_LESS_OR_EQUAL:
			{
				if (scope_get_result_type(scope_get_left(scope)) == SCOPE_TYPE_STRING || scope_get_result_type(scope_get_right(scope)) == SCOPE_TYPE_STRING)
					return RUN_STRING_LOGIC_ERROR;
				else if (scope_get_type(scope_get_left(scope)) == SCOPE_TYPE_ARRAY || scope_get_type(scope_get_right(scope)) == SCOPE_TYPE_ARRAY)
					return RUN_ARRAY_LOGIC_ERROR;
				if (scope_get_result(scope_get_left(scope)) <= scope_get_result(scope_get_right(scope)))
					scope_set_result(scope, SCOPE_BOOLEAN_TRUE);
				else
					scope_set_result(scope, SCOPE_BOOLEAN_FALSE);
				break;
			}
			case SCOPE_LOGIC_MORE:
			{
				if (scope_get_result_type(scope_get_left(scope)) == SCOPE_TYPE_STRING || scope_get_result_type(scope_get_right(scope)) == SCOPE_TYPE_STRING)
					return RUN_STRING_LOGIC_ERROR;
				else if (scope_get_type(scope_get_left(scope)) == SCOPE_TYPE_ARRAY || scope_get_type(scope_get_right(scope)) == SCOPE_TYPE_ARRAY)
					return RUN_ARRAY_LOGIC_ERROR;
				if (scope_get_result(scope_get_left(scope)) > scope_get_result(scope_get_right(scope)))
					scope_set_result(scope, SCOPE_BOOLEAN_TRUE);
				else
					scope_set_result(scope, SCOPE_BOOLEAN_FALSE);
				break;
			}
			case SCOPE_LOGIC_MORE_OR_EQUAL:
			{
				if (scope_get_result_type(scope_get_left(scope)) == SCOPE_TYPE_STRING || scope_get_result_type(scope_get_right(scope)) == SCOPE_TYPE_STRING)
					return RUN_STRING_LOGIC_ERROR;
				else if (scope_get_type(scope_get_left(scope)) == SCOPE_TYPE_ARRAY || scope_get_type(scope_get_right(scope)) == SCOPE_TYPE_ARRAY)
					return RUN_ARRAY_LOGIC_ERROR;
				if (scope_get_result(scope_get_left(scope)) >= scope_get_result(scope_get_right(scope)))
					scope_set_result(scope, SCOPE_BOOLEAN_TRUE);
				else
					scope_set_result(scope, SCOPE_BOOLEAN_FALSE);
				break;
			}
			case SCOPE_MATH_ADD:
			{
				if (scope_get_type(scope_get_left(scope)) == SCOPE_TYPE_STRING && scope_get_type(scope_get_right(scope)) != SCOPE_TYPE_STRING)
					scope_set_result_type(scope, SCOPE_TYPE_STRING);
				else if (scope_get_type(scope_get_left(scope)) != SCOPE_TYPE_STRING && scope_get_type(scope_get_right(scope)) == SCOPE_TYPE_STRING)
					scope_set_result_type(scope, SCOPE_TYPE_STRING);
				else if (scope_get_type(scope_get_left(scope)) == SCOPE_TYPE_STRING && scope_get_type(scope_get_right(scope)) == SCOPE_TYPE_STRING)
					scope_set_result_type(scope, SCOPE_TYPE_STRING);
				else if ((scope_get_type(scope_get_left(scope)) == SCOPE_TYPE_INT || scope_get_type(scope_get_left(scope)) == SCOPE_TYPE_BOOL) && scope_get_type(scope_get_right(scope)) == SCOPE_TYPE_DOUBLE)
					scope_set_result_type(scope, SCOPE_TYPE_DOUBLE);
				else if (scope_get_type(scope_get_left(scope)) == SCOPE_TYPE_DOUBLE && scope_get_type(scope_get_right(scope)) == SCOPE_TYPE_DOUBLE)
					scope_set_result_type(scope, SCOPE_TYPE_DOUBLE);
				else if (scope_get_type(scope_get_left(scope)) == SCOPE_TYPE_DOUBLE && (scope_get_type(scope_get_right(scope)) == SCOPE_TYPE_INT || scope_get_type(scope_get_right(scope)) == SCOPE_TYPE_BOOL))
					scope_set_result_type(scope, SCOPE_TYPE_DOUBLE);
				else if ((scope_get_type(scope_get_left(scope)) == SCOPE_TYPE_INT || scope_get_type(scope_get_left(scope)) == SCOPE_TYPE_BOOL) && (scope_get_type(scope_get_right(scope)) == SCOPE_TYPE_INT || scope_get_type(scope_get_right(scope)) == SCOPE_TYPE_BOOL))
					scope_set_result_type(scope, SCOPE_TYPE_INT);
				else if (scope_get_type(scope_get_left(scope)) == SCOPE_TYPE_ARRAY || scope_get_type(scope_get_right(scope)) == SCOPE_TYPE_ARRAY)
				{
					if (scope_get_type(scope_get_left(scope)) == SCOPE_TYPE_ARRAY && scope_get_type(scope_get_right(scope)) == SCOPE_TYPE_ARRAY)
					{
						//	scope_array_append()
					}
					else
						return RUN_ARRAY_LOGIC_ERROR;
				}
				else
					scope_set_result_type(scope, SCOPE_TYPE_STRING);
				if (scope_get_result_type(scope) == SCOPE_TYPE_INT || scope_get_result_type(scope) == SCOPE_TYPE_DOUBLE)
					scope_set_result(scope, (scope_get_result(scope_get_left(scope)) + scope_get_result(scope_get_right(scope))));
				else
				{
					// Result will be a string
					unsigned int scope_current_size = scope_size();
					char *temporary_strings[3] = {
						// Utilise the current string value, or allocated memory for new conversions
						((scope_get_type(scope_get_left(scope)) == SCOPE_TYPE_STRING) ? scope_get_result_string(variable_get_scope(scope_get_result(scope_get_left(scope)), function_scope)) : new_string()),
						((scope_get_type(scope_get_right(scope)) == SCOPE_TYPE_STRING) ? scope_get_result_string(variable_get_scope(scope_get_result(scope_get_right(scope)), function_scope)) : new_string()),
						NULL
					};
					if (temporary_strings[0] == NULL || temporary_strings[1] == NULL) // Failed to allocate memory for temporary strings
						return RUN_MEMORY_ALLOCATION_ERROR;
					switch (scope_get_type(scope_get_left(scope)))
					{
					case SCOPE_TYPE_INT:
						int_to_str((int)scope_get_result(scope_get_left(scope)), temporary_strings[0], 0);
						break;
					case SCOPE_TYPE_DOUBLE:
						float_to_string(scope_get_result(scope_get_left(scope)), temporary_strings[0]);
						break;
					case SCOPE_TYPE_BOOL:
						bool_to_string(scope_get_result(scope_get_left(scope)), temporary_strings[0]);
						break;
					default:
						return RUN_CONVERSION_FAILURE; // Unknown scope type
					}
					switch (scope_get_type(scope_get_right(scope)))
					{
					case SCOPE_TYPE_INT:
						int_to_str((int)scope_get_result(scope_get_right(scope)), temporary_strings[1], 0);
						break;
					case SCOPE_TYPE_DOUBLE:
						float_to_string(scope_get_result(scope_get_right(scope)), temporary_strings[1]);
						break;
					case SCOPE_TYPE_BOOL:
						bool_to_string(scope_get_result(scope_get_right(scope)), temporary_strings[1]);
						break;
					default:
						return RUN_CONVERSION_FAILURE; // Unknown scope type
					}
					if (strlen(temporary_strings[0]) + strlen(temporary_strings[1]) > STRING_MEMORY_MAX_LENGTH)
						return RUN_MEMORY_ALLOCATION_ERROR;
					unsigned short string_length = (strlen(temporary_strings[0]) + strlen(temporary_strings[1]));
					temporary_strings[2] = new_string_size((string_length + 1));
					if (temporary_strings[2] == NULL)
						return RUN_MEMORY_ALLOCATION_ERROR;
					temporary_strings[2][string_length] = '\0';
					strcat(temporary_strings[2], temporary_strings[0]); // Concatenate string values together
					free(temporary_strings[0]);
					strcat(temporary_strings[2], temporary_strings[1]); // Concatenate string values together
					free(temporary_strings[1]);
					scope_set_result(scope, new_variable(scope_current_size, function_scope, VARIABLE_TEMPORARY)); // Assign a new variable as a response value
					scope_set_result_string(variable_get_scope(scope_get_result(scope), function_scope), temporary_strings[2]); // Assign a string value to response value
					free(temporary_strings[2]);
				}
				break;
			}
			case SCOPE_MATH_SUBTRACT:
			{
				if (scope_get_result_type(scope_get_left(scope)) == SCOPE_TYPE_STRING || scope_get_result_type(scope_get_right(scope)) == SCOPE_TYPE_STRING)
					return RUN_STRING_LOGIC_ERROR;
				else if (scope_get_type(scope_get_left(scope)) == SCOPE_TYPE_ARRAY || scope_get_type(scope_get_right(scope)) == SCOPE_TYPE_ARRAY)
					return RUN_ARRAY_LOGIC_ERROR;
				scope_set_result(scope, (scope_get_result(scope_get_left(scope)) - scope_get_result(scope_get_right(scope))));
				break;
			}
			case SCOPE_MATH_MULTIPLY:
			{
				if (scope_get_result_type(scope_get_left(scope)) == SCOPE_TYPE_STRING || scope_get_result_type(scope_get_right(scope)) == SCOPE_TYPE_STRING)
					return RUN_STRING_LOGIC_ERROR;
				scope_set_result(scope, (scope_get_result(scope_get_left(scope)) * scope_get_result(scope_get_right(scope))));
				break;
			}
			case SCOPE_MATH_DIVIDE:
			{
				if (scope_get_result_type(scope_get_left(scope)) == SCOPE_TYPE_STRING || scope_get_result_type(scope_get_right(scope)) == SCOPE_TYPE_STRING)
					return RUN_STRING_LOGIC_ERROR;
				else if (scope_get_type(scope_get_left(scope)) == SCOPE_TYPE_ARRAY || scope_get_type(scope_get_right(scope)) == SCOPE_TYPE_ARRAY)
					return RUN_ARRAY_LOGIC_ERROR;
				if (scope_get_result(scope_get_right(scope)) == 0)
					return RUN_FAILURE;						 // Zero devision error
				else if (scope_get_result(scope_get_left(scope)) == 0) // Will always result in a value of zero
					scope_set_result(scope, 0);				 // Set result to zero in order to avoid unnecessary mathmatical calculation
				else
					scope_set_result(scope, (scope_get_result(scope_get_left(scope)) / scope_get_result(scope_get_right(scope))));
				break;
			}
			case SCOPE_MATH_MODULO:
			{
				if (scope_get_result_type(scope_get_left(scope)) == SCOPE_TYPE_STRING || scope_get_result_type(scope_get_right(scope)) == SCOPE_TYPE_STRING)
					return RUN_STRING_LOGIC_ERROR;
				else if (scope_get_type(scope_get_left(scope)) == SCOPE_TYPE_ARRAY || scope_get_type(scope_get_right(scope)) == SCOPE_TYPE_ARRAY)
					return RUN_ARRAY_LOGIC_ERROR;
				if (scope_get_result(scope_get_right(scope)) == 0)
					return RUN_FAILURE;						 // Modulo zero error
				else if (scope_get_result(scope_get_left(scope)) == 0) // Will always result in a value of zero
					scope_set_result(scope, 0);				 // Set result to zero in order to avoid mathmatical calculation
				else
					scope_set_result(scope, fmod(scope_get_result(scope_get_left(scope)), scope_get_result(scope_get_right(scope))));
				break;
			}
			case SCOPE_VARIABLE:
			{
				break;
			}
			case SCOPE_VARIABLE_UPDATE:
			{
				struct scope_t *variable_scope = variable_get_scope(scope_get_result(scope_get_right(scope)), function_scope);
				if (scope_get_result_type(scope_get_left(scope)) == SCOPE_TYPE_STRING)
				{
					scope_set_result_type(variable_scope, SCOPE_TYPE_STRING);
					scope_set_result_string(variable_scope, scope_get_result_string(variable_get_scope(scope_get_result(scope_get_left(scope)), function_scope)));
				}
				else
				{
					scope_set_result_type(variable_scope, scope_get_result_type(scope_get_left(scope)));
					scope_set_result(variable_scope, scope_get_result(scope_get_left(scope)));
				}
				break;
			}
			case SCOPE_TYPE_PARAMETER_START:
			{
				function_scope_parameter = RUN_PARAMETER_STACK_OPEN; // Everything subsequent is a function parameter
				function_scope += 1;								 // Increase function scope
				break;
			}
			case SCOPE_TYPE_PARAMETER_END:
			{
				function_scope_parameter = RUN_PARAMETER_STACK_CLOSED; // Everything prior was a function parameter
				break;
			}
			case SCOPE_TYPE_FUNCTION_CALL:
			{
				break;
			}
			case SCOPE_TYPE_FUNCTION:
			{
				function_scope -= 1;
				break;
			}
			case SCOPE_TYPE_CONTAINER:
			{
				cleanup(execution_scope);
				break;
			}
			default:
				break;
			}
			if (function_scope_parameter == RUN_PARAMETER_STACK_OPEN)
				push_parameter_stack(function_scope, scope); // Add current parameter to the parameter stack
			decrease_tree(1);
		}
		else
			return RUN_FAILURE;
	}
	else
	{
		decrease_tree(1); // Does not have traversable children
		return RUN_SWITCH_NODE;
	}
	return RUN_STEP_CONTINUE;
}

void update_scope_branch(struct scope_t *current_scope, struct scope_t *scope)
{
	if (position_tree == NULL)
		current_scope = scope_get_right(scope); // Set to initial child of main function
	current_scope = scope;
	struct position *current_position_branch = position_tree;
	for (;;)
	{
		switch (current_position_branch->branch)
		{
		case RUN_POSITION_BRANCH_LEFT:
		{
			current_scope = scope_get_left(current_scope);
			break;
		}
		case RUN_POSITION_BRANCH_RIGHT:
		{
			current_scope = scope_get_right(current_scope);
			break;
		}
		case RUN_POSITION_BRANCH_NULL:
			return;
		}
		current_position_branch = current_position_branch->next;
	}
}

int run(struct scope_t *scope)
{
	stack_reset(scope);
	if (scope == NULL) // No initial scope was provided
		return RUN_FAILURE;
	int operation = RUN_STEP_CONTINUE; // Set mode to continue
	int operation_state = RUN_SWITCH_NODE;
	struct scope_t *current_scope = scope;
	while (operation == RUN_STEP_CONTINUE) // If can continue
	{
		if (operation_state == RUN_SWITCH_NODE)
			update_scope_branch(current_scope, scope); // Update to the correct scope
		operation = step(current_scope);			   // Get continuation state of current execution step
		operation_state = operation;
		switch (operation)
		{
		case RUN_SWITCH_NODE: // Scope update - can still continue
			operation = RUN_STEP_CONTINUE;
			break;
		case RUN_MEMORY_ALLOCATION_ERROR: // Failed to allocate memory
			fatal_error(MEMORY_ALLOCATION_ERROR);
			break;
		case RUN_STRING_LOGIC_ERROR: case RUN_ARRAY_LOGIC_ERROR: // Logical error with operation
			fatal_error(LOGIC_ERROR);
			break;
		case RUN_CONVERSION_FAILURE:
			fatal_error(CONVERSION_ERROR);
			break;
		case RUN_FAILURE_CLEANING_UP: // Failure to cleanup scope
			fatal_error(CLEANUP_ERROR);
			break;
		case RUN_FAILURE:
			fatal_error(EXECUTION_ERROR); // Unknown execution error occurred
			break;
		default:
			break;
		}
	}
	return operation; // Reason for escape or termination
}

unsigned int scope_size()
{
	unsigned int scope_size = 0;
	if (position_tree == NULL)
		return scope_size;
	struct position *position_tree_current_node = position_tree;
	do
	{
		scope_size += 1;											   // Increment scope size by 1
		position_tree_current_node = position_tree_current_node->next; // Get child node
	} while (position_tree_current_node != NULL);					   // If child node does exist
	return scope_size;
}

#endif