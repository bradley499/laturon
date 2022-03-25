#ifndef parse_c
#define parse_c

#include <stdlib.h>
#include <string.h>
#include "parse.h"
#include "tokenizer.h"
#include "scope.h"
#include "misc.h"

typedef struct parsed_object_type_t
{
	char *name;
	int is_function;
	int scope_type;
	unsigned long long int numeric_reference;
	scope_t *referenced_scope;
	struct parsed_object_type_t *next;
} parsed_object_type_t;

typedef struct parsed_parent_relative_position
{
	int position;
	struct parsed_parent_relative_position *parent;
} parsed_parent_relative_position;

parsed_object_type_t *objects_root = NULL;
parsed_object_type_t *objects_current = NULL;
scope_t *scope_tree = NULL;
scope_t *scope_current = NULL;
parsed_parent_relative_position *relative_position_root = NULL;

scope_t *parse_create_scope(char *name, int is_function, int scope_type, int line);
int parse_scope_tree_append(scope_t *scope, int is_function, int parent_traversal);
void parsed_parent_relative_position_update(unsigned int new, signed int relative_change);

int parse_init()
{
	parse_cleanup(1);
	scope_tree = parse_create_scope(NULL, BOOLEAN_TRUE, SCOPE_TYPE_FUNCTION, 0);
	scope_tree->left = parse_create_scope("print", BOOLEAN_TRUE, SCOPE_TYPE_FUNCTION, 0);
	scope_tree->left->left = parse_create_scope("input", BOOLEAN_TRUE, SCOPE_TYPE_FUNCTION, 0);
	scope_current = NULL;
	return 1;
}

parsed_object_type_t *parse_object_type_new()
{
	parsed_object_type_t *object = xmalloc(sizeof(parsed_object_type_t));
	object->name = NULL;
	object->next = NULL;
	return object;
}

scope_t *parse_create_scope(char *name, int is_function, int scope_type, int line)
{
	scope_t *scope = scope_new();
	scope->type = scope_type;
	parsed_object_type_t *object = parse_object_type_new();
	if (name != NULL)
		strcpy(object->name, name);
	object->is_function = is_function;
	object->numeric_reference = -1;
	object->scope_type = scope_type;
	object->referenced_scope = scope;
	if (is_function && scope_type == SCOPE_TYPE_FUNCTION)
	{
		parsed_object_type_t *current_object = objects_root;
		for (; current_object != NULL;)
		{
			if (current_object->is_function && current_object->scope_type == SCOPE_TYPE_FUNCTION && current_object->name != NULL && strcmp(current_object->name, object->name) == 0)
				syntax_error(DUPLICATE_FUNCTION_DEFINITION, line);
			current_object = current_object->next;
		}
	}
	if (objects_root == NULL)
	{
		objects_root = object;
		objects_current = object;
	}
	else
		objects_current->next = object;
	return scope;
}

int parse_scope_tree_append(scope_t *scope, int is_function, int parent_traversal)
{
}

void parsed_parent_relative_position_update(unsigned int new, signed int relative_change)
{
	parsed_parent_relative_position *current_relative_position = relative_position_root;
	if (!new)
	{
		if (current_relative_position != NULL)
			relative_change = -(current_relative_position->position);
	}
	for (; current_relative_position->parent != NULL;)
	{
		current_relative_position->position += relative_change;
		current_relative_position = current_relative_position->parent;
	}
	if (new)
	{
		parsed_parent_relative_position *new_relative_position = xmalloc(sizeof(parsed_parent_relative_position));
		new_relative_position->position = relative_change;
		new_relative_position->parent = relative_position_root;
		relative_position_root = new_relative_position;
	}
	else if (relative_position_root != NULL)
	{
		current_relative_position = relative_position_root;
		relative_position_root = relative_position_root->parent;
		free(current_relative_position);
		// Write a function to traverse a cursor/current/pointer up each parent...
	}
}

void parse_reformat_tokens(token_t **tokens);

