#ifndef COMPILER_PARSER_H
#define COMPILER_PARSER_H

#include "tokenizer.h"

/*
 * The parser holds:
 *   + a reference to the most recent token parsed
 *   + an index pointing at where we are in the global array of tokens.
 * The parser does not own the memory of [Token *previous]. Tokenizer will be responsible for
 * freeing it.
 * There will be one global instance of the struct Parser throughout the whole parsing operation.
 * */

typedef struct Parser Parser;

struct Parser {
		Token *previous;
		int current;
};
void initParser(Parser *parser);
void freeParser(Parser *parser);

/*
 * Generates Bytecode from the list of tokens given by the tokenizer.
 * The Bytecode generated is written in the bytearray of our virtual machine.
 * */
void parse();

extern Parser parser;

#endif
