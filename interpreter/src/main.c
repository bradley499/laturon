#ifdef EMSCRIPTEN
#include <emscripten/emscripten.h>
#endif
#include <stdio.h>
#include "interact.h"
#include "lexer.h"

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

#ifdef EMSCRIPTEN
int EMSCRIPTEN_KEEPALIVE main() {
	FILE* fp = startUpGetFile();
	executeFile(fp);
#else
int EMSCRIPTEN_KEEPALIVE main(int argc, char **argv) {
	if (argc != 0) {
		output("Please give a location of your source code.", OUTPUT_ERROR);
		return 0;
	}
	FILE* fp = startUpGetFile();
#endif
	output("Version: "NUM_WRAPPER(VERSION), OUTPUT_INFO);
	output("Build:   "NUM_WRAPPER(BUILDNUMBER), OUTPUT_INFO);
	return 0;
}