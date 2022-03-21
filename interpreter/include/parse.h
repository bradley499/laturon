#ifndef parse_h
#define parse_h

// Initialises input/output functions
int parse_init();
// Parses tokens and generates scope graph structure
int parse_tokens();
// Clean up all scopes and objects
void parse_cleanup(int destroy_scopes);
#endif