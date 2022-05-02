#ifndef run_c
#define run_c

#include <stdlib.h>
#include <math.h>
#include <tgmath.h>
#include <float.h>
#include <limits.h>
#include <string.h>
#include "run.h"
#include "variable.h"
#include "tokenizer.h"
#include "misc.h"
#include "interact.h"

#define initial_value(X) (X - 1)
#define secondary_value(X) X

#define FUNCTION_CALL_HOST_OPERATION (CHAR_MAX - FUNCTION_CALL_HOST)

stack_value_t *execution_stack[EXECUTION_STACK_SIZE] = {NULL};
unsigned long long int execution_stack_position = 0;
unsigned long long function_scope = 0;
unsigned long long execution_scope = 0;

struct scope_function_reference
{
	parsed_function_scope_t *function;
	token_t *token;
	struct scope_function_reference *previous;
} scope_function_reference;

struct scope_function_reference *scope_function_references = NULL;

unsigned long long execution_stack_while_references[EXECUTION_STACK_SIZE];
unsigned long long int execution_stack_while_references_position = 0;

void run_stack_free_value();

void run_stack_reset()
{
	for (; execution_stack_position != 0;)
		run_stack_free_value();
}

stack_value_t *run_stack_value_new(variable_type_t type)
{
	stack_value_t *stack_value_new = xmalloc(sizeof(stack_value_t));
	stack_value_new->value = xmalloc(sizeof(variable_value_t));
	stack_value_new->value->type = type;
	return stack_value_new;
}

void run_stack_add_value(stack_value_t *value)
{
	if (execution_stack_position == (EXECUTION_STACK_SIZE - 1))
		fatal_error(STACK_MEMORY_LIMIT_REACHED);
	execution_stack[execution_stack_position++] = value;
}

void run_stack_free_value()
{
	if (execution_stack_position == 0)
		fatal_error(STACK_REFERENCE_ERROR);
	execution_stack_position -= 1;
	if (scope_function_references != NULL && execution_stack[execution_stack_position]->operation_type == FUNCTION_CALL_HOST_OPERATION)
	{
		struct scope_function_reference *function_reference = scope_function_references;
		scope_function_references = scope_function_references->previous;
		free(function_reference);
		execution_stack[execution_stack_position]->operation_type = NOT_DEFINED;
	}
	else if (execution_stack[execution_stack_position]->operation_type == VARIABLE_STRING)
		free(execution_stack[execution_stack_position]->value->contents.string);
	free(execution_stack[execution_stack_position]->value);
	free(execution_stack[execution_stack_position]);
	execution_stack[execution_stack_position] = NULL;
}

int run_stack_size_assigned_ok(unsigned char size)
{
	if (size > execution_stack_position)
		return 0;
	unsigned long long relative_stack_position = (execution_stack_position - 1);
	if (execution_stack[relative_stack_position]->operation_type == NOT_DEFINED)
		if (!variable_value_assigned(variable_get_host_id(execution_stack[relative_stack_position]->value->variable_id_reference[VARIABLE_ID_TYPE_POSITIONAL]), 0))
			return 0;
	if (size == 2)
	{
		relative_stack_position--;
		if (execution_stack[relative_stack_position]->operation_type == NOT_DEFINED)
			if (!variable_value_assigned(variable_get_host_id(execution_stack[relative_stack_position]->value->variable_id_reference[VARIABLE_ID_TYPE_POSITIONAL]), 0))
				return 0;
	}
	return 1;
}

int run_stack_state_is_true(unsigned int relative_position)
{
	if (execution_stack_position < (relative_position + 1))
		fatal_error(STACK_REFERENCE_ERROR);
	unsigned long long relative_stack_position = (execution_stack_position - relative_position - 1);
	switch (execution_stack[relative_stack_position]->value->type)
	{
	case VARIABLE_INT:
	case VARIABLE_BOOLEAN:
		return (execution_stack[relative_stack_position]->value->contents.numeric != BOOLEAN_FALSE);
		break;
	case VARIABLE_DOUBLE:
		return (execution_stack[relative_stack_position]->value->contents.floating != BOOLEAN_FALSE);
		break;
	default:
		break;
	}
	return 0;
}

struct run_step_state
{
	unsigned char state;
	signed long long value;
	token_t *token;
} run_step_state;

