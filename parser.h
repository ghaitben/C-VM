#ifndef COMPILER_PARSER_H
#define COMPILER_PARSER_H

#include "tokenizer.h"

#define WRITE_VALUE(value_type, ...) \
		do { \
				writeByteArray(&current_function->code, OP_VALUE); \
				uint8_t pos_on_value_array = writeValueArray(&vm.value_array, value_type(__VA_ARGS__)); \
				writeByteArray(&current_function->code, pos_on_value_array); \
		}while(false)


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
