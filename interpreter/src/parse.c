#ifndef parse_c
#define parse_c

#include <stdlib.h>
#include <string.h>
#include "parse.h"
#include "tokenizer.h"
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

int parse_init()
{
	parse_cleanup();
	return 1;
}

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

// Get function scope's numeric reference from `name` and `parameter_count`
int parse_get_function_scope_numeric(char *name, int parameter_count)
{
	parsed_function_type_t *current_function = function_definitions;
	if (current_function == NULL)
		fatal_error(STACK_REFERENCE_ERROR);
	int numeric_reference = 1;
	for (; current_function != NULL; numeric_reference++)
	{
		if (strcmp(name, current_function->name) == 0)
		{
			if (current_function->parameters == parameter_count)
				return numeric_reference;
			return -1;
		}
		current_function = current_function->next;
	}
	if (strcmp(name, "print") == 0)
		return -2;
	else if (strcmp(name, "input") == 0)
		return -3;
	else if (strcmp(name, "str") == 0)
		return -4;
	else if (strcmp(name, "int") == 0)
		return -5;
	else
		return 0;
}

// Create a new function scope
int parse_create_function_scope(char *name)
{
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
		for (int numeric_reference = 1;; numeric_reference++)
		{
			if (strcmp(name, current_function->name) == 0)
			{
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
	parse_reformat_tokens(&tokens);
	token_t *current_token = tokens;
	token_t *previous_token[2] = {NULL, NULL};
	int function_scope_unable = 0;
	token_t *token_root_starting_point = NULL;
	token_t *next_token = xmalloc(sizeof(token_t));
	parsed_function_scope_t *function_scopes = NULL;
	int active_parameters = 0;
	for (; current_token != NULL;)
	{
		if (function_scope_unable == 1 && !((current_token->type == SCOPE_OPEN && previous_token[0]->type == PARENTHESES_CLOSE) || current_token->type == SCOPE_CLOSE) && current_token->type != VARIABLE && !(current_token->type == OPERATOR && current_token->contents.numeric == (int)',') && current_token->type != PARENTHESES_CLOSE && !(current_token->type == PARENTHESES_OPEN && previous_token[0]->type == FUNCTION_DECLARATION))
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
				active_parameters = (current_token->next != NULL && current_token->next->type != PARENTHESES_CLOSE);
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
			if (token_root_starting_point == NULL && function_scope_unable == 0)
				token_root_starting_point = current_token;
			else if (function_scope_unable == 0)
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
	int operator_signing = 0;
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
				current_token->contents.string = NULL;
			}
			else if (operator_count != 1)
			{
				if (current_token->contents.numeric == (int)'-')
				{
					operator_signing++;
					if (operator_signing % 2 == 0)
					{
						if (previous_token[1] == NULL)
							syntax_error(INVALID_OPERATION, current_token->line);
						previous_token[1]->next = current_token->next;
						current_token->contents.string = NULL;
						previous_token[0]->contents.string = NULL;
						free(previous_token[0]);
						free(current_token);
						previous_token[1] = previous_token[2];
					}
				}
				else if (current_token->contents.numeric == (int)'+')
				{
					// Remove unnecessary positivity statement
					token_t *next_token = current_token->next;
					current_token->contents.string = NULL;
					token_t *remove_token = current_token;
					current_token = next_token;
					free(remove_token);
					previous_token[0]->next = next_token;
					continue;
				}
			}
		}
		else if (current_token->type == WHILE)
		{
			int while_structure_valid = 0;
			if (current_token->next != NULL)
			{
				if (current_token->next->type == SCOPE_OPEN)
				{
					while_structure_valid = 1;
					token_t *new_token = xmalloc(sizeof(token_t));
					new_token->line = current_token->line;
					new_token->next = current_token->next;
					new_token->type = IF;
					current_token->next = new_token;
				}
			}
			if (while_structure_valid == 0)
				syntax_error(INVALID_WHILE, current_token->line);
		}
		else
		{
			operator_count = 0;
			operator_signing = 0;
		}
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
	int special_operation[3] = {0, 0, 0}; // type, position increment (0 means insert), insert allowed
	for (; current_token != NULL; iteration++)
	{
		int reorganised = 1;
		int preserve_token = 0;
		switch (current_token->type)
		{
		case PARENTHESES_OPEN:
		case FUNCTION_CALL_PARAMETERS:
		{
			parentheses_position++;
			if (special_operation[0] != 0)
				special_operation[1]++;
			break;
		}
		case PARENTHESES_CLOSE:
		{
			if (parentheses_position == 0)
				break;
			parentheses_position--;
			if (special_operation[0] != 0 && (current_token->next == NULL || (current_token->next != NULL && current_token->next->type != OPERATOR)))
				special_operation[1]--;
			if (function_call > 0)
				function_call--;
			break;
		}
		case BRACKETS_OPEN:
		{
			brackets_position++;
			break;
		}
		case BRACKETS_CLOSE:
		{
			if (brackets_position == 0)
				break;
			brackets_position--;
		}
		case SCOPE_OPEN:
		{
			scope_position++;
			break;
		}
		case SCOPE_CLOSE:
		{
			if (scope_position == 0)
				break;
			scope_position--;
			break;
		}
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
		case INSERT:
		case OPERATOR:
		case RETURN:
		{
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
				default:
					break;
				}
				reorganised = 1;
			}
			if (current_token->type == OPERATOR)
			{
				if (current_token->contents.numeric == (int)',')
				{
					break;
				}
			}
			switch (current_token->type)
			{
			case ASSIGN:
			case INSERT:
			case RETURN:
			{
				if (previous_token[0] == NULL)
					syntax_error(INVALID_SYNTAX, current_token->line);
				if (special_operation[0] != 0)
					syntax_error(INVALID_OPERATION, current_token->line);
				special_operation[0] = current_token->type;
				special_operation[1] = 1;
				special_operation[2] = 0;
				token_t *token_remove = current_token;
				previous_token[0]->next = current_token->next;
				current_token = current_token->next;
				if (previous_token[0] != NULL)
					previous_token[0]->next = current_token;
				preserve_token = 3;
				token_destroy(token_remove);
				break;
			}
			case FUNCTION_CALL:
			{
				if (current_token->next == NULL)
					syntax_error(LINE_ENDED_INCORRECTLY, current_token->line);
				function_call++;
				if (current_token->next->type == PARENTHESES_OPEN)
					current_token->next->type = FUNCTION_CALL_PARAMETERS;
				preserve_token = 1;
				break;
			}
			case OPERATOR:
			{
				preserve_token = 1;
				break;
			}
			default:
				break;
			}
			switch (current_token->next->type)
			{
			case PARENTHESES_CLOSE:
			case BRACKETS_CLOSE:
			case SCOPE_CLOSE:
			{
				if (special_operation[0] != RETURN)
					syntax_error(INVALID_OPERATION, current_token->line);
				break;
			}
			default:
				break;
			}
			break;
		}
		default:
		{
			if (special_operation[0] != 0)
			{
				if (current_token->next != NULL && current_token->next->type != OPERATOR)
					special_operation[1]--;
			}
			reorganised = 0;
			break;
		}
		}
		if (preserve_token != 3 && special_operation[0] != 0 && special_operation[1] == 0)
			special_operation[2] = 1;
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
		if (preserve_token == 1)
		{
			// Storing for later insertion
			previous_token[0]->next = current_token->next;
			struct preserved_call_token *preserve_token = xmalloc(sizeof(struct preserved_call_token));
			preserve_token->token = current_token;
			preserve_token->counter = 2;
			preserve_token->parentheses_position = parentheses_position;
			preserve_token->brackets_position = brackets_position;
			preserve_token->scope_position = scope_position;
			preserve_token->previous = preserved_call_tokens;
			preserved_call_tokens = preserve_token;
			if (previous_token[0] != NULL)
				previous_token[0]->next = current_token->next;
			current_token->supporting_reference = previous_token[0];
			current_token = current_token->next;
			continue;
		}
		else if (preserved_call_tokens != NULL && preserved_call_tokens->parentheses_position == parentheses_position && preserved_call_tokens->brackets_position == brackets_position && preserved_call_tokens->scope_position == scope_position)
		{
			// Reinserting previously stored values
			int additional_insertion = 0;
			for (; (preserved_call_tokens != NULL && preserved_call_tokens->parentheses_position == parentheses_position && preserved_call_tokens->brackets_position == brackets_position && preserved_call_tokens->scope_position == scope_position);)
			{
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
			preserve_token = 2;
		}
		else if (special_operation[2] == 1)
		{
			if (special_operation[0] == RETURN)
			{
				preserve_token = 2;
				if (current_token->next != NULL && current_token->next->type != SCOPE_CLOSE)
					syntax_error(STATEMENT_AFTER_RETURN, current_token->line);
			}
			else
				preserve_token = 1;
		}
		else if (preserve_token < 2)
			preserve_token = 1;
		else if (preserve_token == 3)
			continue;
		int special_operation_insertion = 1;
		for (; (preserve_token > 0 && current_token != NULL); preserve_token--)
		{
			previous_token[2] = previous_token[1];
			previous_token[1] = previous_token[0];
			previous_token[0] = current_token;
			if (special_operation[2] == 1 && special_operation_insertion)
			{
				if (current_token->next != NULL && special_operation[0] != RETURN)
					current_token = current_token->next;
				special_operation_insertion = 0;
				token_t *special_token = xmalloc(sizeof(token_t));
				special_token->type = special_operation[0];
				special_token->next = current_token->next;
				current_token->next = special_token;
				special_operation[0] = 0;
				special_operation[1] = 0;
				special_operation[2] = 0;
			}
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
	if (special_operation[0] != 0)
	{
		token_t *special_token = xmalloc(sizeof(token_t));
		special_token->type = special_operation[0];
		special_token->next = NULL;
		previous_token[0]->next = special_token;
	}
	if (preserved_call_tokens != NULL)
		syntax_error(SCOPE_CLOSE_UNABLE, preserved_call_tokens->token->line);
	current_token = root_token;
}

typedef struct parsed_variable_type_t
{
	int is_global;
	int numeric_reference;
	int function_numeric_reference;
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
int parse_get_variable_numeric(char *name, int is_global, int function_numeric_reference)
{
	parsed_variable_type_t *new_variable = xmalloc(sizeof(parsed_variable_type_t));
	new_variable->is_global = is_global;
	new_variable->name = xmalloc(strlen(name) + 1);
	new_variable->function_numeric_reference = function_numeric_reference;
	strcpy(new_variable->name, name);
	if (parse_variable_references == NULL)
	{
		new_variable->numeric_reference = 0;
		parse_variable_references = new_variable;
		return 0;
	}
	parsed_variable_type_t *current_variable = parse_variable_references;
	int numeric_reference = 0;
	for (; current_variable != NULL; numeric_reference++)
	{
		if (strcmp(current_variable->name, name) == 0)
		{
			if (current_variable->is_global == 1 || current_variable->function_numeric_reference == function_numeric_reference)
				return current_variable->numeric_reference;
		}
		if (current_variable->next == NULL)
		{
			new_variable->numeric_reference = numeric_reference;
			current_variable->next = new_variable;
			return numeric_reference;
		}
		current_variable = current_variable->next;
	}
	return numeric_reference;
}

// Convertion function and variable references to numeric values
void parse_numeric_reformat(parsed_function_scope_t **function_scopes)
{
	parsed_function_scope_t *root_function = *function_scopes;
	parsed_function_scope_t *current_function = root_function;
	for (int function_count = 0; current_function != NULL; function_count++)
	{
		int is_global_scope = (current_function == root_function);
		token_t *current_token = current_function->function_token;
		token_t *previous_token = NULL;
		int within_array = 0;
		struct function_call_parameter
		{
			int count;
			int array_scope;
			struct function_call_parameter *parent;
		};
		struct function_call_parameter *parameter_stack = NULL;
		int is_function_declaration = 0;
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
				int function_id = parse_get_function_scope_numeric(current_token->contents.string, parameter_stack->count);
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
				int variable_id = parse_get_variable_numeric(current_token->contents.string, is_global_scope, function_count);
				free(current_token->contents.string);
				current_token->contents.numeric = variable_id;
				break;
			}
			case FUNCTION_CALL_PARAMETERS:
			{
				struct function_call_parameter *parameters = xmalloc(sizeof(struct function_call_parameter));
				if (current_token->next != NULL && current_token->next->type != PARENTHESES_CLOSE)
					parameters->count = 1;
				parameters->array_scope = within_array;
				if (parameter_stack != NULL)
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
					is_function_declaration = 0;
				break;
			}
			case SCOPE_CLOSE:
			{
				if (current_token->next == NULL && !is_global_scope)
					removable = 1;
			}
			case PARENTHESES_CLOSE:
			{
				removable = 1;
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
