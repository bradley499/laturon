#ifndef tokenizer_h
#define tokenizer_h

#include <stdio.h>

typedef struct token_t
{
	unsigned char type;
	char *contents;
	unsigned long long int position;
	struct token_t *next;
} token_t;

// Reads the file provided and splits the syntax into tokens
int tokenize_file(FILE *fp);
// Restructures tokens
int token_optimisation(token_t *tokens);
// Destroy a token and free it's memory;
void token_destroy(token_t *token);
#endif