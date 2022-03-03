#ifdef EMSCRIPTEN
#include <emscripten/emscripten.h>
#endif
#include <stdio.h>
#include "interact.h"
#include "lexer.h"
#include "misc.h"

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
	fp = getExecutionSourceFile(argv[0]);
	run_file();
#endif
	return 0;
}

int EMSCRIPTEN_KEEPALIVE run_file() {
	if (fp == NULL) {
#ifdef EMSCRIPTEN
		fp = getExecutionSourceFile();
		if (fp == NULL) {
#endif
			fatal_error(IO_ERROR);
#ifdef EMSCRIPTEN
			return 1;
		}
#endif
	}
	execute_file(fp);
	fclose(fp);
	fp = NULL;
	return 0;
}