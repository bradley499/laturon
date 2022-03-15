#ifndef parse_c
#define parse_c

#include "tokenizer.h"

int parse_tokens()
{
	token_t * tokens = token_get_head();
	if (tokens == NULL)
		return 0;
	return 1;
}

#endif