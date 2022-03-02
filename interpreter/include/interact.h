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
FILE* getExecutionSourceFile();
// Execution has begun
void ready();
#else
// Return the file pointer of source code
FILE* getExecutionSourceFile(char* filename);
#endif
#endif