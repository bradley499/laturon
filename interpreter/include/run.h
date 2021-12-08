#ifndef run_h
#define run_h

#include "scope.h"

#define RUN_SUCCESS                 0  // Successfully completed operation
#define RUN_FAILURE                 1  // Failed to complete operation
#define RUN_FAILURE_CLEANING_UP     2  // Currently clearing up scope's memory tree
#define RUN_MEMORY_ALLOCATION_ERROR 3  // Failed to allocate memory
#define RUN_MEMORY_EXHAUSTION_ERROR 4  // All avaliable memory for operation was exhausted
#define RUN_STRING_LOGIC_ERROR      5  // Failed to complete operation due to string being used in a logical statement
#define RUN_ARRAY_LOGIC_ERROR       6  // Failed to complete operation due to array being used in a logical statement
#define RUN_SWITCH_NODE             7  // Switch to different node
#define RUN_STEP_CONTINUE           8  // Proceed to the next step
#define RUN_STEP_WAIT               9  // Step needs to input/output before continuing
#define RUN_STEP_OUT                10  // An output is ready
#define RUN_STEP_FINISHED           11 // No more steps remain
#define RUN_PARAMETER_STACK_OPEN    12 // Add values to the parameter stack
#define RUN_PARAMETER_STACK_CLOSED  13 // Do not add values to the parameter stack
#define RUN_CONVERSION_FAILURE      14 // A the source for a conversion to another data type was not understood

// Sets the initial scope for this execution
void set_initial_scope(struct scope_t *initial_scope);
// Executes the next instruction
int step();
// Starts execution
int run();
// Return the level of the scope
unsigned int scope_size();

#endif