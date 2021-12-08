#ifndef scope_h
#define scope_h

#include "stack.h"

#define SCOPE_SUCCESS               0   // Successful operation
#define SCOPE_FAILURE               1   // Failed operation
#define SCOPE_CREATION_FAILURE      2   // Failed to allocate memory for new scope
#define SCOPE_UNDEFINED             3   // Scope has not been configured
#define SCOPE_AWAIT_READY           4   // Scope is await has not been declared
#define SCOPE_AWAITING              5   // Scope is awaiting primary completion
#define SCOPE_AWAIT_FINISHED        6   // Scope operations have finished
#define SCOPE_AWAITING_CONCLUSION   7   // Scope is awaiting secondary completion
#define SCOPE_AWAIT_NO              8   // Scope does not require completion awaits
#define SCOPE_AWAITING_DELETION     9   // Scope declaration for destruction
#define SCOPE_TYPE_CONTAINER        10  // Scope contains other general scopes
#define SCOPE_TYPE_IF_CONTAINER     11  // Scope is for an if container declaration
#define SCOPE_TYPE_IF_ELSE          12  // Scope is for an if, else declaration
#define SCOPE_TYPE_INT              13  // Scope is integer declaration
#define SCOPE_TYPE_BOOL             14  // Scope is boolean declaration
#define SCOPE_TYPE_DOUBLE           15  // Scope is double declaration
#define SCOPE_TYPE_STRING           16  // Scope is string declaration
#define SCOPE_TYPE_FUNCTION         17  // Scope is a function declaration
#define SCOPE_TYPE_FUNCTION_CALL    18  // Scope is a function call
#define SCOPE_TYPE_PARAMETER_START  19  // Scope is a declarting the start of a parameter chain
#define SCOPE_TYPE_PARAMETER_END    20  // Scope is a declarting the end of a parameter chain
#define SCOPE_TYPE_ARRAY            21  // Scope is a array declaration
#define SCOPE_TYPE_ARRAY_AT         22  // Scope is a array value at position declaration
#define SCOPE_VARIABLE              23  // Scope a variable
#define SCOPE_VARIABLE_UPDATE       24  // Scope is updating a variable
#define SCOPE_LOGIC_EQUAL           25  // Scope logic: equal
#define SCOPE_LOGIC_NOT             26  // Scope logic: not
#define SCOPE_LOGIC_MORE            27  // Scope logic: more
#define SCOPE_LOGIC_MORE_OR_EQUAL   28  // Scope logic: more or equal
#define SCOPE_LOGIC_LESS            29  // Scope logic: less
#define SCOPE_LOGIC_LESS_OR_EQUAL   30  // Scope logic: less or equal
#define SCOPE_LOGIC_AND             31  // Scope logic: and
#define SCOPE_MATH_ADD              32  // Scope math: add
#define SCOPE_MATH_SUBTRACT         33  // Scope math: substract
#define SCOPE_MATH_MULTIPLY         34  // Scope math: multiply
#define SCOPE_MATH_DIVIDE           35  // Scope math: divide
#define SCOPE_MATH_MODULO           36  // Scope math: divide
#define SCOPE_BOOLEAN_TRUE          1.0 // Scope boolean true
#define SCOPE_BOOLEAN_FALSE         0.0 // Scope boolean false
#define SCOPE_DEFAULT_VALUE         SCOPE_BOOLEAN_FALSE

struct scope_t
{
	struct scope_t *left;  // Left branch
	struct scope_t *right; // Right branch
	unsigned char type;    // Branch type
	union result
	{
		double numeric;
		char *string;
	} result;                   // Branch result
	struct stack_t *call_stack; // Call stack parental references
	unsigned char result_type;  // Branch result type
	unsigned char await;        // Branch is awaiting result
} __attribute__((__packed__));

// Create a new scope
struct scope_t *scope_new();
// Set type of scope
int scope_set_type(struct scope_t *scope, int set_type);
// Get type of scope
int scope_get_type(struct scope_t *scope);
// Set scope leftward child scope
void scope_set_left(struct scope_t *scope, struct scope_t *set_scope);
// Get scope leftward child scope
struct scope_t *scope_get_left(struct scope_t *scope);
// Set scope rightward child scope
void scope_set_right(struct scope_t *scope, struct scope_t *set_scope);
// Get scope rightward child scope
struct scope_t *scope_get_right(struct scope_t *scope);
// Free up scope memory
void scope_destroy(struct scope_t *scope);
// Set scope result value
void scope_set_result(struct scope_t *scope, double set_result);
// Set scope result string value
unsigned char scope_set_result_string(struct scope_t *scope, char *str);
// Set scope result type
void scope_set_result_type(struct scope_t *scope, int set_result_type);
// Get scope result value
double scope_get_result(struct scope_t *scope);
// Get scope result string value
char *scope_get_result_string(struct scope_t *scope);
// Get scope result type
int scope_get_result_type(struct scope_t *scope);
// Clear scope result value
void scope_clear_result(struct scope_t *scope);
// Get scope's awaited status
int scope_await(struct scope_t *scope);
// Append scope to array
int scope_array_append(struct scope_t *scope_destination, struct scope_t *scope_source);

#endif