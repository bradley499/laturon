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
// Signifies start up, and return the pointer of source code
#ifdef EMSCRIPTEN
FILE* startUpGetFile();
#else
FILE* startUpGetFile(char* filename);
#endif
#endif