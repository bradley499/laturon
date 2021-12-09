#ifndef run_c
#define run_c

#include <stdlib.h>
#include <math.h>

#include "scope.h"
#include "run.h"
#include "misc.h"
#include "parameter.h"
#include "variable.h"
#include "stack.h"

unsigned int execution_scope = 0;
unsigned int function_scope = 0;
unsigned char function_scope_parameter = RUN_PARAMETER_STACK_CLOSED;

scope_t *scope = NULL;
scope_t *root_scope = NULL;

void set_initial_scope(scope_t *initial_scope)
{
	scope = initial_scope;
	root_scope = initial_scope;
}

int step()
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
			scope = scope->left;
			if (scope->type == SCOPE_TYPE_FUNCTION) // Entering a new function scope
				function_scope += 1;				// Increment function scope
			return RUN_SWITCH_NODE;
		}
		else if (scope_status == SCOPE_AWAITING_CONCLUSION)
		{
			scope = scope->right;
			return RUN_SWITCH_NODE;
		}
		else if (scope_status == SCOPE_AWAIT_FINISHED)
		{
			switch (scope->type)
			{
			case SCOPE_LOGIC_AND:
			{
				if (scope_get_result_type(scope_get_left(scope)) != SCOPE_TYPE_BOOL && scope_get_result_type(scope_get_right(scope)) != SCOPE_TYPE_BOOL)
					return RUN_ARITHMETIC_LOGIC_ERROR;
				scope_set_result(scope, (scope_get_result(scope_get_left(scope)) == SCOPE_BOOLEAN_TRUE && scope_get_result(scope_get_right(scope)) == SCOPE_BOOLEAN_TRUE));
				break;
			}
			case SCOPE_LOGIC_NOT:
			{
				if (scope_get_result_type(scope_get_left(scope)) == SCOPE_TYPE_ARRAY)
					return RUN_ARITHMETIC_LOGIC_ERROR;
				else
				{
					scope_set_result_type(SCOPE_TYPE_BOOL);
					if (scope_get_result(scope_get_left(scope)) == SCOPE_BOOLEAN_FALSE)
						scope_set_result(scope, SCOPE_BOOLEAN_TRUE);
					else
						scope_set_result(scope, SCOPE_BOOLEAN_TRUE);
				}
				if (scope_get_result_type(scope_get_left(scope)) == SCOPE_TYPE_BOOL)
					scope_set_result(scope, SCOPE_BOOLEAN_FALSE);
				else
					scope_set_result(scope, SCOPE_BOOLEAN_TRUE);
				break;
			}
			case SCOPE_LOGIC_EQUAL:
			{
				if (scope_get_result_type(scope_get_left(scope)) == scope_get_result_type(scope_get_right(scope)))
					scope_set_result(scope, SCOPE_BOOLEAN_TRUE);
				else
					scope_set_result(scope, SCOPE_BOOLEAN_FALSE);
				break;
			}
			case SCOPE_LOGIC_LESS:
			{
				if (scope_get_result_type(scope_get_left(scope)) == SCOPE_TYPE_ARRAY || scope_get_result_type(scope_get_right(scope)) == SCOPE_TYPE_ARRAY)
					return RUN_ARRAY_LOGIC_ERROR;
				if (scope_get_result(scope_get_left(scope)) < scope_get_result(scope_get_right(scope)))
					scope_set_result(scope, SCOPE_BOOLEAN_TRUE);
				else
					scope_set_result(scope, SCOPE_BOOLEAN_FALSE);
				break;
			}
			case SCOPE_LOGIC_LESS_OR_EQUAL:
			{
				if (scope_get_result_type(scope_get_left(scope)) == SCOPE_TYPE_ARRAY || scope_get_result_type(scope_get_right(scope)) == SCOPE_TYPE_ARRAY)
					return RUN_ARRAY_LOGIC_ERROR;
				if (scope_get_result(scope_get_left(scope)) <= scope_get_result(scope_get_right(scope)))
					scope_set_result(scope, SCOPE_BOOLEAN_TRUE);
				else
					scope_set_result(scope, SCOPE_BOOLEAN_FALSE);
				break;
			}
			case SCOPE_LOGIC_MORE:
			{
				if (scope_get_result_type(scope_get_left(scope)) == SCOPE_TYPE_ARRAY || scope_get_result_type(scope_get_right(scope)) == SCOPE_TYPE_ARRAY)
					return RUN_ARRAY_LOGIC_ERROR;
				scope_set_result_type(SCOPE_TYPE_BOOL);
				if (scope_get_result(scope_get_left(scope)) > scope_get_result(scope_get_right(scope)))
					scope_set_result(scope, SCOPE_BOOLEAN_TRUE);
				else
					scope_set_result(scope, SCOPE_BOOLEAN_FALSE);
				break;
			}
			case SCOPE_LOGIC_MORE_OR_EQUAL:
			{
				if (scope_get_result_type(scope_get_left(scope)) == SCOPE_TYPE_ARRAY || scope_get_result_type(scope_get_right(scope)) == SCOPE_TYPE_ARRAY)
					return RUN_ARRAY_LOGIC_ERROR;
				scope_set_result_type(SCOPE_TYPE_BOOL);
				if (scope_get_result(scope_get_left(scope)) >= scope_get_result(scope_get_right(scope)))
					scope_set_result(scope, SCOPE_BOOLEAN_TRUE);
				else
					scope_set_result(scope, SCOPE_BOOLEAN_FALSE);
				break;
			}
			case SCOPE_MATH_ADD:
			{
				if ((scope_get_result_type(scope_get_left(scope)) == SCOPE_TYPE_INT || scope_get_result_type(scope_get_left(scope)) == SCOPE_TYPE_BOOL) && scope_get_result_type(scope_get_right(scope)) == SCOPE_TYPE_DOUBLE)
					scope_set_result_type(SCOPE_TYPE_DOUBLE);
				else if (scope_get_result_type(scope_get_left(scope)) == SCOPE_TYPE_DOUBLE && scope_get_result_type(scope_get_right(scope)) == SCOPE_TYPE_DOUBLE)
					scope_set_result_type(SCOPE_TYPE_DOUBLE);
				else if (scope_get_result_type(scope_get_left(scope)) == SCOPE_TYPE_DOUBLE && (scope_get_result_type(scope_get_right(scope)) == SCOPE_TYPE_INT || scope_get_result_type(scope_get_right(scope)) == SCOPE_TYPE_BOOL))
					scope_set_result_type(SCOPE_TYPE_DOUBLE);
				else if ((scope_get_result_type(scope_get_left(scope)) == SCOPE_TYPE_INT || scope_get_result_type(scope_get_left(scope)) == SCOPE_TYPE_BOOL) && (scope_get_result_type(scope_get_right(scope)) == SCOPE_TYPE_INT || scope_get_result_type(scope_get_right(scope)) == SCOPE_TYPE_BOOL))
					scope_set_result_type(SCOPE_TYPE_INT);
				else if (scope_get_result_type(scope_get_left(scope)) == SCOPE_TYPE_ARRAY || scope_get_result_type(scope_get_right(scope)) == SCOPE_TYPE_ARRAY)
				{
					if (scope_get_result_type(scope_get_left(scope)) == SCOPE_TYPE_ARRAY && scope_get_result_type(scope_get_right(scope)) == SCOPE_TYPE_ARRAY)
					{
						//	scope_array_append()
						break;
					}
					else
						return RUN_ARRAY_LOGIC_ERROR;
				}
				scope_set_result(scope, (scope_get_result(scope_get_left(scope)) + scope_get_result(scope_get_right(scope))));
				break;
			}
			case SCOPE_MATH_SUBTRACT:
			{
				if (scope_get_result_type(scope_get_left(scope)) == SCOPE_TYPE_ARRAY || scope_get_result_type(scope_get_right(scope)) == SCOPE_TYPE_ARRAY)
					return RUN_ARRAY_LOGIC_ERROR;
				scope_set_result(scope, (scope_get_result(scope_get_left(scope)) - scope_get_result(scope_get_right(scope))));
				break;
			}
			case SCOPE_MATH_MULTIPLY:
			{
				if (scope_get_result_type(scope_get_left(scope)) == SCOPE_TYPE_ARRAY || scope_get_result_type(scope_get_right(scope)) == SCOPE_TYPE_ARRAY)
					return RUN_STRING_LOGIC_ERROR;
				scope_set_result(scope, (scope_get_result(scope_get_left(scope)) * scope_get_result(scope_get_right(scope))));
				break;
			}
			case SCOPE_MATH_DIVIDE:
			{
				if (scope_get_result_type(scope_get_left(scope)) == SCOPE_TYPE_ARRAY || scope_get_result_type(scope_get_right(scope)) == SCOPE_TYPE_ARRAY)
					return RUN_ARRAY_LOGIC_ERROR;
				if (scope_get_result(scope_get_right(scope)) == 0)
					return RUN_FAILURE;								   // Zero devision error
				else if (scope_get_result(scope_get_left(scope)) == 0) // Will always result in a value of zero
					scope_set_result(scope, 0);						   // Set result to zero in order to avoid unnecessary mathmatical calculation
				else
					scope_set_result(scope, (scope_get_result(scope_get_left(scope)) / scope_get_result(scope_get_right(scope))));
				break;
			}
			case SCOPE_MATH_MODULO:
			{
				if (scope_get_result_type(scope_get_left(scope)) == SCOPE_TYPE_ARRAY || scope_get_result_type(scope_get_right(scope)) == SCOPE_TYPE_ARRAY)
					return RUN_ARRAY_LOGIC_ERROR;
				if (scope_get_result(scope_get_right(scope)) == 0)
					return RUN_FAILURE;								   // Modulo zero error
				else if (scope_get_result(scope_get_left(scope)) == 0) // Will always result in a value of zero
					scope_set_result(scope, 0);						   // Set result to zero in order to avoid mathmatical calculation
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
				scope_t *variable_scope = variable_get_scope(scope_get_result(scope_get_right(scope)), function_scope);
				scope_set_result_type(scope_get_result_type(scope_get_left(scope)));
				scope_set_result_type(stack_result_type(scope_get_left(scope)));
				scope_set_result(variable_scope, scope_get_result(scope_get_left(scope)));
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
			{
				if (parameter_stack_parent_push(function_scope, scope) == PARAMETER_PUSH_FAILURE) // Add current parameter to the parameter stack
					return RUN_MEMORY_EXHAUSTION_ERROR;									   // No more space exists on parameter stack
			}
			scope = stack_parent_pop(scope); // Scope now points to scopes stack parent
		}
		else
			return RUN_FAILURE;
	}
	else
	{							  // Does not have traversable children
		scope = stack_parent_pop(scope); // Scope now points to scopes stack parent
		return RUN_SWITCH_NODE;
	}
	return RUN_STEP_CONTINUE;
}

int run()
{
	if (scope == NULL) // No initial scope was provided
		return RUN_FAILURE;
	int operation = RUN_STEP_CONTINUE;	   // Set mode to continue
	while (operation == RUN_STEP_CONTINUE) // If can continue
	{
		operation = step(); // Get continuation state of current execution step
		switch (operation)
		{
		case RUN_SWITCH_NODE: // Scope update - can still continue
			operation = RUN_STEP_CONTINUE;
			break;
		case RUN_MEMORY_EXHAUSTION_ERROR:
		case RUN_MEMORY_ALLOCATION_ERROR: // Failed to allocate memory
			fatal_error(MEMORY_ALLOCATION_ERROR);
			break;
		case RUN_STRING_LOGIC_ERROR:
		case RUN_ARRAY_LOGIC_ERROR: // Logical error with operation
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
	// To implement counter!
	return 0;
}

#endif