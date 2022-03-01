#ifndef interact_c
#define interact_c

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "interact.h"
#include "misc.h"

#ifdef EMSCRIPTEN
#include <stdbool.h>
#include <emscripten/emscripten.h>

#define INTERACT_WASM_SOURCE_FILE "./user_source_file"
#define JAVASCRIPT_INPUT_MAX_SIZE (5 * 1024)

EM_JS(void, js_request_user_input, (const char *message), {
	resetUserInputState();
	requestUserInput(UTF8ToString(message));
});

EM_JS(void, js_reset_user_input, (), {
	resetUserInputState();
});

EM_JS(bool, js_get_user_input_state, (), {
	return getUserInputState();
});

EM_JS(char *, js_get_user_input, (), {
	var userInput = getUserInput();
	var lengthBytes = lengthBytesUTF8(userInput) + 1;
	var stringOnWasmHeap = _malloc(lengthBytes);
	stringToUTF8(userInput, stringOnWasmHeap, lengthBytes);
	return stringOnWasmHeap;
});

EM_JS(void, js_output, (const char *message, unsigned char type), {
	outputMessage(UTF8ToString(message), type);
});

EM_JS(void, js_error, (unsigned int code), {
	error(code);
});

EM_JS(void, js_started, (), {
	initialLoadComplete();
});

char *input(char *message)
{
	js_request_user_input(message);
	do
	{
		emscripten_sleep(100);
	} while (!js_get_user_input_state());
	return js_get_user_input();
}

void output(char *message, output_types type)
{
	js_output(message, type);
	emscripten_sleep(100); // Brief sleep to allow for JavaScript engine to render outputs.
}

void error_code(unsigned int code)
{
	js_error(code);
	exit((code + 1));
}

FILE* EMSCRIPTEN_KEEPALIVE cleanExecutionSourceFile() {
	if (access(INTERACT_WASM_SOURCE_FILE, F_OK) == 0) {
		if (remove(INTERACT_WASM_SOURCE_FILE) != 0) {
			fatal_error(IO_ERROR);
		}
	}
	FILE* fp = fopen(INTERACT_WASM_SOURCE_FILE, "w+");
	return fp;
}

FILE* startUpGetFile()
{
	FILE* fp = cleanExecutionSourceFile();
	if (fp == NULL)
		fatal_error(IO_ERROR);
	js_started();
	return fp;
}

#else

char *input(char *message)
{
	printf("%s", message);
	char *input_string = NULL;
	size_t len = 0;
	long line_size = 0;
	line_size = getline(&input_string, &len, stdin);
	if (len <= 0)
	{
		if (input_string != NULL)
			free(input_string);
		fatal_error(STRING_INPUT_ERROR);
	}
	else
		input_string[(line_size - 1)] = '\0';
	FILE *tes;
	return input_string;
}

void output(char *message, output_types type)
{
	printf("%s\n", message);
}

void error_code(unsigned int code)
{
	exit((code + 1));
}

FILE* startUpGetFile(char* filename)
{
	if (access(filename, F_OK) == 0) {
		FILE* fp = fopen(filename, "r");
		if (fp != NULL)
			return fp;
	}
	fatal_error(IO_ERROR);
}
#endif

#undef _GNU_SOURCE

#endif