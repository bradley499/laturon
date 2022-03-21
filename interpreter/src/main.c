#ifdef EMSCRIPTEN
#include <emscripten/emscripten.h>
#endif
#include <stdio.h>
#include "interact.h"
#include "tokenizer.h"
#include "misc.h"
#include "parse.h"

#ifndef VERSION
#error "No version was defined during compilation"
#endif
#ifndef BUILDNUMBER
#error "No build number was defined during compilation"
#endif

#define NUM_WRAPPER_VALUE(x) #x
#define NUM_WRAPPER(x) NUM_WRAPPER_VALUE(x)

#ifndef EMSCRIPTEN_KEEPALIVE
#define EMSCRIPTEN_KEEPALIVE
#endif

int run_file();

FILE* fp = NULL;

#ifdef EMSCRIPTEN
char run_state = 0;

int EMSCRIPTEN_KEEPALIVE main() {
	ready();
	output("Version: "NUM_WRAPPER(VERSION), OUTPUT_INFO);
	output("Build:   "NUM_WRAPPER(BUILDNUMBER), OUTPUT_INFO);
#else
int main(int argc, char **argv) {
	if (argc != 2) {
		output("Please give a location of your source code.", OUTPUT_ERROR);
		return 1;
	}
	fp = get_execution_source_file(argv[1]);
	run_file();
#endif
	return 0;
}

int EMSCRIPTEN_KEEPALIVE run_file() {
#ifdef EMSCRIPTEN
	if (run_state == 1)
	{
		fclose(fp);
		fp = NULL;
	}
#endif
	if (fp == NULL) {
#ifdef EMSCRIPTEN
		fp = get_execution_source_file();
		if (fp == NULL) {
#endif
			fatal_error(IO_ERROR);
#ifdef EMSCRIPTEN
			return 1;
		}
#endif
	}
#ifdef EMSCRIPTEN
	run_state = 1;
#endif
	parse_cleanup(1);
	parse_init();
	tokenize_file(fp);
#ifdef EMSCRIPTEN
	set_load_state(TOKENIZING_DONE);
	run_state = 2;
#endif
	fclose(fp);
	fp = NULL;
	parse_tokens();
#ifdef EMSCRIPTEN
	set_load_state(PARSING_DONE);
#endif
	return 0;
}