#ifndef tokenizer_h
#define tokenizer_h

#include <stdio.h>

typedef enum token_types
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
	NUMERIC,
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
	RETURN,
	BREAK,
} token_types;

typedef struct token_t
{
	token_types type;
	char *contents;
	unsigned long long int position;
	unsigned long long int line;
	struct token_t *next;
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