int parse_tokens()
{
	token_t *tokens = token_get_head();
	if (tokens == NULL)
		return 0;
	parse_reformat_tokens(&tokens);
	token_t *current_token = tokens;
	token_t *previous_token = NULL;
	int function_scope_parameters = 0;
	int function_scope_parameters_declaration = 0;
	int function_scope_unable = 0;
	for (; current_token != NULL;)
	{
		switch (current_token->type)
		{
		case NOT_DEFINED:
			fatal_error(UNKNOWN_ERROR);
			break;
		case FUNCTION_DECLARATION:
		{
			if (function_scope_unable == 0)
			{
				parse_scope_tree_append(parse_create_scope(current_token->contents, 1, SCOPE_TYPE_FUNCTION, current_token->line), 1, 0);
				function_scope_unable++;
			}
			else
				syntax_error(FUNCTION_DEFINITION_RECURSIVE, current_token->line);
			break;
		}
		case FUNCTION_CALL:
		{
			parse_scope_tree_append(parse_create_scope(current_token->contents, 0, SCOPE_TYPE_FUNCTION_CALL, current_token->line), 0, 0);
			break;
		}
		case VARIABLE:
		{
			if (current_token->next != NULL && current_token->next->type == OPERATOR && current_token->contents == '=')
			{
				parse_scope_tree_append(parse_create_scope(current_token->contents, 0, SCOPE_VARIABLE_UPDATE, current_token->line), 0, 0);
				break;
			}
			parse_scope_tree_append(parse_create_scope(current_token->contents, 0, SCOPE_VARIABLE, current_token->line), 0, 0);
			break;
		}
		case PARENTHESES_OPEN:
		{
			if (previous_token != NULL && (previous_token->type == FUNCTION_DECLARATION || previous_token->type == FUNCTION_CALL))
			{
				if (previous_token->type == FUNCTION_DECLARATION)
				{
					if (function_scope_parameters_declaration != 0)
						syntax_error(SCOPE_OPEN_WITHIN_EXPRESSION, current_token->line);
					else
						function_scope_parameters_declaration = 1;
				}
				parse_scope_tree_append(parse_create_scope(current_token->contents, 0, SCOPE_TYPE_PARAMETER_START, current_token->line), 0, 0);
				function_scope_parameters++;
			}
			else
				parse_scope_tree_append(parse_create_scope(current_token->contents, 0, SCOPE_TYPE_CONTAINER, current_token->line), 0, 1);
			break;
		}
		case PARENTHESES_CLOSE:
		{
			if (previous_token != NULL)
			{
				if (function_scope_parameters_declaration != 0)
					parsed_parent_relative_position_update(0, 0);
				function_scope_parameters_declaration = 0;
			}
			break;
		}
		case BRACKETS_OPEN:
		{
			if (previous_token != NULL && (previous_token->type == VARIABLE || previous_token->type == BRACKETS_CLOSE || previous_token->type == PARENTHESES_OPEN || previous_token->type == PARENTHESES_CLOSE))
				parse_scope_tree_append(parse_create_scope(current_token->contents, 0, SCOPE_TYPE_ARRAY, current_token->line), 0, 1);
			else
				syntax_error(INVALID_VARIABLE_REFERENCE, current_token->line);
			break;
		}
		case BRACKETS_CLOSE:
		{
			break;
		}
		case SCOPE_OPEN:
		{
			parse_scope_tree_append(parse_create_scope(current_token->contents, 0, SCOPE_TYPE_CONTAINER, current_token->line), 0, 0);
			break;
		}
		case SCOPE_CLOSE:
		{
			break;
		}
		case OPERATOR:
		{
			break;
		}
		case LITERAL:
		{
			break;
		}
		case NUMERIC:
		{
			break;
		}
		case EQUALITY:
		{
			break;
		}
		case NOT_EQUALITY:
		{
			break;
		}
		case LESS_OR_EQUALITY:
		{
			break;
		}
		case MORE_OR_EQUALITY:
		{
			break;
		}
		case AND:
		{
			break;
		}
		case OR:
		{
			break;
		}
		case INSERT:
		{
			break;
		}
		case REMOVE:
		{
			break;
		}
		case IF:
		{
			break;
		}
		case ELSE:
		{
			break;
		}
		case WHILE:
		{
			break;
		}
		case RETURN:
		{
			break;
		}
		case BREAK:
		{
			break;
		}
		default:
			break;
		}
		previous_token = current_token;
		current_token = current_token->next;
	}
	return 1;
}

