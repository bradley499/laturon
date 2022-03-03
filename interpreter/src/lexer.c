#ifndef lexer_c
#define lexer_c

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "misc.h"
#include "interact.h"

#define IDENTIFIER_FUNCTION_MAX_LENGTH 8
#define IDENTIFIER_TYPE_FUNCTION "function "
#define IDENTIFIER_MAX_LENGTH 1024

enum syntax_errors
{
	NO_FUNCTION_DEFINITION,		  // No function definition or reference was given
	NO_FUNCTION_REFERENCE,		  // Unable to call a function that has no reference
	SCOPE_OPEN_WITHIN_EXPRESSION, // Attempting to open a scope within an expression
	SCOPE_CLOSE_UNABLE,			  // Closing a scope that has not been opened
	PARENTHESES_CLOSE_UNABLE,	  // Closing a parentheses that has not been opened
	BRACKET_CLOSE_UNABLE,		  // Closing a bracket that has not been opened
	INVALID_FUNCTION_DEFINITION,  // Invalid function definition
};

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

void syntax_error(enum syntax_errors error, unsigned int line);

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

int is_newline(char c)
{
	return (c == '\n');
}

int execute_file(FILE *fp)
{
	int line = 1;
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
					syntax_error(NO_FUNCTION_DEFINITION, line);
				else if (current_token->type != FUNCTION_DECLARATION && current_token->type != FUNCTION_CALL)
					syntax_error(NO_FUNCTION_REFERENCE, line);
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
		char *identifier = xmalloc(sizeof(char) * IDENTIFIER_MAX_LENGTH);
		unsigned char identifier_current_length = 0;
		char character = EOF;
		for (; (character = getc(fp)) != EOF;)
		{
			if (is_whitespace(character))
			{
				if (is_newline(character))
					line++;
				position++;
				if (identifier_current_length == 0)
					continue;
				else
				{
					if (identifier_current_length == IDENTIFIER_FUNCTION_MAX_LENGTH)
					{
						if (strncmp(identifier, IDENTIFIER_TYPE_FUNCTION, IDENTIFIER_FUNCTION_MAX_LENGTH) == 0)
						{
							token->type = FUNCTION_DECLARATION;
							position += identifier_current_length;
							for (unsigned int i = 0; i < identifier_current_length; i++)
								identifier[i] = 0;
							identifier_current_length = 0;
							continue;
						}
					}
					break;
				}
			}
			if (is_scope(character))
			{
				if (current_parentheses > 0 || current_brackets > 0)
					syntax_error(SCOPE_OPEN_WITHIN_EXPRESSION, line);
				if (is_scope_open(character))
					current_scope++;
				else if (is_scope_close(character))
				{
					current_scope--;
					if (current_scope < 0)
						syntax_error(SCOPE_CLOSE_UNABLE, line);
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
						syntax_error(PARENTHESES_CLOSE_UNABLE, line);
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
						syntax_error(BRACKET_CLOSE_UNABLE, line);
					special_state = BRACKETS_CLOSE;
				}
				break;
			}
			identifier[identifier_current_length++] = character;
			if (identifier_current_length >= IDENTIFIER_FUNCTION_MAX_LENGTH)
			{
				char *new_identifier = realloc(identifier, (identifier_current_length + IDENTIFIER_FUNCTION_MAX_LENGTH));
				if (new_identifier == NULL)
					fatal_error(MEMORY_ALLOCATION_ERROR);
				identifier = new_identifier;
			}
		}
		if (token->type == FUNCTION_DECLARATION)
		{
			if (special_state != PARENTHESES_OPEN)
				syntax_error(INVALID_FUNCTION_DEFINITION, line);
			token->contents = xmalloc(sizeof(char) * (identifier_current_length + 1));
			strncpy(token->contents, identifier, identifier_current_length);
			token->contents[identifier_current_length] = '\0';
		}
		if (tokens == NULL)
			tokens = token;
		else
			current_token->next = token;
		current_token = token;
		position += identifier_current_length;
		free(identifier);
		if (character == EOF)
			position = -1;
	}
	return 0;
}

void syntax_error(enum syntax_errors error, unsigned int line)
{
	char buffer[100];
	switch (error)
	{
	case NO_FUNCTION_DEFINITION:
		snprintf(buffer, 100, "No function definition or reference was given. On line: %d", line);
		output(buffer, OUTPUT_ERROR);
		break;
	case NO_FUNCTION_REFERENCE:
		snprintf(buffer, 100, "Unable to call a function that has no reference. On line: %d", line);
		output(buffer, OUTPUT_ERROR);
		break;
	case SCOPE_OPEN_WITHIN_EXPRESSION:
		snprintf(buffer, 100, "Attempting to open a scope within an expression. On line: %d", line);
		output(buffer, OUTPUT_ERROR);
		break;
	case SCOPE_CLOSE_UNABLE:
		snprintf(buffer, 100, "Closing a scope that has not been opened. On line: %d", line);
		output(buffer, OUTPUT_ERROR);
		break;
	case PARENTHESES_CLOSE_UNABLE:
		snprintf(buffer, 100, "Closing a parentheses that has not been opened. On line: %d", line);
		output(buffer, OUTPUT_ERROR);
		break;
	case BRACKET_CLOSE_UNABLE:
		snprintf(buffer, 100, "Closing a bracket that has not been opened. On line: %d", line);
		output(buffer, OUTPUT_ERROR);
		break;
	case INVALID_FUNCTION_DEFINITION:
		snprintf(buffer, 100, "Invalid function definition. On line: %d", line);
		output(buffer, OUTPUT_ERROR);
		break;
	default:
		snprintf(buffer, 100, "Invalid syntax. On line: %d", line);
		output("Invalid syntax. On line: ", OUTPUT_ERROR);
		break;
	}
	exit(EXIT_FAILURE);
}

#endif