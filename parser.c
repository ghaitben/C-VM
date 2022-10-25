#include "parser.h"
#include "error.h"
#include "value.h"
#include "vm.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declaration of static helper functions.
static Token *eatToken();
static Token *peekToken();
static void expression();
static void term();
static void factor();
static void equality();
static void primary();
static void unary();
static void comparison();
static bool matchAndEatToken(TokenType type);
static void eatTokenOrReturnError(TokenType type, const char *message);
static bool reachedEOF();
static bool stringEquals();

Parser parser;

void initParser(Parser *parser) {
		parser->previous = NULL;
		parser->current = 0;
}

void freeParser(Parser *parser) {
		initParser(parser);
}

static void eatTokenOrReturnError(TokenType type, const char *message) {
		CHECK(!reachedEOF() && peekToken()->type == type, message);
		eatToken();
}

static Token *eatToken() {
		parser.previous = &tokenizer.token_array.array[parser.current];
		parser.current++;
		return &tokenizer.token_array.array[parser.current - 1];
}

static Token *peekToken() {
		return &tokenizer.token_array.array[parser.current];
}

static bool reachedEOF() {
		return tokenizer.token_array.array[parser.current].type == TOKEN_EOF;
}

static bool matchAndEatToken(TokenType type) {
		if(reachedEOF() || peekToken()->type != type) return false;
		eatToken();
		return true;
}

static bool stringEquals(char *s, const char *t) {
		return strcmp(s, t) == 0;
}

static void expression() {
		equality();
}

static void equality() {
		comparison();
		while(matchAndEatToken(TOKEN_EQUAL_EQUAL) || matchAndEatToken(TOKEN_BANG_EQUAL)) {
				char *operator = parser.previous->lexeme;
				comparison();

				if(stringEquals(operator, "==")) writeByteArray(&vm.code, OP_EQUAL_EQUAL);
				if(stringEquals(operator, "!=")) writeByteArray(&vm.code, OP_BANG_EQUAL);
		}
}

static void comparison() {
		term();
		while(matchAndEatToken(TOKEN_LESS_EQUAL) || matchAndEatToken(TOKEN_LESS) || 
						matchAndEatToken(TOKEN_GREATER) || matchAndEatToken(TOKEN_GREATER_EQUAL))
		{
				char *operator = parser.previous->lexeme;
				term();

				if(stringEquals(operator, ">=")) writeByteArray(&vm.code, OP_GREATER_EQUAL);
				if(stringEquals(operator, "<=")) writeByteArray(&vm.code, OP_LESS_EQUAL);
				if(stringEquals(operator, ">")) writeByteArray(&vm.code, OP_GREATER);
				if(stringEquals(operator, "<")) writeByteArray(&vm.code, OP_LESS);
		}
}

static void term() {
		factor();
		while(matchAndEatToken(TOKEN_PLUS) || matchAndEatToken(TOKEN_MINUS)) {
				char *operator = parser.previous->lexeme;
				factor();
				if(stringEquals(operator, "+")) writeByteArray(&vm.code, OP_ADD);
				if(stringEquals(operator, "-")) writeByteArray(&vm.code, OP_SUBSTRACT);
		}
}

static void factor() {
		unary();
		while(matchAndEatToken(TOKEN_STAR) || matchAndEatToken(TOKEN_SLASH)) {
				char *operator = parser.previous->lexeme;
				unary();
				if(stringEquals(operator, "*")) writeByteArray(&vm.code, OP_MULTIPLY);
				if(stringEquals(operator, "/")) writeByteArray(&vm.code, OP_DIVIDE);
		}
}

static void unary() {
		while(matchAndEatToken(TOKEN_BANG) || matchAndEatToken(TOKEN_MINUS)) {
				char *operator = parser.previous->lexeme;
				unary();
				if(stringEquals(operator, "!")) writeByteArray(&vm.code, OP_NOT);
				if(stringEquals(operator, "-")) writeByteArray(&vm.code, OP_NEGATE);
		}
		primary();
}

static void primary() {
		if(reachedEOF()) return;

		if(matchAndEatToken(TOKEN_LEFT_PAREN)) {
				expression();
				eatTokenOrReturnError(TOKEN_RIGHT_PAREN, "Expected ')' after the end of the expression");
		}
		else if(matchAndEatToken(TOKEN_NUMBER)) {
				double number = strtod(parser.previous->lexeme, /*endPtr = */ NULL);
				Value value = CREATE_NUMBER(number);
				int pos_on_value_array = writeValueArray(&vm.value_array, value); 
				writeByteArray(&vm.code, OP_VALUE);
				writeByteArray(&vm.code, (uint8_t) pos_on_value_array);
		}
		else if(matchAndEatToken(TOKEN_TRUE)) {
				bool boolean = stringEquals(parser.previous->lexeme, "true");
				Value value = CREATE_BOOLEAN(boolean);
				int pos_on_value_array = writeValueArray(&vm.value_array, value);
				writeByteArray(&vm.code, OP_VALUE);
				writeByteArray(&vm.code, (uint8_t) pos_on_value_array);
		}
		else if(matchAndEatToken(TOKEN_STRING)) {
				printf("%s\n", parser.previous->lexeme);
		}
		else {
				CHECK(/*condition = */false, "Unknown Token");
		}
}

void parse() {
		expression();
}