void parse_reformat_tokens(token_t **tokens)
{
	token_t *current_token = *tokens;
	token_t *previous_token[3] = {NULL, NULL, NULL};
	int operator_count = 0;
	int operator_signing = 0;
	for (; current_token != NULL;)
	{
		if (current_token->type == OPERATOR)
		{
			operator_count++;
			if (current_token->contents == (char *)'&' || current_token->contents == (char *)'|')
				syntax_error(INVALID_OPERATION, current_token->line);
			if (operator_count != 1)
			{
				if (current_token->contents == (char *)'-')
				{
					operator_signing++;
					if (operator_signing % 2 == 0)
					{
						if (previous_token[1] == NULL)
							syntax_error(INVALID_OPERATION, current_token->line);
						previous_token[1]->next = current_token->next;
						current_token->contents = NULL;
						previous_token[0]->contents = NULL;
						free(previous_token[0]);
						free(current_token);
						previous_token[1] = previous_token[2];
					}
				}
				else if (current_token->contents == (char *)'+')
				{
					// Remove unnecessary positivity statement
					token_t *next_token = current_token->next;
					current_token->contents = NULL;
					token_t *remove_token = current_token;
					current_token = next_token;
					free(remove_token);
					previous_token[0]->next = next_token;
					continue;
				}
			}
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
	if (*tokens == NULL)
		syntax_error(INVALID_SYNTAX, 1);
	current_token = *tokens;
	previous_token[0] = NULL;
	previous_token[1] = NULL;
	previous_token[2] = NULL;
	struct function_call_token
	{
		int parentheses_position;
		token_t *token;
		struct function_call_token *previous;
	} function_call_token;
	struct function_call_token *function_call_tokens = NULL;
	int paratheses_position = 0;
	for (; current_token != NULL;)
	{
		switch (current_token->type)
		{
		case FUNCTION_CALL:
		{
			// Store function call for later insertion
			struct function_call_token *function_token = xmalloc(sizeof(struct function_call_token));
			function_token->token = current_token;
			function_token->parentheses_position = paratheses_position;
			function_token->previous = function_call_tokens;
			function_call_tokens = function_token;
			previous_token[0]->next = current_token->next;
			current_token = function_token->token;
			paratheses_position++;
			break;
		}
		case PARENTHESES_CLOSE:
		{
			if (paratheses_position == 0)
				break;
			paratheses_position--;
			if (function_call_tokens == NULL)
				break;
			if (function_call_tokens->parentheses_position == paratheses_position)
			{
				// Can insert function call previously stored
				(function_call_tokens->token)->next = current_token->next;
				current_token->next = function_call_tokens->token;
				struct function_call_token *function_token = function_call_tokens;
				function_call_tokens = function_call_tokens->previous;
				free(function_token);
				previous_token[2] = previous_token[1];
				previous_token[1] = previous_token[0];
				previous_token[0] = current_token;
				current_token = current_token->next;
			}
			break;
		}
		case EQUALITY:
		case NOT_EQUALITY:
		case LESS_OR_EQUALITY:
		case MORE_OR_EQUALITY:
		case AND:
		case OR:
		{
			if (previous_token[0] == NULL)
				syntax_error(INVALID_SYNTAX, current_token->line);
			switch (previous_token[0]->type)
			{
			case FUNCTION_CALL:
			case LITERAL:
			case NUMERIC:
				break;
			default:
				syntax_error(INVALID_SYNTAX, current_token->line);
				break;
			}
			previous_token[1]->next = current_token;
			previous_token[0]->next = current_token->next;
			current_token->next = previous_token[0];
			break;
		}
		default:
			break;
		}
		previous_token[2] = previous_token[1];
		previous_token[1] = previous_token[0];
		previous_token[0] = current_token;
		current_token = current_token->next;
	}
	if (function_call_tokens != NULL)
		syntax_error(SCOPE_CLOSE_UNABLE, function_call_tokens->token->line);
}

void parse_cleanup(int destroy_scopes)
{
	parsed_object_type_t *object = objects_root;
	for (; object != NULL;)
	{
		parsed_object_type_t *object_next = object->next;
		if (object->name != NULL)
			free(object->name);
		parsed_object_type_t *object_current = object;
		free(object_current);
		object = object_next;
	}
	if (destroy_scopes)
		scope_destroy(scope_tree);
}

#endif
