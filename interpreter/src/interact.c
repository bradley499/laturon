#ifndef interact_c
#define interact_c

#ifdef EMSCRIPTEN
#include <stdbool.h>
#include <emscripten/emscripten.h>

#define JAVASCRIPT_INPUT_MAX_SIZE (5 * 1024)

EM_JS(void, js_request_user_input, (const char* message), {
	resetUserInputState();
	requestUserInput(UTF8ToString(message));
});

EM_JS(void, js_reset_user_input, (), {
	resetUserInputState();
});

EM_JS(bool, js_get_user_input_state, (), {
	return getUserInputState();
});

EM_JS(char*, js_get_user_input, (), {
	var userInput = getUserInput();
	var lengthBytes = lengthBytesUTF8(userInput)+1;
	var stringOnWasmHeap = _malloc(lengthBytes);
	stringToUTF8(userInput, stringOnWasmHeap, lengthBytes);
	return stringOnWasmHeap;
});

EM_JS(void, js_output, (const char* message), {
	outputMessage(UTF8ToString(message));
});

char * input(char *message)
{
	js_request_user_input(message);
	do
	{
	    emscripten_sleep(100);
	} while (!js_get_user_input_state());
	return js_get_user_input();
}

void output(char *message)
{
	js_output(message);
	emscripten_sleep(100); // Brief sleep to allow for JavaScript engine to render outputs.
}

#else
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include "misc.h"

char * input(char *message)
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
	return input_string;
}

void output(char *message)
{
	printf("%s\n", message);
}

#undef _GNU_SOURCE
#endif

#endif