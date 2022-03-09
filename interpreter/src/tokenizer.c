#ifndef tokenizer_c
#define tokenizer_c

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "misc.h"
#include "interact.h"
#include "tokenizer.h"

#define IDENTIFIER_FUNCTION_MAX_LENGTH 8
#define IDENTIFIER_TYPE_FUNCTION "function "
#define IDENTIFIER_VARIABLE_MAX_LENGTH 3
#define IDENTIFIER_TYPE_VARIABLE "var "
#define IDENTIFIER_MAX_LENGTH 1024

enum syntax_errors
{
	INVALID_SYNTAX,				   // Invalid syntax
	NO_FUNCTION_DEFINITION,		   // No function definition or reference was given
	NO_FUNCTION_REFERENCE,		   // Unable to call a function that has no reference
	NO_VARIABLE_DEFINITION,		   // No variable definition or reference was given
	SCOPE_OPEN_WITHIN_EXPRESSION,  // Attempting to open a scope within an expression
	SCOPE_CLOSE_UNABLE,			   // Closing a scope that has not been opened
	PARENTHESES_CLOSE_UNABLE,	   // Closing a parentheses that has not been opened
	BRACKET_CLOSE_UNABLE,		   // Closing a bracket that has not been opened
	INVALID_FUNCTION_DEFINITION,   // Invalid function definition
	INVALID_VARIABLE_DEFINITION,   // Invalid variable definition
	FUNCTION_DEFINITION_RECURSIVE, // Unable to define a function with a recursive function definition
	VARIABLE_DEFINITION_RECURSIVE, // Unable to define a variable with a recursive variable definition
	LINE_ENDED_INCORRECTLY,		   // The line ended abruptly
	UNUSED_VARIABLE,			   // A variable is referenced but not utilised
	INVALID_OPERATION,			   // An invalid operation was defined within the syntax
};

enum token_types
{
	NOT_DEFINED,
	FUNCTION_DECLARATION,
	FUNCTION_CALL,
	VARIABLE,
	PARENTHESES_OPEN,
	PARENTHESES_CLOSE,
	BRACKETS_OPEN,
	BRACKETS_CLOSE,
	SCOPE_OPEN,
	SCOPE_CLOSE,
	OPERATOR,
	LITERAL,
	EQUALITY,
	NOT_EQUALITY,
	LESS_OR_EQUALITY,
	MORE_OR_EQUALITY,
};

void syntax_error(enum syntax_errors error, unsigned int line);

