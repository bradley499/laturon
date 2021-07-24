#ifndef fnv1a_c
#define fnv1a_c

#include "fnv1a.h"

unsigned int fnv1a(const char *str, signed int hash)
{
	if (hash == 0)
		hash = 0x811C9DC5; // Default hash seed (2166136261)
	const unsigned char *ptr = (const unsigned char *)str;
	unsigned char character;
	while ((character = *ptr++) != 0)			// Loop until finished string pointer
		hash = (character ^ hash) * 0x01000193; // Calculate hash
	return ((hash < 0) ? -hash : hash);			// Return absolute fnv1a hash value
}

#endif