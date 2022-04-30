#ifndef parse_c
#define parse_c

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "parse.h"
#include "tokenizer.h"
#include "variable.h"
#include "misc.h"

typedef struct parsed_function_type_t
{
	char *name;
	unsigned long long int parameters;
	struct parsed_function_type_t *next;
} parsed_function_type_t;

typedef struct parsed_parent_relative_position
{
	int position;
	struct parsed_parent_relative_position *parent;
} parsed_parent_relative_position;

parsed_function_type_t *function_definitions = NULL;
parsed_parent_relative_position *relative_position_root = NULL;

// Clean up function references
void parse_cleanup_function_references()
{
	parsed_function_type_t *current_function = function_definitions;
	for (; current_function != NULL;)
	{
		free(current_function->name);
		parsed_function_type_t *remove_function = current_function;
		current_function = current_function->next;
		free(remove_function);
	}
	function_definitions = NULL;
}

// Create a new function object
parsed_function_type_t *parse_function_type_new()
{
	parsed_function_type_t *object = xmalloc(sizeof(parsed_function_type_t));
	object->name = NULL;
	object->next = NULL;
	return object;
}

unsigned long long total_functions = 0;

// Get function scope's numeric reference from `name` and `parameter_count`
signed long long parse_get_function_scope_numeric(char *name, unsigned long long parameter_count)
{
	parsed_function_type_t *current_function = function_definitions;
	if (parameter_count == 0)
	{
		if (strcmp(name, "input") == 0)
			return -3;
	}
	else if (parameter_count == 1)
	{
		if (strcmp(name, "print") == 0)
			return -2;
		else if (strcmp(name, "str") == 0)
			return -4;
		else if (strcmp(name, "int") == 0)
			return -5;
		else if (strcmp(name, "float") == 0)
			return -6;
		else if (strcmp(name, "bool") == 0)
			return -7;
		else if (strcmp(name, "len") == 0)
			return -8;
		else if (strcmp(name, "type") == 0)
			return -9;
	}
	if (current_function == NULL)
		return 0;
	signed long long numeric_reference = 1;
	for (; current_function != NULL; numeric_reference++)
	{
		if (strcmp(name, current_function->name) == 0)
		{
			if (current_function->parameters == parameter_count)
				return (total_functions - numeric_reference + 1);
			return -1;
		}
		current_function = current_function->next;
	}
	return 0;
}

// Create a new function scope
int parse_create_function_scope(char *name)
{
	total_functions++;
	parsed_function_type_t *function_definition = parse_function_type_new();
	if (name != NULL)
	{
		function_definition->name = xmalloc(strlen(name) + 1);
		strcpy(function_definition->name, name);
	}
	else
		fatal_error(INVALID_SYNTAX_GENERIC);
	parsed_function_type_t *current_function = function_definitions;
	if (current_function == NULL)
		function_definitions = function_definition;
	else
	{
		for (signed long long numeric_reference = 1;; numeric_reference++)
		{
			if (numeric_reference == LLONG_MAX)
			{
				free(function_definition);
				fatal_error(TOO_BIG_NUMERIC);
			}
			if (strcmp(name, current_function->name) == 0)
			{
				total_functions--;
				free(function_definition);
				return 0;
			}
			if (current_function->next == NULL)
			{
				current_function->next = function_definition;
				break;
			}
			else
				current_function = current_function->next;
		}
	}
	return 1;
}

void parse_reformat_tokens(token_t **tokens);
void parse_numeric_reformat(parsed_function_scope_t **function_scopes);

