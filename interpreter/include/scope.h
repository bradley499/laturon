#ifndef scope_h
#define scope_h

#include "stack.h"

#define SCOPE_SUCCESS 0				// Successful operation
#define SCOPE_FAILURE 1				// Failed operation
#define SCOPE_CREATION_FAILURE 2	// Failed to allocate memory for new scope
#define SCOPE_UNDEFINED 3			// Scope has not been configured
#define SCOPE_AWAIT_READY 4			// Scope is await has not been declared
#define SCOPE_AWAITING 5			// Scope is awaiting primary completion
#define SCOPE_AWAIT_FINISHED 6		// Scope operations have finished
#define SCOPE_AWAITING_CONCLUSION 7 // Scope is awaiting secondary completion
#define SCOPE_AWAIT_NO 8			// Scope does not require completion awaits
#define SCOPE_AWAITING_DELETION 9	// Scope declaration for destruction
typedef enum scope_types
{
	SCOPE_TYPE_CONTAINER = (SCOPE_AWAITING_DELETION + 1), // Scope contains other general scopes
	SCOPE_TYPE_IF_CONTAINER,							  // Scope is for an if container declaration
	SCOPE_TYPE_IF_ELSE,									  // Scope is for an if, else declaration
	SCOPE_TYPE_INT,										  // Scope is integer declaration
	SCOPE_TYPE_BOOL,									  // Scope is boolean declaration
	SCOPE_TYPE_DOUBLE,									  // Scope is double declaration
	SCOPE_TYPE_STRING,									  // Scope is string declaration
	SCOPE_TYPE_STRING_CONTINUED,						  // Scope is string declaration
	SCOPE_TYPE_FUNCTION,								  // Scope is a function declaration
	SCOPE_TYPE_FUNCTION_CALL,							  // Scope is a function call
	SCOPE_TYPE_PARAMETER_START,							  // Scope is a declarting the start of a parameter chain
	SCOPE_TYPE_PARAMETER_END,							  // Scope is a declarting the end of a parameter chain
	SCOPE_TYPE_ARRAY,									  // Scope is a array declaration
	SCOPE_TYPE_ARRAY_AT,								  // Scope is a array value at position declaration
	SCOPE_TYPE_CONSTANT_INT,							  // Scope is a constant declaration of a double
	SCOPE_TYPE_CONSTANT_DOUBLE,							  // Scope is a constant declaration of a double
	SCOPE_TYPE_CONSTANT_BOOL,							  // Scope is a constant declaration of a boolean
	SCOPE_TYPE_CONSTANT_ARRAY,							  // Scope is a constant declaration of an array
} scope_types;
typedef enum scope_variable_states
{
	SCOPE_VARIABLE = (SCOPE_TYPE_CONSTANT_ARRAY + 1), // Scope a variable
	SCOPE_VARIABLE_UPDATE,							  // Scope is updating a variable
} scope_variable_states;
typedef enum scope_logic
{
	SCOPE_LOGIC_EQUAL = (SCOPE_VARIABLE_UPDATE + 1), // Scope logic: equal
	SCOPE_LOGIC_NOT,								 // Scope logic: not
	SCOPE_LOGIC_MORE,								 // Scope logic: more
	SCOPE_LOGIC_MORE_OR_EQUAL,						 // Scope logic: more or equal
	SCOPE_LOGIC_LESS,								 // Scope logic: less
	SCOPE_LOGIC_LESS_OR_EQUAL,						 // Scope logic: less or equal
	SCOPE_LOGIC_AND,								 // Scope logic: and
} scope_logic;
typedef enum scope_math
{
	SCOPE_MATH_ADD = (SCOPE_LOGIC_AND + 1), // Scope math: add
	SCOPE_MATH_SUBTRACT,					// Scope math: substract
	SCOPE_MATH_MULTIPLY,					// Scope math: multiply
	SCOPE_MATH_DIVIDE,						// Scope math: divide
	SCOPE_MATH_MODULO,						// Scope math: divide
} scope_math;
#define SCOPE_BOOLEAN_TRUE 1.0	// Scope boolean true
#define SCOPE_BOOLEAN_FALSE 0.0 // Scope boolean false
#define SCOPE_DEFAULT_VALUE SCOPE_BOOLEAN_FALSE

// Scope structure
struct scope_t
{
	struct scope_t *left;  // Left branch
	struct scope_t *right; // Right branch
	unsigned char type;	   // Branch type
	struct stack_result_t *result;
	struct stack_parent_t *call_stack; // Call stack parental references
	unsigned char await;			   // Branch is awaiting result
};

typedef struct scope_t scope_t;

// Create a new scope
scope_t *scope_new();
// Set type of scope
int scope_set_type(scope_t *scope, int set_type);
// Get type of scope
int scope_get_type(scope_t *scope);
// Set scope leftward child scope
void scope_set_left(scope_t *scope, scope_t *set_scope);
// Get scope leftward child scope
scope_t *scope_get_left(scope_t *scope);
// Set scope rightward child scope
void scope_set_right(scope_t *scope, scope_t *set_scope);
// Get scope rightward child scope
scope_t *scope_get_right(scope_t *scope);
// Free up scope memory
void scope_destroy(scope_t *scope);
// Set scope result value
void scope_set_result(scope_t *scope, double set_result);
// Set scope result value as constant
void scope_set_result_constant(scope_t *scope, double set_result);
// Set scope result type
void scope_set_result_type(unsigned char set_result_type);
// Get scope result value
double scope_get_result(scope_t *scope);
// Get scope result type
int scope_get_result_type(scope_t *scope);
// Clear scope result value
void scope_clear_result(scope_t *scope);
// Get scope's awaited status
int scope_await(scope_t *scope);
// Append scope to array
int scope_array_append(scope_t *scope_destination, scope_t *scope_source);

#endif