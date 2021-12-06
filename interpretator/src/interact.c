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

#endif