struct run_step_state run_stack_step(token_t *token, parsed_function_scope_t *function)
{
	struct run_step_state response = {
		.state = 0,
		.value = 0,
		.token = NULL,
	};
	enum overflow_underflow_value_t
	{
		OK,
		OVERFLOW,
		UNDERFLOW,
	};
	enum overflow_underflow_value_t overflow_underflow = OK;
	unsigned long long relative_stack_position = (execution_stack_position - 1);
	switch (token->type)
	{
	case FUNCTION_CALL:
	{
		for (unsigned long long execution_stack_position_negation = relative_stack_position; execution_stack_position != ULLONG_MAX; execution_stack_position_negation--)
		{
			if (execution_stack[execution_stack_position_negation]->operation_type == FUNCTION_CALL_HOST_OPERATION)
			{
				struct scope_function_reference *function_reference = xmalloc(sizeof(struct scope_function_reference));
				function_reference->function = function;
				function_reference->token = token;
				function_reference->previous = scope_function_references;
				scope_function_references = function_reference;
				break;
			}
		}
		response.state = FUNCTION_CALL;
		response.value = token->contents.numeric;
		break;
	}
	case FUNCTION_CALL_HOST:
	{
		stack_value_t *stack_result = run_stack_value_new(VARIABLE_IS_UNASSIGNED);
		stack_result->value->contents.numeric = 0;
		stack_result->operation_type = FUNCTION_CALL_HOST_OPERATION;
		run_stack_add_value(stack_result);
		break;
	}
	case VARIABLE:
	{
		if (token->contents.numeric < VARIABLE_NUMERIC_REFERENCE_START)
		{
			stack_value_t *stack_result = run_stack_value_new(VARIABLE_BOOLEAN);
			stack_result->value->contents.numeric = token->contents.numeric;
			run_stack_add_value(stack_result);
			break;
		}
		variable_value_t variable_value = variable_get_value(token->contents.numeric, function_scope, execution_scope);
		stack_value_t *stack_result = run_stack_value_new(variable_value.type);
		stack_result->value->variable_id_reference[VARIABLE_ID_TYPE_REFERENCE] = variable_value.variable_id_reference[VARIABLE_ID_TYPE_REFERENCE];
		stack_result->value->variable_id_reference[VARIABLE_ID_TYPE_POSITIONAL] = variable_value.variable_id_reference[VARIABLE_ID_TYPE_POSITIONAL];
		stack_result->value->type = variable_value.type;
		switch (variable_value.type)
		{
		case VARIABLE_INT:
		case VARIABLE_BOOLEAN:
			stack_result->value->contents.numeric = variable_value.contents.numeric;
			break;
		case VARIABLE_DOUBLE:
			stack_result->value->contents.floating = variable_value.contents.floating;
			break;
		case VARIABLE_STRING:
		{
			stack_result->value->contents.string = xmalloc(strlen(variable_value.contents.string) + 1);
			if (!copy_string(stack_result->value->contents.string, variable_value.contents.string))
				fatal_error(MEMORY_ALLOCATION_ERROR);
			break;
		}
		case VARIABLE_ARRAY:
			break;
		case VARIABLE_NULL:
			stack_result->value->contents.numeric = BOOLEAN_FALSE;
			break;
		default:
			break;
		}
		run_stack_add_value(stack_result);
		break;
	}
	case BRACKETS_OPEN:
		break;
	case BRACKETS_CLOSE:
		break;
	case SCOPE_OPEN:
	{
		if (execution_scope == (VARIABLE_UNASSIGNED - 1))
			fatal_error(STACK_MEMORY_LIMIT_REACHED);
		execution_scope++;
		break;
	}
	case SCOPE_CLOSE:
	{
		variable_cleanup(execution_scope);
		execution_scope--;
		if (execution_stack_while_references_position > 0 && execution_stack_while_references[(execution_stack_while_references_position - 1)] == execution_scope)
		{
			execution_stack_while_references_position--;
			for (;; relative_stack_position--)
			{
				if (execution_stack[relative_stack_position]->operation_type == WHILE)
				{
					response.state = WHILE_START;
					response.token = execution_stack[relative_stack_position]->token;
					run_stack_free_value();
					break;
				}
				run_stack_free_value();
			}
		}
		break;
	}
	case OPERATOR:
	{
		if (relative_stack_position < 1)
			fatal_error(STACK_REFERENCE_ERROR);
		int value_types[2] = {0, execution_stack[secondary_value(relative_stack_position)]->value->type};
		signed long long c = token->contents.numeric;
		if (!run_stack_size_assigned_ok(1 + (c != (signed long long)'!')))
			fatal_error(VALUE_NOT_SET);
		if (c != (signed long long)'!')
		{
			if (relative_stack_position < 2)
				fatal_error(STACK_REFERENCE_ERROR);
			value_types[0] = execution_stack[initial_value(relative_stack_position)]->value->type;
		}
		if (c == (signed long long)'+')
		{
			if (value_types[0] == VARIABLE_INT || value_types[0] == VARIABLE_BOOLEAN)
			{
				if (value_types[1] == VARIABLE_INT || value_types[1] == VARIABLE_BOOLEAN)
				{
					if (execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric > 0 && execution_stack[initial_value(relative_stack_position)]->value->contents.numeric > (LLONG_MAX - execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric))
						overflow_underflow = OVERFLOW;
					else if (execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric < 0 && execution_stack[initial_value(relative_stack_position)]->value->contents.numeric < (LLONG_MIN - execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric))
						overflow_underflow = UNDERFLOW;
					signed long long int result = (execution_stack[initial_value(relative_stack_position)]->value->contents.numeric + execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric);
					run_stack_free_value();
					run_stack_free_value();
					stack_value_t *stack_result = run_stack_value_new(VARIABLE_INT);
					stack_result->operation_type = VARIABLE_INT;
					stack_result->value->contents.numeric = result;
					run_stack_add_value(stack_result);
				}
				else if (value_types[1] == VARIABLE_DOUBLE)
				{
					if ((long double)execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric > 0 && execution_stack[initial_value(relative_stack_position)]->value->contents.floating > (LDBL_MAX - (long double)execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric))
						overflow_underflow = OVERFLOW;
					else if ((long double)execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric < 0 && execution_stack[initial_value(relative_stack_position)]->value->contents.floating < (-LDBL_MAX - (long double)execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric))
						overflow_underflow = UNDERFLOW;
					long double result = (execution_stack[initial_value(relative_stack_position)]->value->contents.numeric + execution_stack[secondary_value(relative_stack_position)]->value->contents.floating);
					run_stack_free_value();
					run_stack_free_value();
					stack_value_t *stack_result = run_stack_value_new(VARIABLE_DOUBLE);
					stack_result->operation_type = VARIABLE_DOUBLE;
					stack_result->value->contents.floating = result;
					run_stack_add_value(stack_result);
				}
				else
					fatal_error_lined(LOGIC_ERROR, token->line);
			}
			else if (value_types[0] == VARIABLE_DOUBLE)
			{
				if (value_types[1] == VARIABLE_INT || value_types[1] == VARIABLE_BOOLEAN)
				{
					if (execution_stack[secondary_value(relative_stack_position)]->value->contents.floating > 0 && (long double)execution_stack[initial_value(relative_stack_position)]->value->contents.numeric > (LDBL_MAX - execution_stack[secondary_value(relative_stack_position)]->value->contents.floating))
						overflow_underflow = OVERFLOW;
					else if (execution_stack[secondary_value(relative_stack_position)]->value->contents.floating < 0 && (long double)execution_stack[initial_value(relative_stack_position)]->value->contents.numeric < (-LDBL_MAX - execution_stack[secondary_value(relative_stack_position)]->value->contents.floating))
						overflow_underflow = UNDERFLOW;
					long double result = (execution_stack[initial_value(relative_stack_position)]->value->contents.floating + (long double)execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric);
					run_stack_free_value();
					run_stack_free_value();
					stack_value_t *stack_result = run_stack_value_new(VARIABLE_DOUBLE);
					stack_result->operation_type = VARIABLE_DOUBLE;
					stack_result->value->contents.floating = result;
					run_stack_add_value(stack_result);
				}
				else if (value_types[1] == VARIABLE_DOUBLE)
				{
					if (execution_stack[secondary_value(relative_stack_position)]->value->contents.floating > 0 && execution_stack[initial_value(relative_stack_position)]->value->contents.floating > (LDBL_MAX - execution_stack[secondary_value(relative_stack_position)]->value->contents.floating))
						overflow_underflow = OVERFLOW;
					else if (execution_stack[secondary_value(relative_stack_position)]->value->contents.floating < 0 && execution_stack[initial_value(relative_stack_position)]->value->contents.floating < (-LDBL_MAX - execution_stack[secondary_value(relative_stack_position)]->value->contents.floating))
						overflow_underflow = UNDERFLOW;
					long double result = (execution_stack[initial_value(relative_stack_position)]->value->contents.floating + execution_stack[secondary_value(relative_stack_position)]->value->contents.floating);
					run_stack_free_value();
					run_stack_free_value();
					stack_value_t *stack_result = run_stack_value_new(VARIABLE_DOUBLE);
					stack_result->operation_type = VARIABLE_DOUBLE;
					stack_result->value->contents.floating = result;
					run_stack_add_value(stack_result);
				}
				else
					fatal_error_lined(LOGIC_ERROR, token->line);
			}
			else if (value_types[0] == VARIABLE_STRING)
			{
				if (value_types[1] == VARIABLE_STRING)
				{
					unsigned long result_size = (strlen(execution_stack[initial_value(relative_stack_position)]->value->contents.string)) + strlen(execution_stack[secondary_value(relative_stack_position)]->value->contents.string);
					char *result_string = xmalloc(result_size + 1);
					if (!copy_string(result_string, execution_stack[initial_value(relative_stack_position)]->value->contents.string))
						fatal_error(MEMORY_ALLOCATION_ERROR);
					result_string = strcat(result_string, execution_stack[secondary_value(relative_stack_position)]->value->contents.string);
					if (strlen(result_string) != result_size)
						fatal_error(MEMORY_ALLOCATION_ERROR);
					run_stack_free_value();
					run_stack_free_value();
					stack_value_t *stack_result = run_stack_value_new(VARIABLE_STRING);
					stack_result->operation_type = VARIABLE_STRING;
					stack_result->value->contents.string = result_string;
					run_stack_add_value(stack_result);
				}
				else
					fatal_error_lined(CONVERSION_ERROR, token->line);
			}
			else
				fatal_error_lined(LOGIC_ERROR, token->line);
		}
		else if (c == (signed long long)'-')
		{
			if (value_types[0] == VARIABLE_INT || value_types[0] == VARIABLE_BOOLEAN)
			{
				if (value_types[1] == VARIABLE_INT || value_types[1] == VARIABLE_BOOLEAN)
				{
					if (execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric < 0 && execution_stack[initial_value(relative_stack_position)]->value->contents.numeric > (LLONG_MAX + execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric))
						overflow_underflow = OVERFLOW;
					else if (execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric > 0 && execution_stack[initial_value(relative_stack_position)]->value->contents.numeric < (LLONG_MIN + execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric))
						overflow_underflow = UNDERFLOW;
					signed long long int result = (execution_stack[initial_value(relative_stack_position)]->value->contents.numeric - execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric);
					run_stack_free_value();
					run_stack_free_value();
					stack_value_t *stack_result = run_stack_value_new(VARIABLE_INT);
					stack_result->operation_type = VARIABLE_INT;
					stack_result->value->contents.numeric = result;
					run_stack_add_value(stack_result);
				}
				else if (value_types[1] == VARIABLE_DOUBLE)
				{
					if ((long double)execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric < 0 && execution_stack[initial_value(relative_stack_position)]->value->contents.floating > (LDBL_MAX + (long double)execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric))
						overflow_underflow = OVERFLOW;
					else if ((long double)execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric > 0 && execution_stack[initial_value(relative_stack_position)]->value->contents.floating < (-LDBL_MAX + (long double)execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric))
						overflow_underflow = UNDERFLOW;
					long double result = (execution_stack[initial_value(relative_stack_position)]->value->contents.numeric - execution_stack[secondary_value(relative_stack_position)]->value->contents.floating);
					run_stack_free_value();
					run_stack_free_value();
					stack_value_t *stack_result = run_stack_value_new(VARIABLE_DOUBLE);
					stack_result->operation_type = VARIABLE_DOUBLE;
					stack_result->value->contents.floating = result;
					run_stack_add_value(stack_result);
				}
				else
					fatal_error_lined(LOGIC_ERROR, token->line);
			}
			else if (value_types[0] == VARIABLE_DOUBLE)
			{
				if (value_types[1] == VARIABLE_INT || value_types[1] == VARIABLE_BOOLEAN)
				{
					if (execution_stack[secondary_value(relative_stack_position)]->value->contents.floating < 0 && (long double)execution_stack[initial_value(relative_stack_position)]->value->contents.numeric > (LDBL_MAX + execution_stack[secondary_value(relative_stack_position)]->value->contents.floating))
						overflow_underflow = OVERFLOW;
					else if (execution_stack[secondary_value(relative_stack_position)]->value->contents.floating > 0 && (long double)execution_stack[initial_value(relative_stack_position)]->value->contents.numeric < (-LDBL_MAX + execution_stack[secondary_value(relative_stack_position)]->value->contents.floating))
						overflow_underflow = UNDERFLOW;
					long double result = (execution_stack[initial_value(relative_stack_position)]->value->contents.floating - (long double)execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric);
					run_stack_free_value();
					run_stack_free_value();
					stack_value_t *stack_result = run_stack_value_new(VARIABLE_DOUBLE);
					stack_result->operation_type = VARIABLE_DOUBLE;
					stack_result->value->contents.floating = result;
					run_stack_add_value(stack_result);
				}
				else if (value_types[1] == VARIABLE_DOUBLE)
				{
					if (execution_stack[secondary_value(relative_stack_position)]->value->contents.floating < 0 && execution_stack[initial_value(relative_stack_position)]->value->contents.floating > (LDBL_MAX + execution_stack[secondary_value(relative_stack_position)]->value->contents.floating))
						overflow_underflow = OVERFLOW;
					else if (execution_stack[secondary_value(relative_stack_position)]->value->contents.floating > 0 && execution_stack[initial_value(relative_stack_position)]->value->contents.floating < (-LDBL_MAX + execution_stack[secondary_value(relative_stack_position)]->value->contents.floating))
						overflow_underflow = UNDERFLOW;
					long double result = (execution_stack[initial_value(relative_stack_position)]->value->contents.floating - execution_stack[secondary_value(relative_stack_position)]->value->contents.floating);
					run_stack_free_value();
					run_stack_free_value();
					stack_value_t *stack_result = run_stack_value_new(VARIABLE_DOUBLE);
					stack_result->operation_type = VARIABLE_DOUBLE;
					stack_result->value->contents.floating = result;
					run_stack_add_value(stack_result);
				}
				else
					fatal_error_lined(LOGIC_ERROR, token->line);
			}
			else
				fatal_error_lined(LOGIC_ERROR, token->line);
		}
		else if (c == (signed long long)'*')
		{
			if (value_types[0] == VARIABLE_INT || value_types[0] == VARIABLE_BOOLEAN)
			{
				if (value_types[1] == VARIABLE_INT || value_types[1] == VARIABLE_BOOLEAN)
				{
					if ((execution_stack[initial_value(relative_stack_position)]->value->contents.numeric == -1) && (execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric == LLONG_MIN))
						overflow_underflow = OVERFLOW;
					else if ((execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric == -1) && (execution_stack[initial_value(relative_stack_position)]->value->contents.numeric == LLONG_MIN))
						overflow_underflow = UNDERFLOW;
					else if (execution_stack[initial_value(relative_stack_position)]->value->contents.numeric > (LLONG_MAX / execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric))
						overflow_underflow = OVERFLOW;
					else if (execution_stack[initial_value(relative_stack_position)]->value->contents.numeric < (LLONG_MIN / execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric))
						overflow_underflow = UNDERFLOW;
					signed long long int result = (execution_stack[initial_value(relative_stack_position)]->value->contents.numeric * execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric);
					run_stack_free_value();
					run_stack_free_value();
					stack_value_t *stack_result = run_stack_value_new(VARIABLE_INT);
					stack_result->operation_type = VARIABLE_INT;
					stack_result->value->contents.numeric = result;
					run_stack_add_value(stack_result);
				}
				else if (value_types[1] == VARIABLE_DOUBLE)
				{
					if ((execution_stack[initial_value(relative_stack_position)]->value->contents.numeric == -1) && (execution_stack[secondary_value(relative_stack_position)]->value->contents.floating == -LDBL_MAX))
						overflow_underflow = OVERFLOW;
					else if ((execution_stack[secondary_value(relative_stack_position)]->value->contents.floating == -1) && (execution_stack[initial_value(relative_stack_position)]->value->contents.numeric == -LDBL_MAX))
						overflow_underflow = UNDERFLOW;
					else if (execution_stack[initial_value(relative_stack_position)]->value->contents.numeric > (LDBL_MAX / execution_stack[secondary_value(relative_stack_position)]->value->contents.floating))
						overflow_underflow = OVERFLOW;
					else if (execution_stack[initial_value(relative_stack_position)]->value->contents.numeric < (-LDBL_MAX / execution_stack[secondary_value(relative_stack_position)]->value->contents.floating))
						overflow_underflow = UNDERFLOW;
					long double result = (execution_stack[initial_value(relative_stack_position)]->value->contents.numeric * execution_stack[secondary_value(relative_stack_position)]->value->contents.floating);
					run_stack_free_value();
					run_stack_free_value();
					stack_value_t *stack_result = run_stack_value_new(VARIABLE_DOUBLE);
					stack_result->operation_type = VARIABLE_DOUBLE;
					stack_result->value->contents.floating = result;
					run_stack_add_value(stack_result);
				}
				else
					fatal_error_lined(LOGIC_ERROR, token->line);
			}
			else if (value_types[0] == VARIABLE_DOUBLE)
			{
				if (value_types[1] == VARIABLE_INT || value_types[1] == VARIABLE_BOOLEAN)
				{
					if ((execution_stack[initial_value(relative_stack_position)]->value->contents.floating == -1) && (execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric == -LDBL_MAX))
						overflow_underflow = OVERFLOW;
					else if ((execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric == -1) && (execution_stack[initial_value(relative_stack_position)]->value->contents.floating == -LDBL_MAX))
						overflow_underflow = UNDERFLOW;
					else if (execution_stack[initial_value(relative_stack_position)]->value->contents.floating > (LDBL_MAX / execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric))
						overflow_underflow = OVERFLOW;
					else if (execution_stack[initial_value(relative_stack_position)]->value->contents.floating < (-LDBL_MAX / execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric))
						overflow_underflow = UNDERFLOW;
					long double result = (execution_stack[initial_value(relative_stack_position)]->value->contents.floating * (long double)execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric);
					run_stack_free_value();
					run_stack_free_value();
					stack_value_t *stack_result = run_stack_value_new(VARIABLE_DOUBLE);
					stack_result->operation_type = VARIABLE_DOUBLE;
					stack_result->value->contents.floating = result;
					run_stack_add_value(stack_result);
				}
				else if (value_types[1] == VARIABLE_DOUBLE)
				{
					if ((execution_stack[initial_value(relative_stack_position)]->value->contents.floating == -1) && (execution_stack[secondary_value(relative_stack_position)]->value->contents.floating == -LDBL_MAX))
						overflow_underflow = OVERFLOW;
					else if ((execution_stack[secondary_value(relative_stack_position)]->value->contents.floating == -1) && (execution_stack[initial_value(relative_stack_position)]->value->contents.floating == -LDBL_MAX))
						overflow_underflow = UNDERFLOW;
					else if (execution_stack[initial_value(relative_stack_position)]->value->contents.floating > (LDBL_MAX / execution_stack[secondary_value(relative_stack_position)]->value->contents.floating))
						overflow_underflow = OVERFLOW;
					else if (execution_stack[initial_value(relative_stack_position)]->value->contents.floating < (-LDBL_MAX / execution_stack[secondary_value(relative_stack_position)]->value->contents.floating))
						overflow_underflow = UNDERFLOW;
					long double result = (execution_stack[initial_value(relative_stack_position)]->value->contents.floating * execution_stack[secondary_value(relative_stack_position)]->value->contents.floating);
					run_stack_free_value();
					run_stack_free_value();
					stack_value_t *stack_result = run_stack_value_new(VARIABLE_DOUBLE);
					stack_result->operation_type = VARIABLE_DOUBLE;
					stack_result->value->contents.floating = result;
					run_stack_add_value(stack_result);
				}
				else
					fatal_error_lined(LOGIC_ERROR, token->line);
			}
			else
				fatal_error_lined(LOGIC_ERROR, token->line);
		}
		else if (c == (signed long long)'/')
		{
			if ((value_types[0] == VARIABLE_INT || value_types[0] == VARIABLE_BOOLEAN) && (execution_stack[initial_value(relative_stack_position)]->value->contents.numeric > -LDBL_MAX && execution_stack[initial_value(relative_stack_position)]->value->contents.numeric < LDBL_MAX))
			{
				execution_stack[initial_value(relative_stack_position)]->value->contents.floating = (long double)execution_stack[initial_value(relative_stack_position)]->value->contents.numeric;
				value_types[0] = VARIABLE_DOUBLE;
			}
			if ((value_types[1] == VARIABLE_INT || value_types[1] == VARIABLE_BOOLEAN) && (execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric > -LDBL_MAX && execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric < LDBL_MAX))
			{
				execution_stack[secondary_value(relative_stack_position)]->value->contents.floating = (long double)execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric;
				value_types[1] = VARIABLE_DOUBLE;
			}
			if (value_types[0] == VARIABLE_INT || value_types[0] == VARIABLE_BOOLEAN)
			{
				if (value_types[1] == VARIABLE_INT || value_types[1] == VARIABLE_BOOLEAN)
				{
					if (execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric == 0)
						fatal_error_lined(ZERO_DIVISION_ERROR, token->line);
					signed long long int result = (execution_stack[initial_value(relative_stack_position)]->value->contents.numeric / execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric);
					run_stack_free_value();
					run_stack_free_value();
					stack_value_t *stack_result = run_stack_value_new(VARIABLE_INT);
					stack_result->operation_type = VARIABLE_INT;
					stack_result->value->contents.numeric = result;
					run_stack_add_value(stack_result);
				}
				else if (value_types[1] == VARIABLE_DOUBLE)
				{
					if (execution_stack[secondary_value(relative_stack_position)]->value->contents.floating == 0)
						fatal_error_lined(ZERO_DIVISION_ERROR, token->line);
					long double result = (execution_stack[initial_value(relative_stack_position)]->value->contents.numeric / execution_stack[secondary_value(relative_stack_position)]->value->contents.floating);
					run_stack_free_value();
					run_stack_free_value();
					stack_value_t *stack_result = run_stack_value_new(VARIABLE_DOUBLE);
					stack_result->operation_type = VARIABLE_DOUBLE;
					stack_result->value->contents.floating = result;
					run_stack_add_value(stack_result);
				}
				else
					fatal_error_lined(LOGIC_ERROR, token->line);
			}
			else if (value_types[0] == VARIABLE_DOUBLE)
			{
				if (value_types[1] == VARIABLE_INT || value_types[1] == VARIABLE_BOOLEAN)
				{
					if (execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric == 0)
						fatal_error_lined(ZERO_DIVISION_ERROR, token->line);
					long double result = (execution_stack[initial_value(relative_stack_position)]->value->contents.floating / (long double)execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric);
					run_stack_free_value();
					run_stack_free_value();
					stack_value_t *stack_result = run_stack_value_new(VARIABLE_DOUBLE);
					stack_result->operation_type = VARIABLE_DOUBLE;
					stack_result->value->contents.floating = result;
					run_stack_add_value(stack_result);
				}
				else if (value_types[1] == VARIABLE_DOUBLE)
				{
					if (execution_stack[secondary_value(relative_stack_position)]->value->contents.floating == 0)
						fatal_error_lined(ZERO_DIVISION_ERROR, token->line);
					long double result = (execution_stack[initial_value(relative_stack_position)]->value->contents.floating / execution_stack[secondary_value(relative_stack_position)]->value->contents.floating);
					run_stack_free_value();
					run_stack_free_value();
					stack_value_t *stack_result = run_stack_value_new(VARIABLE_DOUBLE);
					stack_result->operation_type = VARIABLE_DOUBLE;
					stack_result->value->contents.floating = result;
					run_stack_add_value(stack_result);
				}
				else
					fatal_error_lined(LOGIC_ERROR, token->line);
			}
			else
				fatal_error_lined(LOGIC_ERROR, token->line);
		}
		else if (c == (signed long long)'<' || c == (signed long long)'>')
		{
			if (relative_stack_position < 2)
				fatal_error(STACK_REFERENCE_ERROR);
			if (!run_stack_size_assigned_ok(2))
				fatal_error(STACK_REFERENCE_ERROR);
			int value_types[2] = {execution_stack[initial_value(relative_stack_position)]->value->type, execution_stack[secondary_value(relative_stack_position)]->value->type};
			int is_equality = -1;
			if (value_types[0] == VARIABLE_INT || value_types[0] == VARIABLE_BOOLEAN)
			{
				if (value_types[1] == VARIABLE_INT || value_types[1] == VARIABLE_BOOLEAN)
					is_equality = (execution_stack[initial_value(relative_stack_position)]->value->contents.numeric < execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric);
				else if (value_types[1] == VARIABLE_DOUBLE)
					is_equality = (execution_stack[initial_value(relative_stack_position)]->value->contents.numeric < execution_stack[secondary_value(relative_stack_position)]->value->contents.floating);
			}
			else if (value_types[0] == VARIABLE_DOUBLE)
			{
				if (value_types[1] == VARIABLE_INT || value_types[1] == VARIABLE_BOOLEAN)
					is_equality = (execution_stack[initial_value(relative_stack_position)]->value->contents.floating < execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric);
				else if (value_types[1] == VARIABLE_DOUBLE)
					is_equality = (execution_stack[initial_value(relative_stack_position)]->value->contents.floating < execution_stack[secondary_value(relative_stack_position)]->value->contents.floating);
			}
			if (is_equality == -1)
				fatal_error_lined(LOGIC_ERROR, token->line);
			else if (c == (signed long long)'>')
				is_equality = !is_equality;
			run_stack_free_value();
			run_stack_free_value();
			stack_value_t *stack_result = run_stack_value_new(VARIABLE_BOOLEAN);
			stack_result->value->contents.numeric = is_equality;
			stack_result->operation_type = VARIABLE_BOOLEAN;
			run_stack_add_value(stack_result);
		}
		else if (c == (signed long long)'%')
		{
			if (relative_stack_position < 2)
				fatal_error(STACK_REFERENCE_ERROR);
			if (!run_stack_size_assigned_ok(2))
				fatal_error(STACK_REFERENCE_ERROR);
			int value_types[2] = {execution_stack[initial_value(relative_stack_position)]->value->type, execution_stack[secondary_value(relative_stack_position)]->value->type};
			signed long long result_numeric = 0;
			long double result_floating = 0;
			unsigned char result_type = VARIABLE_DOUBLE;
			if (value_types[0] == VARIABLE_INT || value_types[0] == VARIABLE_BOOLEAN)
			{
				if (value_types[1] == VARIABLE_INT || value_types[1] == VARIABLE_BOOLEAN)
				{
					result_type = VARIABLE_INT;
					result_numeric = (execution_stack[initial_value(relative_stack_position)]->value->contents.numeric % execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric);
				}
				else if (value_types[1] == VARIABLE_DOUBLE)
				{
					if (execution_stack[initial_value(relative_stack_position)]->value->contents.numeric > (signed long long)LDBL_MAX)
						overflow_underflow = OVERFLOW;
					else if (execution_stack[initial_value(relative_stack_position)]->value->contents.numeric < (signed long long)-LDBL_MAX)
						overflow_underflow = UNDERFLOW;
					result_floating = fmodl(execution_stack[initial_value(relative_stack_position)]->value->contents.numeric, execution_stack[secondary_value(relative_stack_position)]->value->contents.floating);
				}
				else
					fatal_error_lined(LOGIC_ERROR, token->line);
			}
			else if (value_types[0] == VARIABLE_DOUBLE)
			{
				if (value_types[1] == VARIABLE_INT || value_types[1] == VARIABLE_BOOLEAN)
				{
					if (execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric > (signed long long)LDBL_MAX)
						overflow_underflow = OVERFLOW;
					else if (execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric < (signed long long)-LDBL_MAX)
						overflow_underflow = UNDERFLOW;
					result_floating = fmodl(execution_stack[initial_value(relative_stack_position)]->value->contents.floating, execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric);
				}
				else if (value_types[1] == VARIABLE_DOUBLE)
					result_floating = fmodl(execution_stack[initial_value(relative_stack_position)]->value->contents.floating, execution_stack[secondary_value(relative_stack_position)]->value->contents.floating);
				else
					fatal_error_lined(LOGIC_ERROR, token->line);
			}
			else
				fatal_error_lined(LOGIC_ERROR, token->line);
			run_stack_free_value();
			run_stack_free_value();
			stack_value_t *stack_result = run_stack_value_new(result_type);
			if (result_type == VARIABLE_INT)
				stack_result->value->contents.numeric = result_numeric;
			else
				stack_result->value->contents.floating = result_floating;
			stack_result->operation_type = result_type;
			run_stack_add_value(stack_result);
		}
		else if (c == (signed long long)'!')
		{
			switch (value_types[1])
			{
			case VARIABLE_INT:
			case VARIABLE_BOOLEAN:
			case VARIABLE_DOUBLE:
				execution_stack[relative_stack_position]->value->contents.numeric = !run_stack_state_is_true(0);
				execution_stack[relative_stack_position]->value->type = VARIABLE_BOOLEAN;
				break;
			default:
				fatal_error_lined(LOGIC_ERROR, token->line);
				break;
			}
		}
		else
			syntax_error(INVALID_SYNTAX, token->line);
		break;
	}
	case LITERAL:
	{
		stack_value_t *stack_result = run_stack_value_new(VARIABLE_STRING);
		stack_result->operation_type = VARIABLE_STRING;
		stack_result->value->contents.string = xmalloc(strlen(token->contents.string) + 1);
		if (!copy_string(stack_result->value->contents.string, token->contents.string))
			fatal_error(MEMORY_ALLOCATION_ERROR);
		run_stack_add_value(stack_result);
		break;
	}
	case NUMERIC:
	{
		stack_value_t *stack_result = run_stack_value_new(VARIABLE_INT);
		stack_result->operation_type = VARIABLE_INT;
		stack_result->value->contents.numeric = token->contents.numeric;
		run_stack_add_value(stack_result);
		break;
	}
	case DOUBLE:
	{
		stack_value_t *stack_result = run_stack_value_new(VARIABLE_DOUBLE);
		stack_result->operation_type = VARIABLE_DOUBLE;
		stack_result->value->contents.floating = token->contents.floating;
		run_stack_add_value(stack_result);
		break;
	}
	case ASSIGN:
	{
		if (execution_stack_position < 2)
			fatal_error(LOGIC_ERROR);
		if (token->contents.numeric == 0)
		{
			variable_type_t value_type = execution_stack[secondary_value(relative_stack_position)]->value->type;
			if ((value_type > VARIABLE_IS_UNASSIGNED && value_type < VARIABLE_END) || variable_value_assigned(variable_get_host_id(execution_stack[secondary_value(relative_stack_position)]->value->variable_id_reference[VARIABLE_ID_TYPE_POSITIONAL]), 0))
				variable_set_value(execution_stack[initial_value(relative_stack_position)]->value->variable_id_reference[VARIABLE_ID_TYPE_POSITIONAL], execution_stack[secondary_value(relative_stack_position)]->value->type, execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric, execution_stack[secondary_value(relative_stack_position)]->value->contents.floating, execution_stack[secondary_value(relative_stack_position)]->value->contents.string, execution_stack[secondary_value(relative_stack_position)]->value->contents.array);
			else
				fatal_error(STACK_REFERENCE_ERROR);
		}
		else
		{
			variable_type_t value_type = execution_stack[initial_value(relative_stack_position)]->value->type;
			if ((value_type > VARIABLE_IS_UNASSIGNED && value_type < VARIABLE_END) || variable_value_assigned(variable_get_host_id(execution_stack[initial_value(relative_stack_position)]->value->variable_id_reference[VARIABLE_ID_TYPE_POSITIONAL]), 0))
				variable_set_value(execution_stack[secondary_value(relative_stack_position)]->value->variable_id_reference[VARIABLE_ID_TYPE_POSITIONAL], execution_stack[initial_value(relative_stack_position)]->value->type, execution_stack[initial_value(relative_stack_position)]->value->contents.numeric, execution_stack[initial_value(relative_stack_position)]->value->contents.floating, execution_stack[initial_value(relative_stack_position)]->value->contents.string, execution_stack[initial_value(relative_stack_position)]->value->contents.array);
			else
				fatal_error(STACK_REFERENCE_ERROR);
		}
		run_stack_free_value();
		run_stack_free_value();
		break;
	}
	case EQUALITY:
	case NOT_EQUALITY:
	{
		if (relative_stack_position < 2)
			fatal_error(STACK_REFERENCE_ERROR);
		if (!run_stack_size_assigned_ok(2))
			fatal_error(STACK_REFERENCE_ERROR);
		int value_types[2] = {execution_stack[initial_value(relative_stack_position)]->value->type, execution_stack[secondary_value(relative_stack_position)]->value->type};
		int is_equality = 0;
		if (value_types[0] == VARIABLE_INT || value_types[0] == VARIABLE_BOOLEAN)
		{
			if (value_types[1] == VARIABLE_INT || value_types[1] == VARIABLE_BOOLEAN)
				is_equality = (execution_stack[initial_value(relative_stack_position)]->value->contents.numeric == execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric);
			else if (value_types[1] == VARIABLE_DOUBLE)
				is_equality = (execution_stack[initial_value(relative_stack_position)]->value->contents.numeric == execution_stack[secondary_value(relative_stack_position)]->value->contents.floating);
		}
		else if (value_types[0] == VARIABLE_DOUBLE)
		{
			if (value_types[1] == VARIABLE_INT || value_types[1] == VARIABLE_BOOLEAN)
				is_equality = (execution_stack[initial_value(relative_stack_position)]->value->contents.floating == execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric);
			else if (value_types[1] == VARIABLE_DOUBLE)
				is_equality = (execution_stack[initial_value(relative_stack_position)]->value->contents.floating == execution_stack[secondary_value(relative_stack_position)]->value->contents.floating);
		}
		else if (value_types[0] == VARIABLE_STRING && value_types[1] == VARIABLE_STRING)
			is_equality = (strcmp(execution_stack[initial_value(relative_stack_position)]->value->contents.string, execution_stack[secondary_value(relative_stack_position)]->value->contents.string) == 0);
		if (token->type == NOT_EQUALITY)
			is_equality = !is_equality;
		run_stack_free_value();
		run_stack_free_value();
		stack_value_t *stack_result = run_stack_value_new(VARIABLE_BOOLEAN);
		stack_result->value->contents.numeric = is_equality;
		stack_result->operation_type = token->type;
		run_stack_add_value(stack_result);
		break;
	}
	case LESS_OR_EQUALITY:
	case MORE_OR_EQUALITY:
	{
		if (relative_stack_position < 2)
			fatal_error(STACK_REFERENCE_ERROR);
		if (!run_stack_size_assigned_ok(2))
			fatal_error(STACK_REFERENCE_ERROR);
		int value_types[2] = {execution_stack[initial_value(relative_stack_position)]->value->type, execution_stack[secondary_value(relative_stack_position)]->value->type};
		int is_equality = -1;
		if (value_types[0] == VARIABLE_INT || value_types[0] == VARIABLE_BOOLEAN)
		{
			if (value_types[1] == VARIABLE_INT || value_types[1] == VARIABLE_BOOLEAN)
				is_equality = (execution_stack[initial_value(relative_stack_position)]->value->contents.numeric < execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric);
			else if (value_types[1] == VARIABLE_DOUBLE)
				is_equality = (execution_stack[initial_value(relative_stack_position)]->value->contents.numeric < execution_stack[secondary_value(relative_stack_position)]->value->contents.floating);
		}
		else if (value_types[0] == VARIABLE_DOUBLE)
		{
			if (value_types[1] == VARIABLE_INT || value_types[1] == VARIABLE_BOOLEAN)
				is_equality = (execution_stack[initial_value(relative_stack_position)]->value->contents.floating < execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric);
			else if (value_types[1] == VARIABLE_DOUBLE)
				is_equality = (execution_stack[initial_value(relative_stack_position)]->value->contents.floating < execution_stack[secondary_value(relative_stack_position)]->value->contents.floating);
		}
		if (is_equality == -1)
			fatal_error_lined(LOGIC_ERROR, token->line);
		else if (is_equality == 0)
		{
			if (value_types[0] == VARIABLE_INT || value_types[0] == VARIABLE_BOOLEAN)
			{
				if (value_types[1] == VARIABLE_INT || value_types[1] == VARIABLE_BOOLEAN)
					is_equality = (execution_stack[initial_value(relative_stack_position)]->value->contents.numeric == execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric);
				else if (value_types[1] == VARIABLE_DOUBLE)
					is_equality = (execution_stack[initial_value(relative_stack_position)]->value->contents.numeric == execution_stack[secondary_value(relative_stack_position)]->value->contents.floating);
			}
			else if (value_types[0] == VARIABLE_DOUBLE)
			{
				if (value_types[1] == VARIABLE_INT || value_types[1] == VARIABLE_BOOLEAN)
					is_equality = (execution_stack[initial_value(relative_stack_position)]->value->contents.floating == execution_stack[secondary_value(relative_stack_position)]->value->contents.numeric);
				else if (value_types[1] == VARIABLE_DOUBLE)
					is_equality = (execution_stack[initial_value(relative_stack_position)]->value->contents.floating == execution_stack[secondary_value(relative_stack_position)]->value->contents.floating);
			}
		}
		else if (token->type == MORE_OR_EQUALITY)
			is_equality = !is_equality;
		run_stack_free_value();
		run_stack_free_value();
		stack_value_t *stack_result = run_stack_value_new(VARIABLE_BOOLEAN);
		stack_result->value->contents.numeric = is_equality;
		stack_result->operation_type = token->type;
		run_stack_add_value(stack_result);
		break;
	}
	case AND:
	{
		if (relative_stack_position < 2)
			fatal_error(STACK_REFERENCE_ERROR);
		if (!run_stack_size_assigned_ok(2))
			fatal_error(STACK_REFERENCE_ERROR);
		stack_value_t *stack_result = run_stack_value_new(VARIABLE_BOOLEAN);
		stack_result->value->contents.numeric = (run_stack_state_is_true(0) && run_stack_state_is_true(1));
		stack_result->operation_type = VARIABLE_BOOLEAN;
		run_stack_free_value();
		run_stack_free_value();
		run_stack_add_value(stack_result);
		break;
	}
	case OR:
	{
		if (relative_stack_position < 2)
			fatal_error(STACK_REFERENCE_ERROR);
		if (!run_stack_size_assigned_ok(2))
			fatal_error(STACK_REFERENCE_ERROR);
		stack_value_t *stack_result = run_stack_value_new(VARIABLE_BOOLEAN);
		stack_result->value->contents.numeric = (run_stack_state_is_true(0) || run_stack_state_is_true(1));
		stack_result->operation_type = VARIABLE_BOOLEAN;
		run_stack_free_value();
		run_stack_free_value();
		run_stack_add_value(stack_result);
		break;
	}
	case INSERT:
		break;
	case REMOVE:
		break;
	case IF:
	{
		if (relative_stack_position < 1)
			fatal_error(STACK_REFERENCE_ERROR);
		if (!run_stack_size_assigned_ok(1))
			fatal_error(STACK_REFERENCE_ERROR);
		if (!run_stack_state_is_true(0))
			response.state = IF;
		run_stack_free_value();
		break;
	}
	case ELSE:
	{
		response.state = ELSE;
		break;
	}
	case WHILE:
	{
		if (relative_stack_position < 1)
			fatal_error(STACK_REFERENCE_ERROR);
		if (!run_stack_size_assigned_ok(1))
			fatal_error(STACK_REFERENCE_ERROR);
		if (run_stack_state_is_true(0))
		{
			if (execution_stack_while_references_position == EXECUTION_STACK_SIZE)
				fatal_error(STACK_MEMORY_LIMIT_REACHED);
			execution_stack_while_references[execution_stack_while_references_position++] = execution_scope;
		}
		else
		{
			for (;; relative_stack_position--)
			{
				unsigned char type = execution_stack[relative_stack_position]->operation_type;
				run_stack_free_value();
				if (type == WHILE)
					break;
			}
			response.state = WHILE;
		}
		break;
	}
	case WHILE_START:
	{
		stack_value_t *stack_result = run_stack_value_new(VARIABLE_IS_UNASSIGNED);
		stack_result->value->contents.numeric = (unsigned long long)&(token->next);
		stack_result->operation_type = WHILE;
		stack_result->token = token;
		run_stack_add_value(stack_result);
		break;
	}
	case RETURN:
	{
		response.state = RETURN;
		break;
	}
	case BREAK:
	{
		for (; execution_stack_position > 0; relative_stack_position--)
		{
			if (execution_stack[relative_stack_position]->operation_type == WHILE)
			{
				response.state = BREAK;
				run_stack_free_value();
				break;
			}
			run_stack_free_value();
		}
		if (execution_stack_position == 0)
			fatal_error(STACK_REFERENCE_ERROR);
		break;
	}
	case NEGATE:
	{
		if (relative_stack_position < 1)
			fatal_error(STACK_REFERENCE_ERROR);
		if (!run_stack_size_assigned_ok(1))
			fatal_error(STACK_REFERENCE_ERROR);
		variable_type_t value_type = execution_stack[relative_stack_position]->value->type;
		switch (value_type)
		{
		case VARIABLE_INT:
		case VARIABLE_BOOLEAN:
			execution_stack[relative_stack_position]->value->contents.numeric = -execution_stack[relative_stack_position]->value->contents.numeric;
			break;
		case VARIABLE_DOUBLE:
			execution_stack[relative_stack_position]->value->contents.floating = -execution_stack[relative_stack_position]->value->contents.floating;
			break;
		default:
			fatal_error_lined(LOGIC_ERROR, token->line);
		}
		break;
	}
	case ARRAY_LOCATION:
		break;
	default:
		fatal_error(UNKNOWN_ERROR);
		break;
	}
	switch (overflow_underflow)
	{
	case OVERFLOW:
		output("Integer overflow just occurred.", OUTPUT_WARNING);
		break;
	case UNDERFLOW:
		output("Integer underflow just occurred.", OUTPUT_WARNING);
		break;
	default:
		break;
	}
	return response;
}

