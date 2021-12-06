#ifndef run_h
#define run_h

#include "scope.h"

#define RUN_SUCCESS                 0  // Successfully completed operation
#define RUN_FAILURE                 1  // Failed to complete operation
#define RUN_FAILURE_CLEANING_UP     2  // Currently clearing up scope's memory tree
#define RUN_MEMORY_ALLOCATION_ERROR 3  // Failed to allocate memory
#define RUN_STRING_LOGIC_ERROR      4  // Failed to complete operation due to string being used in a logical statement
#define RUN_ARRAY_LOGIC_ERROR       5  // Failed to complete operation due to array being used in a logical statement
#define RUN_POSITION_TREE_FAILURE   6  // Unable to use position tree
#define RUN_POSITION_BRANCH_NULL    7  // Branch does not have a directional position defined
#define RUN_POSITION_BRANCH_LEFT    8  // Branch directional position is left
#define RUN_POSITION_BRANCH_RIGHT   9  // Branch directional position is right
#define RUN_SWITCH_NODE             10 // Switch to different node
#define RUN_STEP_CONTINUE           11 // Proceed to the next step
#define RUN_STEP_WAIT               12 // Step needs to input/output before continuing
#define RUN_STEP_OUT                13 // An output is ready
#define RUN_STEP_FINISHED           14 // No more steps remain
#define RUN_PARAMETER_STACK_OPEN    15 // Add values to the parameter stack
#define RUN_PARAMETER_STACK_CLOSED  16 // Do not add values to the parameter stack
#define RUN_CONVERSION_FAILURE      17 // A the source for a conversion to another data type was not understood

struct position
{
	unsigned char branch;
	struct position *next;
} __attribute__((__packed__));

int new_tree(int branch_position);
struct position * new_branch();
void delete_tree(struct position *position_tree_node);
void decrease_tree(unsigned int size);
int step(struct scope_t *scope);
void update_scope_branch(struct scope_t *current_scope, struct scope_t *scope);
int run(struct scope_t *scope);
unsigned int scope_size();

#endif