int parse_tokens(parsed_function_scope_t **functions)
{
	token_t *tokens = token_get_head();
	if (tokens == NULL)
		return 0;
	token_t *token = xmalloc(sizeof(token_t));
	token->type = NO_OPERATION; // Create a no operation token to allow for rearrangement
	token->next = tokens;
	tokens = token;
	parse_reformat_tokens(&tokens);
	token_t *current_token = tokens;
	token_t *previous_token[2] = {NULL, NULL};
	unsigned long long function_scope_unable = 0;
	token_t *token_root_starting_point = token;
	token_t *next_token = xmalloc(sizeof(token_t));
	parsed_function_scope_t *function_scopes = NULL;
	signed long long active_parameters = 0;
	for (; current_token != NULL;)
	{
		if (function_scope_unable == 1 && !((current_token->type == SCOPE_OPEN && previous_token[0]->type == PARENTHESES_CLOSE) || current_token->type == PARENTHESES_CLOSE) && current_token->type != VARIABLE && !(current_token->type == OPERATOR && current_token->contents.numeric == (int)',') && current_token->type != PARENTHESES_CLOSE && !(current_token->type == PARENTHESES_OPEN && previous_token[0]->type == FUNCTION_DECLARATION))
			syntax_error(INVALID_PARAMETERS, current_token->line);
		switch (current_token->type)
		{
		case NOT_DEFINED:
			fatal_error(UNKNOWN_ERROR);
			break;
		case FUNCTION_DECLARATION:
		{
			if (function_scope_unable == 0)
			{
				if (previous_token[0] != NULL)
					previous_token[0]->next = NULL;
				if (!parse_create_function_scope(current_token->contents.string))
					syntax_error(DUPLICATE_FUNCTION_DEFINITION, current_token->line);
				parsed_function_scope_t *new_function_scope = xmalloc(sizeof(function_scopes));
				new_function_scope->function_token = current_token;
				new_function_scope->next = function_scopes;
				function_scopes = new_function_scope;
				function_scope_unable = 1;
				active_parameters = 0;
			}
			else
				syntax_error(FUNCTION_DEFINITION_RECURSIVE, current_token->line);
			break;
		}
		case SCOPE_OPEN:
		{
			if (function_scope_unable > 0)
				function_scope_unable++;
			else
				previous_token[1] = current_token;
			break;
		}
		case SCOPE_CLOSE:
		{
			if (function_scope_unable > 2)
				function_scope_unable--;
			else if (function_scope_unable == 0)
				previous_token[1] = current_token;
			else if (function_scope_unable == 2)
			{
				function_scope_unable = 0;
				token_t *terminating_token = current_token;
				next_token->next = current_token->next;
				previous_token[0]->next = NULL;
				current_token = next_token;
				free(terminating_token);
				if (current_token->next->type != FUNCTION_DECLARATION && previous_token[1] != NULL)
					previous_token[1]->next = current_token->next;
			}
			break;
		}
		case OPERATOR:
		{
			if (function_scope_unable > 0 && active_parameters != -1 && current_token->contents.numeric == (int)',')
			{
				active_parameters++;
				if (active_parameters == LLONG_MAX)
					syntax_error(PARAMETER_ABUSED, current_token->line);
				if (previous_token[0]->type != PARENTHESES_OPEN && active_parameters == 1)
					active_parameters++;
			}
			break;
		}
		case PARENTHESES_CLOSE:
		{
			if (function_scope_unable > 0 && active_parameters >= 0)
			{
				if (previous_token[0]->type != PARENTHESES_OPEN && active_parameters == 0)
					active_parameters++;
				parsed_function_type_t *current_function = function_definitions;
				if (current_function == NULL)
					fatal_error(UNKNOWN_ERROR);
				for (; current_function->next != NULL;)
					current_function = current_function->next;
				current_function->parameters = active_parameters;
			}
			active_parameters = -1;
			break;
		}
		default:
		{
			if (function_scope_unable == 0)
				previous_token[1] = current_token;
			break;
		}
		}
		previous_token[0] = current_token;
		current_token = current_token->next;
	}
	free(next_token);
	if (token_root_starting_point == NULL)
	{
		parse_cleanup();
		return 0;
	}
	previous_token[1]->next = NULL;
	parsed_function_scope_t *new_function_scope = xmalloc(sizeof(function_scopes));
	new_function_scope->function_token = token_root_starting_point;
	new_function_scope->next = function_scopes;
	function_scopes = new_function_scope;
	*functions = function_scopes;
	parse_numeric_reformat(functions);
	return 1;
}

