#ifndef lexer_c
#define lexer_c

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "misc.h"

#define IDENTIFIER_FUNCTION_MAX_LENGTH 8
#define IDENTIFIER_FUNCTION_MAX_LENGTH_OVERFLOW (IDENTIFIER_FUNCTION_MAX_LENGTH + 1)
#define IDENTIFIER_TYPE_FUNCTION "function "
#define IDENTIFIER_MAX_LENGTH 1024

enum token_types
{
	NOT_DEFINED,
	FUNCTION_DECLARATION,
	FUNCTION_CALL,
	PARENTHESES_OPEN,
	PARENTHESES_CLOSE,
	BRACKETS_OPEN,
	BRACKETS_CLOSE,
	SCOPE_OPEN,
	SCOPE_CLOSE,
	OPERATOR,
};

typedef struct token_t
{
	unsigned char type;
	char *contents;
	struct token_t *next;
} token_t;

int is_operator(char c)
{
	return (c == '+' || c == '-' || c == '*' || c == '/' || c == '=' || c == '<' || c == '>');
}

int is_scope_open(char c)
{
	return (c == '{');
}

int is_scope_close(char c)
{
	return (c == '}');
}

int is_scope(char c)
{
	return (is_scope_open(c) || is_scope_close(c));
}

int is_parentheses_open(char c)
{
	return (c == '(');
}

int is_parentheses_close(char c)
{
	return (c == ')');
}

int is_parentheses(char c)
{
	return (is_parentheses_open(c) || is_parentheses_close(c));
}

int is_bracket_open(char c)
{
	return (c == '[');
}

int is_bracket_close(char c)
{
	return (c == ']');
}

int is_bracket(char c)
{
	return (is_bracket_open(c) || is_bracket_close(c));
}

int is_whitespace(char c)
{
	return (c == ' ' || c == '\n' || c == '\t' || c == '\v' || c == '\f' || c == '\r');
}

int execute_file(FILE *fp)
{
	int position = 0;
	int current_parentheses = 0;
	int current_brackets = 0;
	int current_scope = 0;
	token_t *tokens = NULL;
	token_t *current_token = NULL;
	enum token_types special_state = NOT_DEFINED;
	for (; position != -1;)
	{
		if (special_state > 0)
		{
			if (special_state == PARENTHESES_OPEN)
			{
				if (current_token == NULL)
					fatal_error(INVALID_SYNTAX); // No function definition or reference was given
				else if (current_token->type != FUNCTION_DECLARATION && current_token->type != FUNCTION_CALL)
					fatal_error(INVALID_SYNTAX); // Unable to call a function that has no reference
			}
			token_t *token = xmalloc(sizeof(token_t));
			token->contents = NULL;
			token->type = special_state;
			special_state = NOT_DEFINED;
			current_token->next = token;
			current_token = token;
		}
		token_t *token = xmalloc(sizeof(token_t));
		token->contents = NULL;
		token->type = NOT_DEFINED;
		char* identifier = xmalloc(sizeof(char) * IDENTIFIER_MAX_LENGTH);
		unsigned char identifier_current_length = 0;
		char character = EOF;
		for (; (character = getc(fp)) != EOF;)
		{
			if (is_whitespace(character))
			{
				position++;
				if (identifier_current_length == 0)
					continue;
				else
					break;
			}
			if (is_scope(character))
			{
				if (current_parentheses > 0 || current_brackets > 0)
					fatal_error(INVALID_SYNTAX); // Attempting to open a scope within an expression
				if (is_scope_open(character))
					current_scope++;
				else if (is_scope_close(character))
				{
					current_scope--;
					if (current_scope < 0)
						fatal_error(INVALID_SYNTAX); // Closing a scope that has not been opened
				}
				break;
			}
			if (is_parentheses(character) || is_bracket(character))
			{
				if (is_parentheses_open(character))
				{
					current_parentheses++;
					special_state = PARENTHESES_OPEN;
				}
				else if (is_parentheses_close(character))
				{
					current_parentheses--;
					if (current_parentheses < 0)
						fatal_error(INVALID_SYNTAX); // Closing a parentheses that has not been opened
					special_state = PARENTHESES_CLOSE;
				}
				else if (is_bracket_open(character))
				{
					current_brackets++;
					special_state = BRACKETS_OPEN;
				}
				else if (is_bracket_close(character))
				{
					current_brackets--;
					if (current_brackets < 0)
						fatal_error(INVALID_SYNTAX); // Closing a bracket that has not been opened
					special_state = BRACKETS_CLOSE;
				}
				break;
			}
			if (identifier_current_length == IDENTIFIER_FUNCTION_MAX_LENGTH_OVERFLOW)
			{
				if (strncmp(identifier, IDENTIFIER_TYPE_FUNCTION, IDENTIFIER_FUNCTION_MAX_LENGTH_OVERFLOW) == 0)
				{
					token->type = FUNCTION_DECLARATION;
					position += identifier_current_length;
					for (unsigned int i = 0; i < identifier_current_length; i++)
						identifier[i] = 0;
					identifier_current_length = 0;
					continue;
				}
			}
			identifier[identifier_current_length++] = character;
			if (identifier_current_length >= IDENTIFIER_FUNCTION_MAX_LENGTH)
			{
				char* new_identifier = realloc(identifier, (identifier_current_length + IDENTIFIER_FUNCTION_MAX_LENGTH));
				if (new_identifier == NULL)
					fatal_error(MEMORY_ALLOCATION_ERROR);
				identifier = new_identifier;
			}
		}
		if (token->type == FUNCTION_DECLARATION)
		{
			if (special_state != BRACKETS_OPEN)
				fatal_error(INVALID_SYNTAX); // Invalid function definition
			strncpy(token->contents, identifier, identifier_current_length);
		}
		if (tokens == NULL)
			tokens = token;
		else
			current_token->next = token;
		current_token = token;
		position += identifier_current_length;
		free(identifier);
	}
	return 0;
}

#endif