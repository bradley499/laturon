#ifndef tokenizer_h
#define tokenizer_h

#include <stdio.h>

typedef enum token_types
{
	NOT_DEFINED,
	FUNCTION_DECLARATION,
	FUNCTION_CALL,
	FUNCTION_CALL_HOST,
	FUNCTION_CALL_PARAMETERS,
	VARIABLE,
	PARENTHESES_OPEN,
	PARENTHESES_CLOSE,
	BRACKETS_OPEN,
	BRACKETS_CLOSE,
	SCOPE_OPEN,
	SCOPE_CLOSE,
	OPERATOR,
	LITERAL,
	NUMERIC,
	DOUBLE,
	ASSIGN,
	EQUALITY,
	NOT_EQUALITY,
	LESS_OR_EQUALITY,
	MORE_OR_EQUALITY,
	AND,
	OR,
	INSERT,
	REMOVE,
	IF,
	ELSE,
	WHILE,
	WHILE_START,
	RETURN,
	BREAK,
	NEGATE,
	ARRAY_LOCATION,
	NO_OPERATION,
} token_types;

typedef struct token_t
{
	token_types type;
	union
	{
		char *string;
		signed long long int numeric;
		long double floating;
	} contents;
	unsigned long long int position;
	unsigned long long int line;
	struct token_t *next;
	struct token_t *supporting_reference;
} token_t;

// Reads the file provided and splits the syntax into tokens
void tokenize_file(FILE *fp);
// Destroy a token and free it's memory;
void token_destroy(token_t *token);
// Cleanup and destroy all tokens
void token_cleanup();
// Returns the token generated
token_t *token_get_head();

#endif