void run_stack_value_add_null()
{
	stack_value_t *stack_result = run_stack_value_new(VARIABLE_NULL);
	stack_result->value->contents.numeric = BOOLEAN_FALSE;
	run_stack_add_value(stack_result);
}

int run(parsed_function_scope_t **functions)
{
	parsed_function_scope_t *functions_root = *functions;
	parsed_function_scope_t *current_function = *functions;
	token_t *current_token = current_function->function_token;
	if (current_token->type == NO_OPERATION)
		current_token = current_token->next;
	if (current_token == NULL)
		return 0;
	for (;;)
	{
		struct run_step_state response = run_stack_step(current_token, current_function);
		switch (response.state)
		{
		case FUNCTION_CALL:
		{
			if (response.value < 0)
			{
				switch (response.value)
				{
				case -1: // print
				{
					if (execution_stack_position == 1)
					{
						stack_value_t *stack_result = run_stack_value_new(VARIABLE_NULL);
						stack_result->value->contents.numeric = BOOLEAN_FALSE;
						run_stack_add_value(stack_result);
					}
					else if (execution_stack_position == 0)
						fatal_error(STACK_REFERENCE_ERROR);
					unsigned long long relative_position = (execution_stack_position - 1);
					char *output_string = NULL;
					switch (execution_stack[relative_position]->value->type)
					{
					case VARIABLE_INT:
						output_string = xmalloc(20);
						int_to_str(execution_stack[relative_position]->value->contents.numeric, output_string, 0);
						break;
					case VARIABLE_DOUBLE:
						output_string = xmalloc(50);
						float_to_string(execution_stack[relative_position]->value->contents.floating, output_string);
						break;
					case VARIABLE_STRING:
					{
						output_string = xmalloc(strlen(execution_stack[relative_position]->value->contents.string) + 1);
						if (!copy_string(output_string, execution_stack[relative_position]->value->contents.string))
							fatal_error(MEMORY_ALLOCATION_ERROR);
						break;
					}
					case VARIABLE_BOOLEAN:
						output_string = xmalloc(6);
						bool_to_string(execution_stack[relative_position]->value->contents.numeric, output_string);
						break;
					case VARIABLE_ARRAY:
						break;
					case VARIABLE_NULL:
					{
						output_string = xmalloc(5);
						if (!copy_string(output_string, "null"))
							fatal_error(MEMORY_ALLOCATION_ERROR);
						break;
					}
					default:
						fatal_error_lined(STACK_REFERENCE_ERROR, current_token->line);
						break;
					}
					output(output_string, OUTPUT_GENERIC);
					free(output_string);
					run_stack_free_value();
					run_stack_free_value();
					if (current_token->next != NULL && (current_token->next->type == PARENTHESES_CLOSE || current_token->next->type == BRACKETS_CLOSE || current_token->next->type == OPERATOR || current_token->next->type == ASSIGN || current_token->next->type == FUNCTION_CALL))
						run_stack_value_add_null();
					else
						response.state = VARIABLE_NULL;
					break;
				}
				case -2: // input
				{
					unsigned long long relative_stack_position = (execution_stack_position - 1);
					char operation_type = 0;
					do
					{
						operation_type = execution_stack[relative_stack_position]->operation_type;
						run_stack_free_value();
					} while (operation_type != FUNCTION_CALL_HOST_OPERATION);
					stack_value_t *stack_result = run_stack_value_new(VARIABLE_STRING);
					stack_result->operation_type = VARIABLE_STRING;
					stack_result->value->contents.string = input();
					run_stack_add_value(stack_result);
					break;
				}
				case -3: // convert to string
				{
					if (execution_stack_position <= 1)
						fatal_error(STACK_REFERENCE_ERROR);
					unsigned long long relative_position = (execution_stack_position - 1);
					stack_value_t *stack_result = run_stack_value_new(VARIABLE_STRING);
					stack_result->operation_type = VARIABLE_STRING;
					switch (execution_stack[relative_position]->value->type)
					{
					case VARIABLE_INT:
					{
						stack_result->value->contents.string = xmalloc(20);
						int_to_str(execution_stack[relative_position]->value->contents.numeric, stack_result->value->contents.string, 0);
						break;
					}
					case VARIABLE_DOUBLE:
					{
						stack_result->value->contents.string = xmalloc(50);
						float_to_string(execution_stack[relative_position]->value->contents.floating, stack_result->value->contents.string);
						break;
					}
					case VARIABLE_STRING:
					{
						stack_result->value->contents.string = xmalloc(strlen(execution_stack[relative_position]->value->contents.string) + 1);
						if (!copy_string(stack_result->value->contents.string, execution_stack[relative_position]->value->contents.string))
							fatal_error(MEMORY_ALLOCATION_ERROR);
						break;
					}
					case VARIABLE_BOOLEAN:
					{
						stack_result->value->contents.string = xmalloc(6);
						bool_to_string(execution_stack[relative_position]->value->contents.numeric, stack_result->value->contents.string);
						break;
					}
					case VARIABLE_ARRAY:
						break;
					case VARIABLE_NULL:
					{
						stack_result->value->contents.string = xmalloc(5);
						if (!copy_string(stack_result->value->contents.string, "null"))
							fatal_error(MEMORY_ALLOCATION_ERROR);
						break;
					}
					default:
						fatal_error_lined(CONVERSION_ERROR, current_token->line);
						break;
					}
					run_stack_free_value();
					run_stack_free_value();
					run_stack_add_value(stack_result);
					break;
				}
				case -4: // convert to integer
				{
					if (execution_stack_position <= 1)
						fatal_error(STACK_REFERENCE_ERROR);
					unsigned long long relative_position = (execution_stack_position - 1);
					stack_value_t *stack_result = run_stack_value_new(VARIABLE_INT);
					stack_result->operation_type = VARIABLE_INT;
					switch (execution_stack[relative_position]->value->type)
					{
					case VARIABLE_INT:
						stack_result->value->contents.numeric = execution_stack[relative_position]->value->contents.numeric;
						break;
					case VARIABLE_DOUBLE:
						stack_result->value->contents.numeric = (signed long long)execution_stack[relative_position]->value->contents.floating;
						break;
					case VARIABLE_STRING:
					{
						int is_negative = -1;
						for (size_t i = 0; i < strlen(execution_stack[relative_position]->value->contents.string); i++)
						{
							unsigned char c = execution_stack[relative_position]->value->contents.string[i];
							if ((c < '0' || c > '9') && c != '-')
								fatal_error_lined(CONVERSION_ERROR, current_token->line);
							else if (c == '-')
							{
								if (is_negative != -1)
									fatal_error_lined(CONVERSION_ERROR, current_token->line);
								is_negative = 1;
							}
							if (is_negative != -1)
								is_negative = 0;
							if (i == 18)
								convertion_numeric_warning();
						}
						stack_result->value->contents.numeric = str_to_int(execution_stack[relative_position]->value->contents.string);
						break;
					}
					case VARIABLE_BOOLEAN:
						stack_result->value->contents.numeric = run_stack_state_is_true(0);
						break;
					default:
						fatal_error_lined(CONVERSION_ERROR, current_token->line);
						break;
					}
					run_stack_free_value();
					run_stack_free_value();
					run_stack_add_value(stack_result);
					break;
				}
				case -5: // convert to floating point
				{
					if (execution_stack_position <= 1)
						fatal_error(STACK_REFERENCE_ERROR);
					unsigned long long relative_position = (execution_stack_position - 1);
					stack_value_t *stack_result = run_stack_value_new(VARIABLE_DOUBLE);
					stack_result->operation_type = VARIABLE_DOUBLE;
					switch (execution_stack[relative_position]->value->type)
					{
					case VARIABLE_INT:
					{
						if (execution_stack[relative_position]->value->contents.numeric > (signed long long)LDBL_MAX)
							output("Integer overflow just occurred.", OUTPUT_WARNING);
						else if (execution_stack[relative_position]->value->contents.numeric < (signed long long)LDBL_MAX)
							output("Integer underflow just occurred.", OUTPUT_WARNING);
						stack_result->value->contents.floating = (long double)execution_stack[relative_position]->value->contents.numeric;
						break;
					}
					case VARIABLE_DOUBLE:
						stack_result->value->contents.floating = execution_stack[relative_position]->value->contents.floating;
						break;
					case VARIABLE_STRING:
					{
						int is_negative = -1;
						int is_decimal = 0;
						for (size_t i = 0; i < strlen(execution_stack[relative_position]->value->contents.string); i++)
						{
							unsigned char c = execution_stack[relative_position]->value->contents.string[i];
							if ((c < '0' || c > '9') && c != '-' && c != '.')
								fatal_error_lined(CONVERSION_ERROR, current_token->line);
							else if (c == '-')
							{
								if (is_negative != -1)
									fatal_error_lined(CONVERSION_ERROR, current_token->line);
								is_negative = 1;
							}
							else if (c == '.')
							{
								if (is_decimal == 1)
									fatal_error_lined(CONVERSION_ERROR, current_token->line);
								is_decimal = 1;
							}
							if (is_negative != -1)
								is_negative = 0;
							if (i == 18)
								convertion_floating_warning();
						}
						stack_result->value->contents.floating = str_to_float(execution_stack[relative_position]->value->contents.string);
						break;
					}
					case VARIABLE_BOOLEAN:
						stack_result->value->contents.floating = (long double)run_stack_state_is_true(0);
						break;
					default:
						fatal_error_lined(CONVERSION_ERROR, current_token->line);
						break;
					}
					run_stack_free_value();
					run_stack_free_value();
					run_stack_add_value(stack_result);
					break;
				}
				case -6: // convert to boolean
				{
					if (execution_stack_position <= 1)
						fatal_error(STACK_REFERENCE_ERROR);
					unsigned long long relative_position = (execution_stack_position - 1);
					stack_value_t *stack_result = run_stack_value_new(VARIABLE_BOOLEAN);
					stack_result->operation_type = VARIABLE_BOOLEAN;
					switch (execution_stack[relative_position]->value->type)
					{
					case VARIABLE_INT:
						stack_result->value->contents.numeric = (long double)run_stack_state_is_true(0);
						break;
					case VARIABLE_DOUBLE:
						stack_result->value->contents.numeric = (long double)run_stack_state_is_true(0);
						break;
					case VARIABLE_STRING:
					{
						if (execution_stack[relative_position]->value->contents.string == NULL)
							stack_result->value->contents.numeric = 0;
						else
							stack_result->value->contents.numeric = (long double)(strlen(execution_stack[relative_position]->value->contents.string) > 0);
						break;
					}
					case VARIABLE_BOOLEAN:
						stack_result->value->contents.numeric = (long double)run_stack_state_is_true(0);
						break;
					case VARIABLE_ARRAY:
						stack_result->value->contents.numeric = (long double)(array_length(execution_stack[relative_position]->value->contents.array) > 0);
						break;
					case VARIABLE_NULL:
						stack_result->value->contents.numeric = BOOLEAN_FALSE;
						break;
					default:
						fatal_error_lined(CONVERSION_ERROR, current_token->line);
						break;
					}
					run_stack_free_value();
					run_stack_free_value();
					run_stack_add_value(stack_result);
					break;
				}
				case -7: // get length of value
				{
					if (execution_stack_position <= 1)
						fatal_error(STACK_REFERENCE_ERROR);
					unsigned long long relative_position = (execution_stack_position - 1);
					unsigned long long length = 0;
					switch (execution_stack[relative_position]->value->type)
					{
					case VARIABLE_STRING:
					{
						if (execution_stack[relative_position]->value->contents.string == NULL)
							length = 0;
						else
							length = strlen(execution_stack[relative_position]->value->contents.string);
						break;
					}
					case VARIABLE_ARRAY:
						length = array_length(execution_stack[relative_position]->value->contents.array);
						break;
					default:
						fatal_error_lined(EXECUTION_ERROR, current_token->line);
						break;
					}
					run_stack_free_value();
					run_stack_free_value();
					stack_value_t *stack_result = run_stack_value_new(VARIABLE_INT);
					stack_result->operation_type = VARIABLE_INT;
					stack_result->value->contents.numeric = length;
					run_stack_add_value(stack_result);
					break;
				}
				case -8: // get type of value
				{
					if (execution_stack_position <= 1)
						fatal_error(STACK_REFERENCE_ERROR);
					unsigned long long relative_position = (execution_stack_position - 1);
					variable_type_t type = execution_stack[relative_position]->value->type;
					stack_value_t *stack_result = run_stack_value_new(VARIABLE_STRING);
					stack_result->value->contents.string = xmalloc(8);
					stack_result->operation_type = VARIABLE_STRING;
					int success = 0;
					switch (type)
					{
					case VARIABLE_INT:
						success = copy_string(stack_result->value->contents.string, "integer");
						break;
					case VARIABLE_DOUBLE:
						success = copy_string(stack_result->value->contents.string, "float");
						break;
					case VARIABLE_STRING:
						success = copy_string(stack_result->value->contents.string, "string");
						break;
					case VARIABLE_BOOLEAN:
						success = copy_string(stack_result->value->contents.string, "boolean");
						break;
					case VARIABLE_ARRAY:
						success = copy_string(stack_result->value->contents.string, "list");
						break;
					case VARIABLE_NULL:
						success = copy_string(stack_result->value->contents.string, "null");
						break;
					default:
						fatal_error(UNKNOWN_ERROR);
						break;
					}
					if (!success)
						fatal_error(MEMORY_ALLOCATION_ERROR);
					run_stack_free_value();
					run_stack_free_value();
					run_stack_add_value(stack_result);
					break;
				}
				default:
					fatal_error(STACK_REFERENCE_ERROR);
					break;
				}
				current_token = current_token->next;
			}
			else
			{
				current_function = functions_root->next;
				for (unsigned long long function_id = 1; function_id != response.value && current_function != NULL; function_id++)
					current_function = current_function->next;
				if (current_function == NULL)
					fatal_error_lined(EXECUTION_ERROR, current_token->line);
				current_token = current_function->function_token;
				if (function_scope == (VARIABLE_UNASSIGNED - 1))
					fatal_error(STACK_REFERENCE_ERROR);
				function_scope++;
			}
			break;
		}
		case IF:
		case ELSE:
		case BREAK:
		case WHILE:
		{
			unsigned long long scope = 0;
			unsigned long long scope_exit = 0;
			if (response.state == BREAK)
			{
				scope = execution_scope;
				scope_exit = execution_stack_while_references[--execution_stack_while_references_position];
			}
			if (response.state != BREAK)
			{
				unsigned long long line = current_token->line;
				current_token = current_token->next;
				if (current_token->type != SCOPE_OPEN)
					fatal_error_lined(INVALID_SYNTAX_GENERIC, line);
			}
			for (; current_token != NULL;)
			{
				if (current_token->type == SCOPE_OPEN)
					scope++;
				else if (current_token->type == SCOPE_CLOSE)
					scope--;
				current_token = current_token->next;
				if (scope == scope_exit)
					break;
			}
			if (response.state == IF && current_token->next != NULL && current_token->type == ELSE)
				current_token = current_token->next;
			break;
		}
		case WHILE_START:
		{
			current_token = response.token;
			break;
		}
		case RETURN:
		{
			if (current_function == functions_root)
				syntax_error(RETURN_WITHOUT_FUNCTION, current_token->line);
			current_function = scope_function_references->function;
			current_token = scope_function_references->token->next;
			unsigned long long relative_stack_position = (execution_stack_position - 1);
			stack_value_t *stack_result = run_stack_value_new(execution_stack[relative_stack_position]->value->type);
			stack_result->operation_type = stack_result->value->type;
			switch (stack_result->operation_type)
			{
			case VARIABLE_INT:
			case VARIABLE_BOOLEAN:
			case VARIABLE_NULL:
				stack_result->value->contents.numeric = execution_stack[relative_stack_position]->value->contents.numeric;
				break;
			case VARIABLE_DOUBLE:
				stack_result->value->contents.floating = execution_stack[relative_stack_position]->value->contents.floating;
				break;
			case VARIABLE_STRING:
			{
				stack_result->value->contents.string = xmalloc(strlen(execution_stack[relative_stack_position]->value->contents.string) + 1);
				if (!copy_string(stack_result->value->contents.string, execution_stack[relative_stack_position]->value->contents.string))
					fatal_error(MEMORY_ALLOCATION_ERROR);
				break;
			}
			case VARIABLE_ARRAY:
				stack_result->value->contents.array = array_duplicate(execution_stack[relative_stack_position]->value->contents.array);
				break;
			default:
				fatal_error(UNKNOWN_ERROR);
				break;
			}
			for (char type = NOT_DEFINED;;relative_stack_position--)
			{
				type = execution_stack[relative_stack_position]->operation_type; 
				run_stack_free_value();
				if (type == FUNCTION_CALL_HOST_OPERATION)
					break;
			}
			run_stack_add_value(stack_result);
			execution_scope--;
			function_scope--;
			break;
		}
		default:
		{
			current_token = current_token->next;
			break;
		}
		}
		if (execution_stack_while_references_position > 0)
		{
			for (unsigned long long relative_execution_stack_while_references_position = (execution_stack_while_references_position - 1);;relative_execution_stack_while_references_position--)
			{
				if (execution_scope < execution_stack_while_references[relative_execution_stack_while_references_position])
					relative_execution_stack_while_references_position--;
				if (relative_execution_stack_while_references_position == 0)
					break;
			}
		}
		for (; current_token == NULL;)
		{
			if (current_function == functions_root)
			{
				variable_cleanup(0);
				return 0;
			}
			else if (execution_stack_position > 0)
			{
				unsigned long long relative_stack_position = (execution_stack_position - 1);
				for (;; relative_stack_position--)
				{
					if (execution_stack[relative_stack_position]->operation_type == FUNCTION_CALL_HOST_OPERATION)
						break;
					else if (execution_stack[relative_stack_position]->operation_type == WHILE)
					{
						current_token = execution_stack[relative_stack_position]->token;
						execution_scope--;
						run_stack_free_value();
						break;
					}
					run_stack_free_value();
				}
				if (current_token != NULL)
					break;
				current_function = scope_function_references->function;
				current_token = scope_function_references->token->next;
				variable_cleanup(execution_scope);
				execution_scope--;
				function_scope--;
				run_stack_free_value();
				if (response.state == VARIABLE_NULL && current_token != NULL && current_token->next != NULL && (current_token->next->type == PARENTHESES_CLOSE || current_token->next->type == BRACKETS_CLOSE || current_token->next->type == OPERATOR || current_token->next->type == ASSIGN || current_token->next->type == FUNCTION_CALL))
					run_stack_value_add_null();
			}
			else
				fatal_error(UNKNOWN_ERROR);
		}
	}
	variable_cleanup(0);
	return 0;
}

#endif
