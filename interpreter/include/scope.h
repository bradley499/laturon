#ifndef scope_h
#define scope_h

#include "stack.h"

#define SCOPE_SUCCESS 0				  // Successful operation
#define SCOPE_FAILURE 1				  // Failed operation
#define SCOPE_CREATION_FAILURE 2	  // Failed to allocate memory for new scope
#define SCOPE_UNDEFINED 3			  // Scope has not been configured
#define SCOPE_AWAIT_READY 4			  // Scope is await has not been declared
#define SCOPE_AWAITING 5			  // Scope is awaiting primary completion
#define SCOPE_AWAIT_FINISHED 6		  // Scope operations have finished
#define SCOPE_AWAITING_CONCLUSION 7	  // Scope is awaiting secondary completion
#define SCOPE_AWAIT_NO 8			  // Scope does not require completion awaits
#define SCOPE_AWAITING_DELETION 9	  // Scope declaration for destruction
#define SCOPE_TYPE_CONTAINER 10		  // Scope contains other general scopes
#define SCOPE_TYPE_IF_CONTAINER 11	  // Scope is for an if container declaration
#define SCOPE_TYPE_IF_ELSE 12		  // Scope is for an if, else declaration
#define SCOPE_TYPE_INT 13			  // Scope is integer declaration
#define SCOPE_TYPE_BOOL 14			  // Scope is boolean declaration
#define SCOPE_TYPE_DOUBLE 15		  // Scope is double declaration
#define SCOPE_TYPE_STRING 16		  // Scope is string declaration
#define SCOPE_TYPE_FUNCTION 17		  // Scope is a function declaration
#define SCOPE_TYPE_FUNCTION_CALL 18	  // Scope is a function call
#define SCOPE_TYPE_PARAMETER_START 19 // Scope is a declarting the start of a parameter chain
#define SCOPE_TYPE_PARAMETER_END 20	  // Scope is a declarting the end of a parameter chain
#define SCOPE_TYPE_ARRAY 21			  // Scope is a array declaration
#define SCOPE_TYPE_ARRAY_AT 22		  // Scope is a array value at position declaration
#define SCOPE_TYPE_CONSTANT_INT 23	  // Scope is a constant declaration of a double
#define SCOPE_TYPE_CONSTANT_DOUBLE 24 // Scope is a constant declaration of a double
#define SCOPE_TYPE_CONSTANT_BOOL 25	  // Scope is a constant declaration of a boolean
#define SCOPE_TYPE_CONSTANT_ARRAY 26  // Scope is a constant declaration of an array
#define SCOPE_VARIABLE 27			  // Scope a variable
#define SCOPE_VARIABLE_UPDATE 28	  // Scope is updating a variable
#define SCOPE_LOGIC_EQUAL 29		  // Scope logic: equal
#define SCOPE_LOGIC_NOT 30			  // Scope logic: not
#define SCOPE_LOGIC_MORE 31			  // Scope logic: more
#define SCOPE_LOGIC_MORE_OR_EQUAL 32  // Scope logic: more or equal
#define SCOPE_LOGIC_LESS 33			  // Scope logic: less
#define SCOPE_LOGIC_LESS_OR_EQUAL 34  // Scope logic: less or equal
#define SCOPE_LOGIC_AND 35			  // Scope logic: and
#define SCOPE_MATH_ADD 36			  // Scope math: add
#define SCOPE_MATH_SUBTRACT 37		  // Scope math: substract
#define SCOPE_MATH_MULTIPLY 38		  // Scope math: multiply
#define SCOPE_MATH_DIVIDE 39		  // Scope math: divide
#define SCOPE_MATH_MODULO 40		  // Scope math: divide
#define SCOPE_BOOLEAN_TRUE 1.0		  // Scope boolean true
#define SCOPE_BOOLEAN_FALSE 0.0		  // Scope boolean false
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