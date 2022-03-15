#ifndef tokenizer_h
#define tokenizer_h

#include <stdio.h>

typedef struct token_t
{
	unsigned char type;
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