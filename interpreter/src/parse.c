#ifndef parse_c
#define parse_c

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "tokenizer.h"
#include "scope.h"
#include "misc.h"

typedef struct parsed_object_type_t
{
	char *name;
	bool is_function;
	unsigned long long int numeric_reference;
	struct parsed_object_type_t *next;
} parsed_object_type_t;

parsed_object_type_t *objects_root = NULL;
parsed_object_type_t *objects_current = NULL;
scope_t *scope_tree = NULL;

scope_t *parse_create_scope(char *name, bool is_function, int scope_type);

int parse_init()
{
	scope_tree = parse_create_scope("print", true, SCOPE_TYPE_FUNCTION);
	scope_tree->left = parse_create_scope("input", true, SCOPE_TYPE_FUNCTION);
	objects_root->numeric_reference = 1;
	objects_root->next->numeric_reference = 2;
	return 1;
}

parsed_object_type_t *parse_object_type_new()
{
	parsed_object_type_t *object = xmalloc(sizeof(parsed_object_type_t));
	object->name = NULL;
	object->next = NULL;
	return object;
}

scope_t *parse_create_scope(char *name, bool is_function, int scope_type)
{
	scope_t *scope = scope_new();
	scope->type = scope_type;
	parsed_object_type_t *object = parse_object_type_new();
	strcpy(object->name, name);
	object->is_function = is_function;
	object->numeric_reference = -1;
	if (objects_root == NULL)
	{
		objects_root = object;
		objects_current = object;
	}
	else
		objects_current->next = object;
	return scope;
}

int parse_tokens()
{
	token_t *tokens = token_get_head();
	if (tokens == NULL)
		return 0;
	return 1;
}

void parse_cleanup()
{
	parsed_object_type_t *object = objects_root;
	for (;object != NULL;)
	{
		parsed_object_type_t *object_next = object->next;
		if (object->name != NULL)
			free(object->name);
		parsed_object_type_t *object_current = object;
		free(object_current);
		object = object_next;
	}
	scope_destroy(scope_tree);
}

#endif