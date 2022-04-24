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
#include <time.h>
#include <limits.h>
#include <emscripten/emscripten.h>

#define INTERACT_WASM_SOURCE_FILE "./user_source_file"
#define JAVASCRIPT_INPUT_MAX_SIZE (5 * 1024)

time_t execution_time = 0;
unsigned short execution_output = 0;
unsigned int stopable = BOOLEAN_FALSE;

EM_JS(void, js_request_user_input, (), {
	resetUserInputState();
	requestUserInput();
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

EM_JS(void, js_error, (unsigned int code, unsigned long long line), {
	error(code, line);
});

EM_JS(void, js_set_load_state, (unsigned int state), {
	setLoadState(state);
});

EM_JS(int, js_check_stopping_state, (), {
	return checkStoppingState();
});

char *input()
{
	js_request_user_input();
	do
	{
		emscripten_sleep(100);
	} while (!js_get_user_input_state());
	return js_get_user_input();
}

void output(char *message, output_types type)
{
	time_t current_clock = time(NULL);
	unsigned long long delay = 0;
	if (current_clock < (execution_time + 2))
	{
		if (execution_output++ > 500)
		{
			delay = (50 * (int)(execution_output / 50));
			if (execution_output == 5000)
				execution_output = 4999;
		}
	}
	execution_time = current_clock;
	js_output(message, type);
	if (delay != 0)
		emscripten_sleep(50 * (int)(execution_output / 500));
}

void error_code(unsigned int code)
{
	js_error(code, 0);
	exit(EXIT_FAILURE);
}

void error_code_lined(unsigned int code, unsigned long long line)
{
	js_error(code, line);
	exit(EXIT_FAILURE);
}

FILE* get_execution_source_file()
{
	if (access(INTERACT_WASM_SOURCE_FILE, F_OK) == 0) {
		FILE* fp = fopen(INTERACT_WASM_SOURCE_FILE, "r");
		if (fp != NULL)
			return fp;
	}
	fatal_error(IO_ERROR);
	return NULL;
}

void set_load_state(unsigned int state)
{
	js_set_load_state(state);
}

int stopping_execution()
{
	int stopping = js_check_stopping_state();
	if (stopping == BOOLEAN_TRUE)
		execution_output = 0;
	return stopping;
}

#else

char *input(char *message)
{
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

void output(char *message, output_types type)
{
	switch (type)
	{
	case OUTPUT_ERROR:
		printf("\033[0;31m[ERROR] %s\033[0m\n", message);
		break;
	case OUTPUT_WARNING:
		printf("\033[0;33m[WARNING] %s\033[0m\n", message);
		break;
	case OUTPUT_INFO:
		printf("\033[0;36m[INFO] %s\033[0m\n", message);
		break;
	default:
		printf("%s\n", message);
		break;
	}
}

#define ERROR_MESSAGE_SIZE 140

void error_code(error_codes code)
{
	char buffer[ERROR_MESSAGE_SIZE];
	switch (code)
	{
	case MEMORY_ALLOCATION_ERROR:
		copy_string(buffer, "Failed to allocate memory.");
		break;
	case LOGIC_ERROR:
		copy_string(buffer, "Logical operation failed.");
		break;
	case CONVERSION_ERROR:
		copy_string(buffer, "Failure to convert to another type.");
		break;
	case CLEANUP_ERROR:
		copy_string(buffer, "Failed to cleanup variables outside of current scope.");
		break;
	case EXECUTION_ERROR:
		copy_string(buffer, "Failed to execute operation.");
		break;
	case STACK_REFERENCE_ERROR:
		copy_string(buffer, "An invalid reference to a call stack scope occurred.");
		break;
	case STRING_INPUT_ERROR:
		copy_string(buffer, "Failed to correctly read in user input string.");
		break;
	case MISSING_COMPOUND_LITERAL:
		copy_string(buffer, "A reference to a compound literal does not exist.");
		break;
	case ARRAY_TYPE_EXPECTED_ERROR:
		copy_string(buffer, "An array routine was not given an array to operate on.");
		break;
	case ARRAY_LOCATION_ERROR:
		copy_string(buffer, "A reference to an item within an array that is out of range.");
		break;
	case IO_ERROR:
		copy_string(buffer, "Failed to perform an operation on source file.");
		break;
	case INVALID_SYNTAX_GENERIC:
		copy_string(buffer, "The source provided has invalid syntax.");
		break;
	case STACK_MEMORY_LIMIT_REACHED:
		copy_string(buffer, "The total amount of stack memory available to execute your program has been reached.");
		break;
	case TOO_BIG_NUMERIC:
		copy_string(buffer, "Too many functions or variables are declared within your program to be handled within memory.");
		break;
	case VALUE_NOT_SET:
		copy_string(buffer, "Attempted to use a variable which has not been set yet.");
		break;
	case ZERO_DIVISION_ERROR:
		copy_string(buffer, "Zero division error.");
		break;
	case UNKNOWN_ERROR:
	default:
		copy_string(buffer, "An unknown error.");
		break;
	}
	output(buffer, OUTPUT_ERROR);
	exit((code + 1));
}

void error_code_lined(error_codes code, unsigned long long line)
{
	char buffer[ERROR_MESSAGE_SIZE];
	switch (code)
	{
	case MEMORY_ALLOCATION_ERROR:
		snprintf(buffer, ERROR_MESSAGE_SIZE, "Failed to allocate memory. On line: %lld", line);
		break;
	case LOGIC_ERROR:
		snprintf(buffer, ERROR_MESSAGE_SIZE, "Logical operation failed. On line: %lld", line);
		break;
	case CONVERSION_ERROR:
		snprintf(buffer, ERROR_MESSAGE_SIZE, "Failure to convert to another type. On line: %lld", line);
		break;
	case CLEANUP_ERROR:
		snprintf(buffer, ERROR_MESSAGE_SIZE, "Failed to cleanup variables outside of current scope. On line: %lld", line);
		break;
	case EXECUTION_ERROR:
		snprintf(buffer, ERROR_MESSAGE_SIZE, "Failed to execute operation. On line: %lld", line);
		break;
	case STACK_REFERENCE_ERROR:
		snprintf(buffer, ERROR_MESSAGE_SIZE, "An invalid reference to a call stack scope occurred. On line: %lld", line);
		break;
	case STRING_INPUT_ERROR:
		snprintf(buffer, ERROR_MESSAGE_SIZE, "Failed to correctly read in user input string. On line: %lld", line);
		break;
	case MISSING_COMPOUND_LITERAL:
		snprintf(buffer, ERROR_MESSAGE_SIZE, "A reference to a compound literal does not exist. On line: %lld", line);
		break;
	case ARRAY_TYPE_EXPECTED_ERROR:
		snprintf(buffer, ERROR_MESSAGE_SIZE, "An array routine was not given an array to operate on. On line: %lld", line);
		break;
	case ARRAY_LOCATION_ERROR:
		snprintf(buffer, ERROR_MESSAGE_SIZE, "A reference to an item within an array that is out of range. On line: %lld", line);
		break;
	case IO_ERROR:
		snprintf(buffer, ERROR_MESSAGE_SIZE, "Failed to perform an operation on source file. On line: %lld", line);
		break;
	case INVALID_SYNTAX_GENERIC:
		snprintf(buffer, ERROR_MESSAGE_SIZE, "The source provided has invalid syntax. On line: %lld", line);
		break;
	case STACK_MEMORY_LIMIT_REACHED:
		snprintf(buffer, ERROR_MESSAGE_SIZE, "The total amount of stack memory available to execute your program has been reached. On line: %lld", line);
		break;
	case TOO_BIG_NUMERIC:
		snprintf(buffer, ERROR_MESSAGE_SIZE, "Too many functions or variables are declared within your program to be handled within memory. On line: %lld", line);
		break;
	case VALUE_NOT_SET:
		snprintf(buffer, "Attempted to use a variable which has not been set yet. On line: %lld\n", line);
		break;
	case ZERO_DIVISION_ERROR:
		snprintf(buffer, "Zero division error. On line: %lld\n", line);
		break;
	case UNKNOWN_ERROR:
	default:
		snprintf(buffer, ERROR_MESSAGE_SIZE, "An unknown error. On line: %lld", line);
		break;
	}
	output(buffer, OUTPUT_ERROR);
	exit((code + 1));
}

FILE* get_execution_source_file(char* filename)
{
	if (access(filename, F_OK) == 0) {
		FILE* fp = fopen(filename, "r");
		if (fp != NULL)
			return fp;
	}
	fatal_error(IO_ERROR);
	return NULL;
}
#endif

#undef _GNU_SOURCE

#endif