// Restructure the format of tokens to account for stack machine operations
void parse_reformat_tokens(token_t **tokens)
{
	token_t *root_token = *tokens;
	token_t *current_token = root_token;
	token_t *previous_token[3] = {NULL, NULL, NULL};
	int operator_count = 0;
	for (; current_token != NULL;)
	{
		if (current_token->type == OPERATOR)
		{
			operator_count++;
			if (current_token->contents.numeric == (int)'&' || current_token->contents.numeric == (int)'|')
				syntax_error(INVALID_OPERATION, current_token->line);
			else if (current_token->contents.numeric == (int)'=')
			{
				current_token->type = ASSIGN;
				current_token->contents.numeric = 0;
				operator_count = 0;
			}
			else
			{
				if (current_token->contents.numeric == (int)'-')
				{
					if (operator_count != 1)
						current_token->type = NEGATE;
					else if (previous_token[0] != NULL && (previous_token[0]->type == ASSIGN || previous_token[0]->type == INSERT || previous_token[0]->type == EQUALITY || previous_token[0]->type == NOT_EQUALITY || previous_token[0]->type == LESS_OR_EQUALITY || previous_token[0]->type == MORE_OR_EQUALITY || previous_token[0]->type == AND || previous_token[0]->type == OR || previous_token[0]->type == NEGATE || previous_token[0]->type == BRACKETS_OPEN || previous_token[0]->type == PARENTHESES_OPEN || previous_token[0]->type == RETURN || (previous_token[0]->type == OPERATOR && (previous_token[0]->contents.numeric == (int)'<' || previous_token[0]->contents.numeric == (int)'>'))))
						current_token->type = NEGATE;
				}
				if (current_token->contents.numeric == (int)'+' && operator_count != 1)
				{
					// Remove unnecessary positivity statement
					token_t *remove_token = current_token;
					current_token = current_token->next;
					free(remove_token);
					previous_token[0]->next = current_token;
					continue;
				}
			}
		}
		else if (current_token->type == WHILE)
		{
			int while_structure_valid = 0;
			if (current_token->next != NULL && current_token->next->type == PARENTHESES_OPEN)
				while_structure_valid = 1;
			if (while_structure_valid == 0)
				syntax_error(INVALID_WHILE, current_token->line);
		}
		else
			operator_count = 0;
		previous_token[2] = previous_token[1];
		previous_token[1] = previous_token[0];
		previous_token[0] = current_token;
		current_token = current_token->next;
	}
	if (root_token == NULL)
		syntax_error(INVALID_SYNTAX, 1);
	current_token = root_token;
	previous_token[0] = NULL;
	previous_token[1] = NULL;
	previous_token[2] = NULL;
	struct preserved_call_token
	{
		int counter;
		int parentheses_position;
		int brackets_position;
		int scope_position;
		token_t *token;
		struct preserved_call_token *previous;
	};
	struct array_value_identifiers
	{
		token_t *token;
		struct array_value_identifiers *previous;
	};
	struct preserved_call_token *preserved_call_tokens = NULL;
	int parentheses_position = 0;
	int brackets_position = 0;
	int scope_position = 0;
	int iteration = 0;
	int function_call = 0;
	int function_scope[2] = {0, 0};
	for (; current_token != NULL; iteration++)
	{
		int additional_variable = 0;
		int reorganised = 1;
		int preserve_token = 0;
		switch (current_token->type)
		{
		case FUNCTION_DECLARATION:
		{
			if (function_scope[0] != 0 || function_scope[1] == 1)
				syntax_error(FUNCTION_DEFINITION_RECURSIVE, current_token->line);
			function_scope[1] = 1;
			reorganised = 0;
			break;
		}
		case PARENTHESES_OPEN:
		{
			parentheses_position++;
			reorganised = 0;
			break;
		}
		case PARENTHESES_CLOSE:
		{
			if (parentheses_position == 0)
				break;
			parentheses_position--;
			if (function_call == 0 && function_scope[1] == 1 && function_scope[0] == 0 && current_token->next != NULL && current_token->next->type == SCOPE_OPEN)
				function_scope[0] = (scope_position + 1);
			reorganised = 0;
			if (current_token->next != NULL && (current_token->next->type == VARIABLE || current_token->next->type == LITERAL || current_token->next->type == NUMERIC || current_token->next->type == DOUBLE))
				additional_variable = 1;
			break;
		}
		case BRACKETS_OPEN:
		{
			fatal_error_lined(UNIMPLEMENTED_YET, current_token->line);
			brackets_position++;
			reorganised = 0;
			break;
		}
		case BRACKETS_CLOSE:
		{
			fatal_error_lined(UNIMPLEMENTED_YET, current_token->line);
			if (brackets_position == 0)
				break;
			brackets_position--;
			reorganised = 0;
			if (current_token->next != NULL && (current_token->next->type == VARIABLE || current_token->next->type == LITERAL || current_token->next->type == NUMERIC || current_token->next->type == DOUBLE))
				additional_variable = 1;
			break;
		}
		case SCOPE_OPEN:
		{
			scope_position++;
			reorganised = 0;
			break;
		}
		case SCOPE_CLOSE:
		{
			if (scope_position == 0)
				break;
			scope_position--;
			if (function_scope[1] != 0 && (function_scope[0] - 1) == scope_position)
			{
				function_scope[0] = 0;
				function_scope[1] = 0;
			}
			reorganised = 0;
			break;
		}
		case INSERT:
		case REMOVE:
			fatal_error_lined(UNIMPLEMENTED_YET, current_token->line);
			break;
		case FUNCTION_CALL:
		case IF:
		case WHILE:
		case ASSIGN:
		case EQUALITY:
		case NOT_EQUALITY:
		case LESS_OR_EQUALITY:
		case MORE_OR_EQUALITY:
		case AND:
		case OR:
		case OPERATOR:
		case RETURN:
		case NEGATE:
		{
			if (current_token->type == ASSIGN && (parentheses_position > 0 || brackets_position > 0))
				syntax_error(INVALID_OPERATION, current_token->line);
			if (previous_token[0] == NULL)
			{
				switch (current_token->type)
				{
				case ASSIGN:
				case EQUALITY:
				case NOT_EQUALITY:
				case LESS_OR_EQUALITY:
				case MORE_OR_EQUALITY:
				case AND:
				case OR:
				case INSERT:
				case OPERATOR:
				case RETURN:
					syntax_error(INVALID_OPERATION, current_token->line);
					break;
				case NEGATE:
					syntax_error(INVALID_SYNTAX, current_token->line);
					break;
				default:
					break;
				}
				reorganised = 1;
			}
			if (current_token->type == OPERATOR)
				if (current_token->contents.numeric == (int)',')
					break;
			if (current_token->type == RETURN)
				if (current_token->next != NULL && current_token->next->type == SCOPE_CLOSE)
					syntax_error(EMPTY_RETURN, current_token->line);
			switch (current_token->type)
			{
			case EQUALITY:
			case NOT_EQUALITY:
			case LESS_OR_EQUALITY:
			case MORE_OR_EQUALITY:
			case AND:
			case OR:
			case ASSIGN:
			case INSERT:
				if (current_token->next == NULL || current_token->next->type == SCOPE_CLOSE || current_token->next->type == BRACKETS_CLOSE || current_token->next->type == PARENTHESES_CLOSE)
					syntax_error(INVALID_SYNTAX, ((current_token->next != NULL) ? current_token->next->line : current_token->line));
				break;
			default:
				break;
			}
			switch (current_token->type)
			{
			case EQUALITY:
			case NOT_EQUALITY:
			case LESS_OR_EQUALITY:
			case MORE_OR_EQUALITY:
			case AND:
			case OR:
			case ASSIGN:
			case INSERT:
			case RETURN:
			{
				if (previous_token[0] == NULL)
					syntax_error(INVALID_SYNTAX, current_token->line);
				if (current_token->type == RETURN && function_scope[1] == 0)
					syntax_error(RETURN_WITHOUT_FUNCTION, current_token->line);
				preserve_token = 1;
				break;
			}
			case FUNCTION_CALL:
			{
				if (current_token->next == NULL)
					syntax_error(LINE_ENDED_INCORRECTLY, current_token->line);
				function_call++;
				if (current_token->next->type == PARENTHESES_OPEN)
					current_token->next->type = FUNCTION_CALL_PARAMETERS;
				preserve_token = 4;
				previous_token[2] = current_token;
				break;
			}
			case OPERATOR:
			{
				if (current_token->next != NULL && current_token->next->type == SCOPE_CLOSE)
					syntax_error(INVALID_SYNTAX, current_token->line);
				preserve_token = 1;
				break;
			}
			case NEGATE:
			{
				preserve_token = 1;
				break;
			}
			case IF:
				preserve_token = 1;
				break;
			case WHILE:
				preserve_token = 5;
				break;
			default:
				break;
			}
			break;
		}
		case VARIABLE:
		case LITERAL:
		case NUMERIC:
		case DOUBLE:
		{
			reorganised = 0;
			if (current_token->next != NULL && (current_token->next->type == VARIABLE || current_token->next->type == LITERAL || current_token->next->type == NUMERIC || current_token->next->type == DOUBLE))
				additional_variable = 1;
			break;
		}
		default:
		{
			reorganised = 0;
			break;
		}
		}
		if (iteration == 0 && reorganised == 1)
		{
			if (current_token->next != NULL)
			{
				if (current_token->next->type != PARENTHESES_OPEN)
				{
					root_token = current_token->next;
					*tokens = root_token;
				}
				else
					iteration--;
			}
			else
				syntax_error(INVALID_SYNTAX, current_token->line);
		}
		int preserved_function_call = 0;
		int reinstert_enabler = 0;
		if (preserve_token == 1 || preserve_token == 4 || preserve_token == 5)
		{
			// Storing for later insertion
			struct preserved_call_token *preserved_token = xmalloc(sizeof(struct preserved_call_token));
			if (current_token->type == FUNCTION_CALL)
				preserved_function_call = 1;
			preserved_token->token = current_token;
			preserved_token->counter = 2;
			preserved_token->parentheses_position = parentheses_position;
			preserved_token->brackets_position = brackets_position;
			preserved_token->scope_position = scope_position;
			preserved_token->previous = preserved_call_tokens;
			preserved_call_tokens = preserved_token;
			switch (preserve_token)
			{
			case 1:
			{
				if (previous_token[0] != NULL)
					previous_token[0]->next = current_token->next;
				else
					previous_token[0] = current_token;
				break;
			}
			case 4:
			{
				token_t *new_token = xmalloc(sizeof(token_t));
				memcpy(new_token, current_token, sizeof(token_t));
				new_token->type = FUNCTION_CALL_HOST; // Declare start of function call
				if (previous_token[0] != NULL)
					previous_token[0]->next = new_token;
				else
				{
					new_token->next = current_token->next;
					previous_token[0] = new_token;
					root_token = new_token;
					*tokens = root_token;
				}
				break;
			}
			case 5:
			{
				token_t *new_token = xmalloc(sizeof(token_t));
				memcpy(new_token, current_token, sizeof(token_t));
				new_token->type = WHILE_START; // Declare start of while loop
				if (previous_token[0] != NULL)
					previous_token[0]->next = new_token;
				else
				{
					new_token->next = current_token->next;
					previous_token[0] = new_token;
					root_token = new_token;
					*tokens = root_token;
				}
				break;
			}
			}
			current_token->supporting_reference = previous_token[0];
			current_token = current_token->next;
			reinstert_enabler = 1;
		}
		else if (current_token->type != PARENTHESES_CLOSE && current_token->type != BRACKETS_CLOSE && (current_token->next != NULL && current_token->next->type == OPERATOR))
			reinstert_enabler = 1;
		if (preserved_function_call == 1)
			parentheses_position++;
		int reinsertable = reinstert_enabler;
		if (reinsertable == 0 && current_token->type != OPERATOR)
		{
			if (preserve_token != 0)
			{
				if (preserved_function_call == 0)
				{
					if (preserved_call_tokens != NULL && preserved_call_tokens->parentheses_position == preserved_call_tokens->previous->parentheses_position)
						reinsertable = (preserved_call_tokens != NULL && preserved_call_tokens->previous != NULL && preserved_call_tokens->previous->parentheses_position == parentheses_position && preserved_call_tokens->previous->brackets_position == brackets_position && preserved_call_tokens->previous->scope_position == scope_position);
				}
			}
			else if (preserved_function_call == 0)
				reinsertable = (preserved_call_tokens != NULL && preserved_call_tokens->parentheses_position == parentheses_position && preserved_call_tokens->brackets_position == brackets_position && preserved_call_tokens->scope_position == scope_position);
		}
		else
			reinsertable = 0;
		if (reinsertable)
		{
			// Reinserting previously stored values
			int additional_insertion = 0;
			do
			{
				if (additional_insertion == 1)
					current_token = current_token->next;
				struct preserved_call_token *preserved_token = preserved_call_tokens;
				if (preserve_token != 0)
				{
					preserved_token = preserved_token->previous;
					preserved_call_tokens->previous = preserved_call_tokens->previous;
				}
				else
					preserved_call_tokens = preserved_call_tokens->previous;
				token_t *restored_token = preserved_token->token;
				if (restored_token->type == FUNCTION_CALL)
					function_call--;
				if (additional_insertion != 1 && (restored_token->type == ASSIGN || restored_token->type == EQUALITY || restored_token->type == NOT_EQUALITY || restored_token->type == LESS_OR_EQUALITY || restored_token->type == MORE_OR_EQUALITY || restored_token->type == AND || restored_token->type == OR || restored_token->type == INSERT || restored_token->type == RETURN) && current_token->type != LITERAL && current_token->type != NUMERIC && current_token->type != DOUBLE && current_token->type != VARIABLE && current_token->next != NULL && !(current_token->next->type == VARIABLE || current_token->next->type == FUNCTION_CALL || current_token->next->type == WHILE || current_token->next->type == FUNCTION_DECLARATION || current_token->next->type == PARENTHESES_OPEN || current_token->next->type == SCOPE_OPEN || current_token->next->type == BRACKETS_OPEN))
				{
					restored_token->next = current_token;
					previous_token[2] = previous_token[1];
					previous_token[1] = previous_token[0];
					previous_token[0]->next = restored_token;
					previous_token[0] = restored_token;
				}
				else
				{
					restored_token->next = current_token->next;
					current_token->next = restored_token;
					previous_token[2] = previous_token[1];
					previous_token[1] = previous_token[0];
					previous_token[0] = current_token;
				}
				free(preserved_token);
				reinsertable = 0;
				if (preserve_token != 0)
					reinsertable = (preserved_call_tokens != NULL && preserved_call_tokens->previous != NULL && preserved_call_tokens->previous->parentheses_position == parentheses_position && preserved_call_tokens->previous->brackets_position == brackets_position && preserved_call_tokens->previous->scope_position == scope_position);
				else
					reinsertable = (preserved_call_tokens != NULL && preserved_call_tokens->parentheses_position == parentheses_position && preserved_call_tokens->brackets_position == brackets_position && preserved_call_tokens->scope_position == scope_position);
				if (reinsertable)
					additional_insertion = 1;
			} while (reinsertable);
			preserve_token = 2;
		}
		else if (additional_variable == 1)
			syntax_error(DANGLING_VARIABLE_REFERENCE, current_token->line);
		else if (preserve_token == 1 || preserve_token == 4 || preserve_token == 5)
			continue;
		else if (preserve_token < 2)
			preserve_token = 1;
		else if (preserve_token == 3)
			continue;
		for (; (preserve_token > 0 && current_token != NULL); preserve_token--)
		{
			previous_token[2] = previous_token[1];
			previous_token[1] = previous_token[0];
			previous_token[0] = current_token;
			current_token = current_token->next;
		}
	}
	int additional_insertion = 0;
	for (; (preserved_call_tokens != NULL && preserved_call_tokens->parentheses_position == parentheses_position && preserved_call_tokens->brackets_position == brackets_position && preserved_call_tokens->scope_position == scope_position);)
	{
		// Reinserting previously stored values
		if (additional_insertion == 1)
			current_token = current_token->next;
		struct preserved_call_token *preserved_token = preserved_call_tokens;
		preserved_call_tokens = preserved_call_tokens->previous;
		token_t *restored_token = preserved_token->token;
		restored_token->next = current_token->next;
		current_token->next = restored_token;
		free(preserved_token);
		additional_insertion = 1;
	}
	if (additional_insertion == 1)
		current_token->next = NULL;
	if (preserved_call_tokens != NULL)
		syntax_error(SCOPE_CLOSE_UNABLE, preserved_call_tokens->token->line);
}

