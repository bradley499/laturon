#ifdef EMSCRIPTEN
#include <emscripten/emscripten.h>
#endif
#include <stdio.h>
#include "interact.h"
#include "tokenizer.h"
#include "misc.h"
#include "parse.h"
#include "run.h"

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

FILE *fp = NULL;

#ifdef EMSCRIPTEN
int EMSCRIPTEN_KEEPALIVE versioning()
#else
int versioning()
#endif
{
	output("Version: " NUM_WRAPPER(VERSION), OUTPUT_INFO);
	output("Build: " NUM_WRAPPER(BUILDNUMBER), OUTPUT_INFO);
	return 0;
}

#ifndef EMSCRIPTEN
int main(int argc, char **argv)
{
	if (argc != 2)
	{
		output("Please give a location of your source code.", OUTPUT_ERROR);
		return 1;
	}
	fp = get_execution_source_file(argv[1]);
	run_file();
	return 0;
}
#endif

#ifdef EMSCRIPTEN
int EMSCRIPTEN_KEEPALIVE run_file()
#else
int run_file()
#endif
{
	if (fp == NULL)
	{
#ifdef EMSCRIPTEN
		fp = get_execution_source_file();
		if (fp == NULL)
		{
#endif
			fatal_error(IO_ERROR);
#ifdef EMSCRIPTEN
			return 1;
		}
#endif
	}
	parse_cleanup();
	variable_initialisation();
	run_stack_reset();
	tokenize_file(fp);
	fclose(fp);
	fp = NULL;
	parsed_function_scope_t *functions = NULL;
	if (parse_tokens(&functions) == 0)
	{
#ifdef EMSCRIPTEN
		set_load_state(EXECUTION_COMPLETED);
#endif
		return 0; // Nothing to execute
	}
#ifdef EMSCRIPTEN
	set_load_state(PARSING_DONE);
#endif
	run(&functions);
#ifdef EMSCRIPTEN
	set_load_state(EXECUTION_COMPLETED);
#endif
	return 0;
}
