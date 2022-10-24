#ifndef COMPILER_PARSER_H
#define COMPILER_PARSER_H

#include "tokenizer.h"

typedef struct Parser Parser;

struct Parser {
		Token *previous;
		int current;
};
void initParser(Parser *parser);
void freeParser(Parser *parser);
void parse();

extern Parser parser;

#endif