typedef struct parsed_variable_type_t
{
	unsigned long long numeric_reference;
	signed long long function_numeric_reference;
	char *name;
	struct parsed_variable_type_t *next;
} parsed_variable_type_t;

parsed_variable_type_t *parse_variable_references = NULL;

// Clean up variable references
void parse_cleanup_variable_references()
{
	parsed_variable_type_t *current_variable = parse_variable_references;
	for (; current_variable != NULL;)
	{
		parsed_variable_type_t *remove_variable = current_variable;
		current_variable = current_variable->next;
		free(current_variable->name);
		free(remove_variable);
	}
	parse_variable_references = NULL;
}

// Assign a numeric reference to a variable by name and scope
variable_id parse_get_variable_numeric(char *name, int function_numeric_reference, int is_parameter)
{
	if (strcmp(name, "false") == 0)
		return BOOLEAN_FALSE;
	else if (strcmp(name, "true") == 0)
		return BOOLEAN_TRUE;
	else if (strcmp(name, "null") == 0)
		return VALUE_NULL;
	parsed_variable_type_t *new_variable = xmalloc(sizeof(parsed_variable_type_t));
	new_variable->name = xmalloc(strlen(name) + 1);
	new_variable->function_numeric_reference = function_numeric_reference;
	strcpy(new_variable->name, name);
	if (parse_variable_references == NULL)
	{
		new_variable->numeric_reference = VARIABLE_NUMERIC_REFERENCE_START;
		parse_variable_references = new_variable;
		return VARIABLE_NUMERIC_REFERENCE_START;
	}
	parsed_variable_type_t *current_variable = parse_variable_references;
	variable_id numeric_reference = VARIABLE_NUMERIC_REFERENCE_START;
	for (; current_variable != NULL; numeric_reference++)
	{
		if (numeric_reference == VARIABLE_UNASSIGNED)
		{
			free(new_variable);
			fatal_error(TOO_BIG_NUMERIC);
		}
		if (strcmp(current_variable->name, name) == 0)
		{
			if (is_parameter == 1)
				return -1;
			if (current_variable->function_numeric_reference == 0 || current_variable->function_numeric_reference == function_numeric_reference)
				return current_variable->numeric_reference;
		}
		if (current_variable->next == NULL)
		{
			new_variable->numeric_reference = ++numeric_reference;
			current_variable->next = new_variable;
			return numeric_reference;
		}
		current_variable = current_variable->next;
	}
	fatal_error(UNKNOWN_ERROR);
	return 0;
}

