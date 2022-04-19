#ifndef parse_h
#define parse_h

#include "tokenizer.h"

typedef struct parsed_function_scope_t
{
	token_t *function_token;
	struct parsed_function_scope_t *next;
} parsed_function_scope_t;

// Parses tokens and generates function structure
int parse_tokens(parsed_function_scope_t **functions);
// Clean up all function definitions
void parse_cleanup();
#endif