#ifndef interact_h
#define interact_h

#include <stdio.h>

typedef enum output_types {
	OUTPUT_INFO,
	OUTPUT_WARNING,
	OUTPUT_ERROR,
	OUTPUT_GENERIC,
} output_types;

// Request input from user
char * input(char *message);
// Output to user
void output(char *message, output_types type);
// Exit error code
void error_code(unsigned int code);
#ifdef EMSCRIPTEN
// Return the file pointer of source code
FILE* get_execution_source_file();
// Execution has begun
void ready();
// Set the current loading state
void set_load_state(unsigned int state);

typedef enum loading_states {
	TOKENIZING_DONE = 2,
	PARSING_DONE
} loading_states;
#else
// Return the file pointer of source code
FILE* get_execution_source_file(char* filename);
#endif
#endif