// Convertion function and variable references to numeric values
void parse_numeric_reformat(parsed_function_scope_t **function_scopes)
{
	parsed_function_scope_t *root_function = *function_scopes;
	parsed_function_scope_t *current_function = root_function;
	for (signed long long function_count = 0; current_function != NULL; function_count++)
	{
		if (function_count == LLONG_MAX)
			fatal_error(TOO_BIG_NUMERIC);
		token_t *current_token = current_function->function_token;
		token_t *previous_token = NULL;
		int within_array = 0;
		struct function_call_parameter
		{
			unsigned long long count;
			unsigned long long array_scope;
			struct function_call_parameter *parent;
		};
		struct function_call_parameter *parameter_stack = NULL;
		struct function_parameter_variable
		{
			variable_id variable_id;
			struct function_parameter_variable *previous;
		};
		struct function_parameter_variable *parameter_variable_stack = NULL;
		int is_function_declaration = 0;
		int is_function_parameter = 0;
		for (; current_token != NULL;)
		{
			int removable = 0;
			switch (current_token->type)
			{
			case FUNCTION_DECLARATION:
			{
				is_function_declaration = 1;
				break;
			}
			case FUNCTION_CALL:
			{
				if (parameter_stack == NULL)
					syntax_error(INVALID_FUNCTION_PARAMETERS, current_token->line);
				signed long long function_id = parse_get_function_scope_numeric(current_token->contents.string, parameter_stack->count);
				struct function_call_parameter *parameters = parameter_stack;
				parameter_stack = parameters->parent;
				free(parameters);
				if (function_id == 0)
					syntax_error(NO_FUNCTION_REFERENCE, current_token->line);
				else if (function_id == -1)
					syntax_error(INVALID_FUNCTION_PARAMETERS, current_token->line);
				else if (function_id < -1)
					function_id++;
				free(current_token->contents.string);
				current_token->contents.numeric = function_id;
				break;
			}
			case VARIABLE:
			{
				variable_id variable_id = parse_get_variable_numeric(current_token->contents.string, function_count, is_function_parameter);
				free(current_token->contents.string);
				if (is_function_parameter == 1 && variable_id == -1)
					syntax_error(INVALID_PARAMETER_NAME_GLOBAL, current_token->line);
				current_token->contents.numeric = variable_id;
				if (is_function_declaration)
				{
					struct function_parameter_variable *parameter = xmalloc(sizeof(struct function_parameter_variable));
					parameter->variable_id = variable_id;
					parameter->previous = parameter_variable_stack;
					parameter_variable_stack = parameter;
				}
				break;
			}
			case FUNCTION_CALL_PARAMETERS:
			{
				struct function_call_parameter *parameters = xmalloc(sizeof(struct function_call_parameter));
				if (current_token->next != NULL && current_token->next->type != PARENTHESES_CLOSE)
					parameters->count = 1;
				parameters->array_scope = within_array;
				if (parameter_stack == NULL)
					parameter_stack = parameters;
				else
				{
					parameters->parent = parameter_stack;
					parameter_stack = parameters;
				}
				removable = 1;
			}
			case OPERATOR:
			{
				if (current_token->contents.numeric == (int)',')
				{
					if (is_function_declaration == 0)
					{
						if (parameter_stack == NULL && within_array == 0)
							syntax_error(INVALID_SYNTAX, current_token->line);
						if (parameter_stack != NULL && parameter_stack->array_scope == within_array)
							parameter_stack->count++;
					}
					removable = 1;
				}
				break;
			}
			case PARENTHESES_OPEN:
			{
				if (is_function_declaration == 1)
				{
					if (is_function_parameter == 1)
						syntax_error(INVALID_PARAMETERS, current_token->line);
					else
						is_function_parameter = 1;
				}
				removable = 1;
				break;
			}
			case BRACKETS_OPEN:
			{
				within_array++;
				break;
			}
			case BRACKETS_CLOSE:
			{
				within_array--;
				break;
			}
			case SCOPE_OPEN:
			{
				if (is_function_declaration == 1)
				{
					is_function_declaration = 0;
					unsigned long long line = 0;
					if (previous_token != NULL)
						line = previous_token->line;
					for (; parameter_variable_stack != NULL;)
					{
						token_t *next_token = current_token->next;
						token_t *parameter_variable_assign = xmalloc(sizeof(token_t));
						parameter_variable_assign->line = line;
						parameter_variable_assign->type = VARIABLE;
						struct function_parameter_variable *current_parameter = parameter_variable_stack;
						parameter_variable_assign->contents.numeric = current_parameter->variable_id;
						parameter_variable_stack = parameter_variable_stack->previous;
						free(current_parameter);
						current_token->next = parameter_variable_assign;
						current_token = current_token->next;
						token_t *parameter_assign = xmalloc(sizeof(token_t));
						parameter_assign->next = next_token;
						parameter_assign->line = line;
						parameter_assign->type = ASSIGN;
						parameter_assign->contents.numeric = 1;
						current_token->next = parameter_assign;
						current_token = current_token->next;
					}
				}
				break;
			}
			case SCOPE_CLOSE:
			{
				if (current_token->next == NULL && function_count > 0)
					removable = 1;
				break;
			}
			case PARENTHESES_CLOSE:
			{
				removable = 1;
				is_function_parameter = 0;
				break;
			}
			default:
				break;
			}
			if (is_function_declaration == 1)
				removable = 1;
			if (removable == 1)
			{
				if (previous_token != NULL)
					previous_token->next = current_token->next;
				else
					current_function->function_token = current_token->next;
				token_t *remove = current_token;
				current_token = current_token->next;
				free(remove);
				continue;
			}
			previous_token = current_token;
			current_token = current_token->next;
		}
		if (parameter_stack != NULL)
			fatal_error(STACK_REFERENCE_ERROR);
		current_function = current_function->next;
	}
}

void parse_cleanup()
{
	parsed_function_type_t *function_definition = function_definitions;
	for (; function_definition != NULL;)
	{
		parsed_function_type_t *function_definition_next = function_definition->next;
		if (function_definition->name != NULL)
			free(function_definition->name);
		parsed_function_type_t *object_current = function_definition;
		free(object_current);
		function_definition = function_definition_next;
	}
	parse_cleanup_function_references();
	parse_cleanup_variable_references();
}

#endif
