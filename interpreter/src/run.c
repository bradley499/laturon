#ifndef run_c
#define run_c

#include <stdlib.h>
#include <math.h>
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

stack_value_t *execution_stack[EXECUTION_STACK_SIZE];
unsigned long long int execution_stack_position = 0;
unsigned long long function_scope = 0;
unsigned long long execution_scope = 0;

struct scope_states
{
	unsigned long long function_scope;
	unsigned long long execution_scope;
} scope_states;

struct scope_states execution_stack_while_references[EXECUTION_STACK_SIZE];
unsigned long long int execution_stack_while_references_position = 0;

void run_stack_free_value();

void run_stack_reset()
{
	for (;execution_stack_position != 0;)
		run_stack_free_value();
}

stack_value_t *run_stack_value_new(variable_type_t type)
{
	stack_value_t *stack_value_new = xmalloc(sizeof(stack_value_t));
	stack_value_new->value.type = type;
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
	if (execution_stack[--execution_stack_position]->value.type == VARIABLE_STRING)
		free(execution_stack[execution_stack_position]->value.contents.string);
	free(execution_stack[execution_stack_position]);
}

int run_stack_size_assigned_ok(unsigned char size)
{
	if (size > execution_stack_position)
		return 0;
	int relative_stack_position = (execution_stack_position - 1);
	if (!variable_value_assigned(variable_get_host_id(execution_stack[relative_stack_position]->value.variable_id_reference[VARIABLE_ID_TYPE_POSITIONAL]), 0))
		return 0;
	if (size == 2)
		if (!variable_value_assigned(variable_get_host_id(execution_stack[(relative_stack_position - 1)]->value.variable_id_reference[VARIABLE_ID_TYPE_POSITIONAL]), 0))
			return 0;
	return 1;
}