int is_operator(char c)
{
	return (c == '+' || c == '-' || c == '*' || c == '/' || c == '=' || c == '<' || c == '>' || c == '%' || c == '!' || c == ',');
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

int is_literal(char c)
{
	if (c == '\"')
		return 1;
	else if (c == '\'')
		return 2;
	else
		return 0;
}

int is_whitespace(char c)
{
	return (c <= ' ' || c == '\n' || c == '\t' || c == '\v' || c == '\f' || c == '\r');
}

int is_newline(char c)
{
	return (c == '\n');
}

int is_numeric(char c)
{
	return (c >= '0' && c <= '9');
}

int is_comment(char c)
{
	return (c == '#');
}

int tokenize_file(FILE *fp)
{
	int line = 1;
	long long int position = 0;
	int current_parentheses = 0;
	int current_brackets = 0;
	int current_scope = 0;
	int current_literal = 0;
	int current_numeric = 0;
	int current_comment = 0;
	token_t *tokens = NULL;
	token_t *current_token = NULL;
	enum token_types special_state = NOT_DEFINED;
	char character = 0;
	for (;;)
	{
		if (special_state > 0)
		{
			if (special_state == PARENTHESES_OPEN)
			{
				if (current_token == NULL)
					syntax_error(NO_FUNCTION_DEFINITION, line);
				else if (!(current_token->type == FUNCTION_DECLARATION || current_token->type == FUNCTION_CALL))
					syntax_error(NO_FUNCTION_REFERENCE, line);
			}
			else if (special_state == BRACKETS_OPEN)
			{
				if (current_token == NULL)
					syntax_error(NO_VARIABLE_DEFINITION, line);
				else if (!((current_token->type != BRACKETS_OPEN && current_token->type != BRACKETS_CLOSE && current_token->type != PARENTHESES_CLOSE && current_token->type != VARIABLE) || (current_token->type == OPERATOR && (current_token->contents == (char *)'=' || current_token->contents == (char *)','))))
					syntax_error(NO_VARIABLE_DEFINITION, line);
			}
			token_t *token = xmalloc(sizeof(token_t));
			token->contents = NULL;
			token->type = special_state;
			token->position = position;
			special_state = NOT_DEFINED;
			current_token->next = token;
			current_token = token;
		}
		if (character == EOF)
			break;
		current_comment = 0;
		token_t *token = xmalloc(sizeof(token_t));
		token->contents = NULL;
		token->type = NOT_DEFINED;
		char *identifier = xmalloc(sizeof(char) * IDENTIFIER_MAX_LENGTH);
		unsigned char identifier_current_length = 0;
		unsigned long long current_identifier_max_length = IDENTIFIER_MAX_LENGTH;
		for (; (character = getc(fp)) != EOF;)
		{
			if (is_whitespace(character))
			{
				if (is_newline(character))
				{
					if (current_literal != 0 || current_parentheses != 0 || current_brackets != 0)
						syntax_error(LINE_ENDED_INCORRECTLY, line);
					line++;
				}
				if (current_literal == 0)
				{
					position++;
					if (identifier_current_length == 0)
						continue;
					else if (identifier_current_length == IDENTIFIER_FUNCTION_MAX_LENGTH)
					{
						if (strncmp(identifier, IDENTIFIER_TYPE_FUNCTION, IDENTIFIER_FUNCTION_MAX_LENGTH) == 0)
						{
							if (token->type == FUNCTION_DECLARATION)
								syntax_error(FUNCTION_DEFINITION_RECURSIVE, line);
							token->type = FUNCTION_DECLARATION;
							token->position = (position - identifier_current_length);
							position += identifier_current_length;
							for (unsigned int i = 0; i < identifier_current_length; i++)
								identifier[i] = 0;
							identifier_current_length = 0;
							continue;
						}
						break;
					}
				}
			}
			if (is_scope(character) && current_literal == 0)
			{
				if (current_parentheses > 0 || current_brackets > 0)
					syntax_error(SCOPE_OPEN_WITHIN_EXPRESSION, line);
				if (is_scope_open(character))
				{
					special_state = SCOPE_OPEN;
					current_scope++;
				}
				else if (is_scope_close(character))
				{
					special_state = SCOPE_CLOSE;
					current_scope--;
					if (current_scope < 0)
						syntax_error(SCOPE_CLOSE_UNABLE, line);
				}
				if (current_parentheses > 0 && current_brackets > 0)
				{
					if (special_state == SCOPE_OPEN)
						syntax_error(SCOPE_OPEN_WITHIN_EXPRESSION, line);
					else
						syntax_error(SCOPE_CLOSE_UNABLE, line);
				}
				break;
			}
			else if ((is_parentheses(character) || is_bracket(character)) && current_literal == 0)
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
			else if (is_operator(character) && current_literal == 0)
			{
				if (identifier_current_length > 0)
				{
					int identifier_position = 0;
					int line_up[2] = {0, 0};
					int variable_unused = 0;
					for (; identifier[identifier_position] != '\0';)
					{
						if (identifier[identifier_position] == '\n')
							line_up[0]++;
						if (variable_unused == 0)
						{
							if (is_whitespace(identifier[identifier_position]) && (identifier_position < (identifier_current_length - 1)))
							{
								variable_unused = 1;
								line_up[1] = line_up[0];
							}
						}
						identifier_position++;
					}
					if (variable_unused == 1)
						syntax_error(UNUSED_VARIABLE, (line - line_up[0] + line_up[1]));
					if (line_up[0] > 0)
						syntax_error(LINE_ENDED_INCORRECTLY, (line - line_up[0]));
					token_t *new_token = xmalloc(sizeof(token_t));
					new_token->contents = NULL;
					new_token->contents = xmalloc(sizeof(char) * (identifier_current_length + 1));
					strncpy(new_token->contents, identifier, identifier_current_length);
					new_token->contents[identifier_current_length] = '\0';
					new_token->type = VARIABLE;
					new_token->position = (position + identifier_current_length - 1);
					current_token->next = new_token;
					current_token = new_token;
				}
				break;
			}
			else
			{
				int literal = is_literal(character);
				if (literal != 0)
				{
					if (current_literal == 0)
					{
						token->type = LITERAL;
						token->position = position;
						current_literal = literal;
						continue;
					}
					else if (current_literal == literal)
					{
						int escaped = 0;
						if (identifier_current_length > 0)
						{
							if (identifier[(identifier_current_length - 1)] == '\\')
								escaped = 1;
						}
						current_literal = 0;
						if (!escaped)
						{
							current_literal = -1;
							break;
						}
					}
				}
				else if (is_numeric(character))
					current_numeric = 1;
				else if (is_comment(character))
				{
					current_comment = 1;
					for (; (character = getc(fp)) != EOF && !is_newline(character);)
						position++;
					if (is_newline(character))
						line++;
					break;
				}
				else if (!is_numeric(character) && (current_numeric != 0) && character != '.')
				{
					current_numeric = -1;
					current_literal = -1;
					break;
				}
			}
			identifier[identifier_current_length++] = character;
			if (identifier_current_length >= current_identifier_max_length)
			{
				current_identifier_max_length += IDENTIFIER_MAX_LENGTH;
				char *new_identifier = realloc(identifier, current_identifier_max_length);
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
		else
		{
			if (token->type == VARIABLE)
			{
				if (special_state != NOT_DEFINED)
					syntax_error(INVALID_VARIABLE_DEFINITION, line);
				token->contents = xmalloc(sizeof(char) * (identifier_current_length + 1));
				strncpy(token->contents, identifier, identifier_current_length);
				token->contents[identifier_current_length] = '\0';
			}
			if (token->type == NOT_DEFINED || token->type == VARIABLE)
			{
				if (is_operator(character))
				{
					if (current_token == NULL && token->type != VARIABLE)
						syntax_error(INVALID_SYNTAX, line);
					if (token->type != VARIABLE && current_token->type == OPERATOR)
					{
						if (character != '=')
							syntax_error(INVALID_OPERATION, line);
						if (current_token->position != position)
							syntax_error(INVALID_OPERATION, line);
						if (current_token->contents == (char *)'=')
							current_token->type = EQUALITY;
						else if (current_token->contents == (char *)'!')
							current_token->type = NOT_EQUALITY;
						else if (current_token->contents == (char *)'>')
							current_token->type = MORE_OR_EQUALITY;
						else if (current_token->contents == (char *)'<')
							current_token->type = LESS_OR_EQUALITY;
						else
							syntax_error(INVALID_OPERATION, line);
						current_token->contents = NULL;
						token->contents = NULL;
					}
					else
					{
						token->type = OPERATOR;
						*token->contents = character;
						token->position = position;
					}
				}
				else if (identifier_current_length > 0)
				{
					identifier[identifier_current_length] = '\0';
					token->contents = xmalloc(sizeof(char) * (identifier_current_length + 1));
					strncpy(token->contents, identifier, identifier_current_length);
					token->contents[identifier_current_length] = '\0';
					token->type = (special_state == PARENTHESES_OPEN ? FUNCTION_CALL : VARIABLE);
					token->position = position;
				}
				else if (special_state == 0 && character != EOF && !is_scope(character) && current_comment == 0)
					syntax_error(INVALID_SYNTAX, line);
				else if (special_state != 0)
				{
					position += identifier_current_length;
					continue;
				}
			}
			else if (current_literal == -1)
			{
				if (identifier_current_length > 0)
				{
					identifier[identifier_current_length] = '\0';
					token->contents = xmalloc(sizeof(char) * (identifier_current_length + 1));
					strncpy(token->contents, identifier, identifier_current_length);
					token->contents[identifier_current_length] = '\0';
				}
				else
					token->contents = NULL;
				token->position = position;
				token->type = LITERAL;
			}
			else
				syntax_error(INVALID_SYNTAX, line);
		}
		if (token->type == VARIABLE && current_literal == -1 && current_numeric == -1)
			token->type = LITERAL;
		if (current_literal == -1)
			current_literal = 0;
		if (tokens == NULL)
			tokens = token;
		else
			current_token->next = token;
		current_token = token;
		position += identifier_current_length;
		free(identifier);
	}
	if (current_parentheses != 0 || current_brackets != 0 || current_scope != 0 || current_literal > 0)
		syntax_error(INVALID_SYNTAX, line);
	token_optimisation(tokens);
	return 0;
}

int token_optimisation(token_t *tokens)
{
	token_t *current_token = tokens;
	token_t *previous_token = NULL;
	for (; current_token != NULL;)
	{
		if (current_token->type == 0)
		{
			if (previous_token == NULL)
				tokens = current_token->next;
			else
				previous_token->next = current_token->next;
			if (current_token->contents != NULL)
				free(current_token->contents);
			free(current_token);
		}
		previous_token = current_token;
		current_token = current_token->next;
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
		break;
	case NO_FUNCTION_REFERENCE:
		snprintf(buffer, 100, "Unable to call a function that has no reference. On line: %d", line);
		break;
	case NO_VARIABLE_DEFINITION:
		snprintf(buffer, 100, "No variable definition or reference was given. On line: %d", line);
		break;
	case SCOPE_OPEN_WITHIN_EXPRESSION:
		snprintf(buffer, 100, "Attempting to open a scope within an expression. On line: %d", line);
		break;
	case SCOPE_CLOSE_UNABLE:
		snprintf(buffer, 100, "Closing a scope that has not been opened. On line: %d", line);
		break;
	case PARENTHESES_CLOSE_UNABLE:
		snprintf(buffer, 100, "Closing a parentheses that has not been opened. On line: %d", line);
		break;
	case BRACKET_CLOSE_UNABLE:
		snprintf(buffer, 100, "Closing a bracket that has not been opened. On line: %d", line);
		break;
	case INVALID_FUNCTION_DEFINITION:
		snprintf(buffer, 100, "Invalid function definition. On line: %d", line);
		break;
	case INVALID_VARIABLE_DEFINITION:
		snprintf(buffer, 100, "Invalid variable definition. On line: %d", line);
		break;
	case FUNCTION_DEFINITION_RECURSIVE:
		snprintf(buffer, 100, "Unable to define a function with a recursive function definition. On line: %d", line);
		break;
	case VARIABLE_DEFINITION_RECURSIVE:
		snprintf(buffer, 100, "Unable to define a variable with a recursive variable definition. On line: %d", line);
		break;
	case INVALID_OPERATION:
		snprintf(buffer, 100, "An invalid operation was defined within the syntax. On line: %d", line);
		break;
	case UNUSED_VARIABLE:
		snprintf(buffer, 100, "A variable is referenced but not utilised. On line: %d", line);
		break;
	case LINE_ENDED_INCORRECTLY:
		snprintf(buffer, 100, "The line ended abruptly. On line: %d", line);
		break;
	case INVALID_SYNTAX:
	default:
		snprintf(buffer, 100, "Invalid syntax. On line: %d", line);
		break;
	}
	output(buffer, OUTPUT_ERROR);
	exit(EXIT_FAILURE);
}

#endif