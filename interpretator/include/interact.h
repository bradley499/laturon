#ifndef interact_h
#define interact_h

#ifdef EMSCRIPTEN

char * input(char *message);
void output(char *message);

#else

#warning No input/output exchange supported yet!

#define input(NULL)
#define output(NULL)

#endif

#endif