int run_stack_state_is_true(unsigned int relative_position)
{
	if (execution_stack_position < (relative_position + 1))
		fatal_error(STACK_REFERENCE_ERROR);
	int relative_stack_position = (execution_stack_position - relative_position - 1);
	switch (execution_stack[relative_stack_position]->value.type)
	{
	case VARIABLE_INT:
	case VARIABLE_BOOLEAN:
		return (execution_stack[relative_stack_position]->value.contents.numeric == BOOLEAN_TRUE);
		break;
	case VARIABLE_DOUBLE:
		return (execution_stack[relative_stack_position]->value.contents.floating == BOOLEAN_TRUE);
		break;
	case VARIABLE_STRING:
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
} run_step_state;

struct run_step_state run_stack_step(token_t *token)
{
	struct run_step_state response = {
		.state = 0,
		.value = 0,
	};
	enum overflow_underflow_value_t
	{
		OK,
		OVERFLOW,
		UNDERFLOW,
	};
	enum overflow_underflow_value_t overflow_underflow = OK;
	int relative_stack_position = (execution_stack_position - 1);
	switch (token->type)
	{
	case FUNCTION_CALL:
	{
		for (unsigned long long execution_stack_position_negation = (execution_stack_position - 1); execution_stack_position != ULLONG_MAX; execution_stack_position_negation--)
		{
			if (execution_stack[execution_stack_position_negation]->operation_type == FUNCTION_CALL_HOST)
			{
				execution_stack[execution_stack_position_negation]->value.contents.numeric = (signed long long)&token;
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
		stack_result->value.contents.numeric = 0;
		stack_result->operation_type = FUNCTION_CALL_HOST;
		run_stack_add_value(stack_result);
		break;
	}
	case VARIABLE:
	{
		if (token->contents.numeric < VARIABLE_NUMERIC_REFERENCE_START)
		{
			stack_value_t *stack_result = run_stack_value_new(VARIABLE_BOOLEAN);
			stack_result->value.contents.numeric = token->contents.numeric;
			run_stack_add_value(stack_result);
			break;
		}
		variable_value_t variable_value = variable_get_value(token->contents.numeric, function_scope, execution_scope);
		stack_value_t *stack_result = run_stack_value_new(variable_value.type);
		stack_result->value = variable_value;
		run_stack_add_value(stack_result);
		break;
	}
	case BRACKETS_OPEN:
		break;
	case BRACKETS_CLOSE:
		break;
	case SCOPE_OPEN:
		execution_scope++;
		break;
	case SCOPE_CLOSE:
	{
		execution_scope--;
		if (execution_stack_while_references[(execution_stack_while_references_position - 1)].execution_scope == execution_scope)
		{
			for (;;)
			{
				if (execution_stack[relative_stack_position]->operation_type == WHILE)
				{
					response.state = WHILE_START;
					response.value = execution_stack[relative_stack_position]->value.contents.numeric;
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
		if (execution_stack_position < 2)
			fatal_error(LOGIC_ERROR);
		signed long long c = token->contents.numeric;
		if (c == (signed long long)'+')
		{
			if (relative_stack_position < 2)
				fatal_error(STACK_REFERENCE_ERROR);
			int value_types[2] = {execution_stack[initial_value(relative_stack_position)]->value.type, execution_stack[secondary_value(relative_stack_position)]->value.type};
			fatal_error(STACK_REFERENCE_ERROR);
			if (!run_stack_size_assigned_ok(2))
				fatal_error(STACK_REFERENCE_ERROR);
			if (value_types[0] == VARIABLE_INT || value_types[0] == VARIABLE_BOOLEAN)
			{
				if (value_types[1] == VARIABLE_INT || value_types[1] == VARIABLE_BOOLEAN)
				{
					if (execution_stack[initial_value(relative_stack_position)]->value.contents.numeric > 0 && execution_stack[secondary_value(relative_stack_position)]->value.contents.numeric > (INT_MAX - execution_stack[initial_value(relative_stack_position)]->value.contents.numeric))
						overflow_underflow = OVERFLOW;
					else if (execution_stack[initial_value(relative_stack_position)]->value.contents.numeric < 0 && execution_stack[secondary_value(relative_stack_position)]->value.contents.numeric < (INT_MIN - execution_stack[initial_value(relative_stack_position)]->value.contents.numeric))
						overflow_underflow = UNDERFLOW;
					signed long long int result = (execution_stack[initial_value(relative_stack_position)]->value.contents.numeric + execution_stack[secondary_value(relative_stack_position)]->value.contents.numeric);
					run_stack_free_value();
					run_stack_free_value();
					stack_value_t *stack_result = run_stack_value_new(VARIABLE_INT);
					stack_result->value.contents.numeric = result;
					run_stack_add_value(stack_result);
				}
				else if (value_types[1] == VARIABLE_DOUBLE)
				{
					if ((long double)execution_stack[initial_value(relative_stack_position)]->value.contents.numeric > 0 && execution_stack[secondary_value(relative_stack_position)]->value.contents.floating > (DBL_MAX - (long double)execution_stack[initial_value(relative_stack_position)]->value.contents.numeric))
						overflow_underflow = OVERFLOW;
					else if ((long double)execution_stack[initial_value(relative_stack_position)]->value.contents.numeric < 0 && execution_stack[secondary_value(relative_stack_position)]->value.contents.floating < (DBL_MIN - (long double)execution_stack[initial_value(relative_stack_position)]->value.contents.numeric))
						overflow_underflow = UNDERFLOW;
					long double result = (execution_stack[initial_value(relative_stack_position)]->value.contents.numeric + execution_stack[secondary_value(relative_stack_position)]->value.contents.floating);
					run_stack_free_value();
					run_stack_free_value();
					stack_value_t *stack_result = run_stack_value_new(VARIABLE_DOUBLE);
					stack_result->value.contents.floating = result;
					run_stack_add_value(stack_result);
				}
				else if (value_types[1] == VARIABLE_STRING)
					fatal_error(LOGIC_ERROR);
			}
			else if (value_types[0] == VARIABLE_DOUBLE)
			{
				if (value_types[1] == VARIABLE_INT || value_types[1] == VARIABLE_BOOLEAN)
				{
					if (execution_stack[initial_value(relative_stack_position)]->value.contents.numeric > 0 && (long double)execution_stack[secondary_value(relative_stack_position)]->value.contents.numeric > (DBL_MAX - execution_stack[initial_value(relative_stack_position)]->value.contents.numeric))
						overflow_underflow = OVERFLOW;
					else if (execution_stack[initial_value(relative_stack_position)]->value.contents.numeric < 0 && (long double)execution_stack[secondary_value(relative_stack_position)]->value.contents.numeric < (DBL_MIN - execution_stack[initial_value(relative_stack_position)]->value.contents.numeric))
						overflow_underflow = UNDERFLOW;
					long double result = (execution_stack[initial_value(relative_stack_position)]->value.contents.numeric + (long double)execution_stack[secondary_value(relative_stack_position)]->value.contents.numeric);
					run_stack_free_value();
					run_stack_free_value();
					stack_value_t *stack_result = run_stack_value_new(VARIABLE_DOUBLE);
					stack_result->value.contents.floating = result;
					run_stack_add_value(stack_result);
				}
				else if (value_types[1] == VARIABLE_DOUBLE)
				{
					if (execution_stack[initial_value(relative_stack_position)]->value.contents.numeric > 0 && execution_stack[secondary_value(relative_stack_position)]->value.contents.floating > (DBL_MAX - execution_stack[initial_value(relative_stack_position)]->value.contents.numeric))
						overflow_underflow = OVERFLOW;
					else if (execution_stack[initial_value(relative_stack_position)]->value.contents.numeric < 0 && execution_stack[secondary_value(relative_stack_position)]->value.contents.floating < (DBL_MIN - execution_stack[initial_value(relative_stack_position)]->value.contents.numeric))
						overflow_underflow = UNDERFLOW;
					long double result = (execution_stack[initial_value(relative_stack_position)]->value.contents.numeric + execution_stack[secondary_value(relative_stack_position)]->value.contents.floating);
					run_stack_free_value();
					run_stack_free_value();
					stack_value_t *stack_result = run_stack_value_new(VARIABLE_DOUBLE);
					stack_result->value.contents.floating = result;
					run_stack_add_value(stack_result);
				}
				else if (value_types[1] == VARIABLE_STRING)
					fatal_error(LOGIC_ERROR);
			}
		}
		else if (c == (signed long long)'-')
		{
		}
		else if (c == (signed long long)'*')
		{
		}
		else if (c == (signed long long)'/')
		{
		}
		else if (c == (signed long long)'<')
		{
		}
		else if (c == (signed long long)'>')
		{
		}
		else if (c == (signed long long)'%')
		{
		}
		else if (c == (signed long long)'!')
		{
		}
		else
			syntax_error(INVALID_SYNTAX, token->line);
	}
	case LITERAL:
	{
		stack_value_t *stack_result = run_stack_value_new(VARIABLE_STRING);
		stack_result->value.contents.string = xmalloc(strlen(token->contents.string));
		if (strcmp(strcpy(stack_result->value.contents.string, token->contents.string), token->contents.string) != 0)
			fatal_error(MEMORY_ALLOCATION_ERROR);
		run_stack_add_value(stack_result);
		break;
	}
	case NUMERIC:
	{
		stack_value_t *stack_result = run_stack_value_new(VARIABLE_INT);
		stack_result->value.contents.numeric = token->contents.numeric;
		run_stack_add_value(stack_result);
		break;
	}
	case DOUBLE:
	{
		stack_value_t *stack_result = run_stack_value_new(VARIABLE_DOUBLE);
		stack_result->value.contents.floating = token->contents.floating;
		run_stack_add_value(stack_result);
		break;
	}
	case ASSIGN:
	{
		if (execution_stack_position < 2)
			fatal_error(LOGIC_ERROR);
		if (!variable_value_assigned(variable_get_host_id(execution_stack[(relative_stack_position - (1 + token->contents.numeric))]->value.variable_id_reference[VARIABLE_ID_TYPE_POSITIONAL]), 0))
			fatal_error(STACK_REFERENCE_ERROR);
		if (token->contents.numeric == 0)
			variable_set_value(execution_stack[initial_value(relative_stack_position)]->value.variable_id_reference[VARIABLE_ID_TYPE_POSITIONAL], execution_stack[secondary_value(relative_stack_position)]->value.type, execution_stack[secondary_value(relative_stack_position)]->value.contents.numeric, execution_stack[secondary_value(relative_stack_position)]->value.contents.floating, execution_stack[secondary_value(relative_stack_position)]->value.contents.string);
		else
			variable_set_value(execution_stack[secondary_value(relative_stack_position)]->value.variable_id_reference[VARIABLE_ID_TYPE_POSITIONAL], execution_stack[initial_value(relative_stack_position)]->value.type, execution_stack[secondary_value(relative_stack_position)]->value.contents.numeric, execution_stack[secondary_value(relative_stack_position)]->value.contents.floating, execution_stack[secondary_value(relative_stack_position)]->value.contents.string);
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
		int value_types[2] = {execution_stack[initial_value(relative_stack_position)]->value.type, execution_stack[secondary_value(relative_stack_position)]->value.type};
		int is_equality = 0;
		if (value_types[0] == VARIABLE_INT || value_types[0] == VARIABLE_BOOLEAN)
		{
			if (value_types[1] == VARIABLE_INT || value_types[1] == VARIABLE_BOOLEAN)
				is_equality = (execution_stack[initial_value(relative_stack_position)]->value.contents.numeric == execution_stack[secondary_value(relative_stack_position)]->value.contents.numeric);
			else if (value_types[1] == VARIABLE_DOUBLE)
				is_equality = (execution_stack[initial_value(relative_stack_position)]->value.contents.numeric == execution_stack[secondary_value(relative_stack_position)]->value.contents.floating);
		}
		else if (value_types[0] == VARIABLE_DOUBLE)
		{
			if (value_types[1] == VARIABLE_INT || value_types[1] == VARIABLE_BOOLEAN)
				is_equality = (execution_stack[initial_value(relative_stack_position)]->value.contents.floating == execution_stack[secondary_value(relative_stack_position)]->value.contents.numeric);
			else if (value_types[1] == VARIABLE_DOUBLE)
				is_equality = (execution_stack[initial_value(relative_stack_position)]->value.contents.floating == execution_stack[secondary_value(relative_stack_position)]->value.contents.floating);
		}
		else if (value_types[0] == VARIABLE_STRING && value_types[1] == VARIABLE_STRING)
			is_equality = (strcmp(execution_stack[initial_value(relative_stack_position)]->value.contents.string, execution_stack[secondary_value(relative_stack_position)]->value.contents.string) == 0);
		if (token->type == NOT_EQUALITY)
			is_equality = !is_equality;
		run_stack_free_value();
		run_stack_free_value();
		stack_value_t *stack_result = run_stack_value_new(VARIABLE_BOOLEAN);
		stack_result->value.contents.numeric = is_equality;
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
		int value_types[2] = {execution_stack[initial_value(relative_stack_position)]->value.type, execution_stack[secondary_value(relative_stack_position)]->value.type};
		int is_equality = -1;
		if (value_types[0] == VARIABLE_INT || value_types[0] == VARIABLE_BOOLEAN)
		{
			if (value_types[1] == VARIABLE_INT || value_types[1] == VARIABLE_BOOLEAN)
				is_equality = (execution_stack[initial_value(relative_stack_position)]->value.contents.numeric < execution_stack[secondary_value(relative_stack_position)]->value.contents.numeric);
			else if (value_types[1] == VARIABLE_DOUBLE)
				is_equality = (execution_stack[initial_value(relative_stack_position)]->value.contents.numeric < execution_stack[secondary_value(relative_stack_position)]->value.contents.floating);
		}
		else if (value_types[0] == VARIABLE_DOUBLE)
		{
			if (value_types[1] == VARIABLE_INT || value_types[1] == VARIABLE_BOOLEAN)
				is_equality = (execution_stack[initial_value(relative_stack_position)]->value.contents.floating < execution_stack[secondary_value(relative_stack_position)]->value.contents.numeric);
			else if (value_types[1] == VARIABLE_DOUBLE)
				is_equality = (execution_stack[initial_value(relative_stack_position)]->value.contents.floating < execution_stack[secondary_value(relative_stack_position)]->value.contents.floating);
		}
		if (is_equality == -1)
			fatal_error(LOGIC_ERROR);
		else if (is_equality == 0)
		{
			if (value_types[0] == VARIABLE_INT || value_types[0] == VARIABLE_BOOLEAN)
			{
				if (value_types[1] == VARIABLE_INT || value_types[1] == VARIABLE_BOOLEAN)
					is_equality = (execution_stack[initial_value(relative_stack_position)]->value.contents.numeric == execution_stack[secondary_value(relative_stack_position)]->value.contents.numeric);
				else if (value_types[1] == VARIABLE_DOUBLE)
					is_equality = (execution_stack[initial_value(relative_stack_position)]->value.contents.numeric == execution_stack[secondary_value(relative_stack_position)]->value.contents.floating);
			}
			else if (value_types[0] == VARIABLE_DOUBLE)
			{
				if (value_types[1] == VARIABLE_INT || value_types[1] == VARIABLE_BOOLEAN)
					is_equality = (execution_stack[initial_value(relative_stack_position)]->value.contents.floating == execution_stack[secondary_value(relative_stack_position)]->value.contents.numeric);
				else if (value_types[1] == VARIABLE_DOUBLE)
					is_equality = (execution_stack[initial_value(relative_stack_position)]->value.contents.floating == execution_stack[secondary_value(relative_stack_position)]->value.contents.floating);
			}
		}
		else if (token->type == MORE_OR_EQUALITY)
			is_equality = !is_equality;
		run_stack_free_value();
		run_stack_free_value();
		stack_value_t *stack_result = run_stack_value_new(VARIABLE_BOOLEAN);
		stack_result->value.contents.numeric = is_equality;
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
		stack_result->value.contents.numeric = (run_stack_state_is_true(0) && run_stack_state_is_true(1));
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
		stack_result->value.contents.numeric = (run_stack_state_is_true(0) || run_stack_state_is_true(1));
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
		stack_value_t *stack_result = run_stack_value_new(VARIABLE_IS_UNASSIGNED);
		stack_result->value.contents.numeric = (signed long long)&(token->next);
		stack_result->operation_type = WHILE;
		run_stack_add_value(stack_result);
		break;
	}
	case WHILE_START:
	{
		if (relative_stack_position < 1)
			fatal_error(STACK_REFERENCE_ERROR);
		if (!run_stack_size_assigned_ok(1))
			fatal_error(STACK_REFERENCE_ERROR);
		if (run_stack_state_is_true(0))
			execution_stack_while_references[execution_stack_while_references_position++].execution_scope = execution_scope;
		else
		{
			for (;;)
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
	case RETURN:
	{
		for (;;)
		{
			run_stack_free_value();
			if (execution_stack[relative_stack_position]->operation_type == FUNCTION_CALL_HOST)
				break;
		}
		break;
	}
	case BREAK:
	{
		for (;;)
		{
			if (execution_stack[relative_stack_position]->operation_type == WHILE)
			{
				response.state = BREAK;
				response.value = execution_stack[relative_stack_position]->value.contents.numeric;
				run_stack_free_value();
				break;
			}
			run_stack_free_value();
		}
		if (execution_stack_while_references_position == 0)
			fatal_error(STACK_REFERENCE_ERROR);
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

int run(parsed_function_scope_t **functions)
{
	parsed_function_scope_t *functions_root = *functions;
	parsed_function_scope_t *current_function = *functions;
	token_t *current_token = current_function->function_token;
	for (;;)
	{
		struct run_step_state response = run_stack_step(current_token);
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
						stack_value_t *stack_result = run_stack_value_new(VARIABLE_INT);
						stack_result->value.contents.numeric = 0;
						run_stack_add_value(stack_result);
					}
					else if (execution_stack_position == 0)
						fatal_error(STACK_REFERENCE_ERROR);
					unsigned long long relative_position = (execution_stack_position - 1);
					char *output_string = NULL;
					switch (execution_stack[relative_position]->value.type)
					{
					case VARIABLE_INT:
						output_string = xmalloc(20);
						int_to_str(execution_stack[relative_position]->value.contents.numeric, output_string, 0);
						break;
					case VARIABLE_DOUBLE:
						output_string = xmalloc(20);
						float_to_string(execution_stack[relative_position]->value.contents.floating, output_string);
						break;
					case VARIABLE_STRING:
						output_string = execution_stack[relative_position]->value.contents.string;
						break;
					case VARIABLE_BOOLEAN:
						output_string = xmalloc(5);
						bool_to_string(execution_stack[relative_position]->value.contents.numeric, output_string);
						break;
					case VARIABLE_ARRAY:
						break;
					default:
						fatal_error(STACK_REFERENCE_ERROR);
						break;
					}
					output(output_string, OUTPUT_GENERIC);
					run_stack_free_value();
					run_stack_free_value();
					if (current_token->next->type == PARENTHESES_CLOSE || current_token->next->type == BRACKETS_CLOSE || current_token->next->type == OPERATOR || current_token->next->type == ASSIGN || current_token->next->type == FUNCTION_CALL)
					{
						stack_value_t *stack_result = run_stack_value_new(VARIABLE_INT);
						stack_result->value.contents.numeric = 0;
						run_stack_add_value(stack_result);
					}
					break;
				}
				case -2: // input
				{
					stack_value_t *stack_result = run_stack_value_new(VARIABLE_STRING);
					stack_result->value.contents.string = input();
					run_stack_add_value(stack_result);
					break;
				}
				case -3: // convert to string
				{
					break;
				}
				case -4: // convert to integer
				{
					break;
				}
				case -5: // convert to floating point
				{
					break;
				}
				case -6: // convert to boolean
				{
					break;
				}
				case -7: // get length of value
				{
					break;
				}
				default:
				{
					fatal_error(STACK_REFERENCE_ERROR);
					break;
				}
				}
				current_token = current_token->next;
			}
			else
			{
				current_function = functions_root;
				for (unsigned long long function_id = 0; function_id != response.value && current_function != NULL; function_id++)
					current_function = current_function->next;
				if (current_function == NULL)
					fatal_error(EXECUTION_ERROR);
				current_token = current_function->function_token;
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
				scope_exit = execution_stack_while_references[--execution_stack_while_references_position].execution_scope;
			}
			current_token = current_token->next;
			if (current_token->type != SCOPE_OPEN)
				fatal_error(INVALID_SYNTAX_GENERIC);
			for (;current_token != NULL;)
			{
				if (current_token->type == SCOPE_OPEN)
					scope++;
				else if (current_token->type == SCOPE_CLOSE)
					scope--;
				current_token = current_token->next;
				if (scope == scope_exit)
					break;
			}
			if (response.state == IF && current_token->next != NULL && current_token->next->type == ELSE)
				current_token = current_token->next;
			break;
		}
		case WHILE_START:
		{
			current_token = (token_t *)response.value;
			break;
		}
		default:
		{
			current_token = current_token->next;
			break;
		}
		}
		if (current_token == NULL)
		{
			if (current_function == functions_root)
				break;
		}
	}
	return 0;
}

#endif
