#include "parser.h"
#include "error.h"
#include "value.h"
#include "vm.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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

// Checks whether the current token type matches the types passed in as a parameter.
// If the types match, it eats the token, otherwise it returns an error and exits the system.
static void eatTokenOrReturnError(TokenType type, const char *message) {
		CHECK(!reachedEOF() && peekToken()->type == type, message);
		eatToken();
}

// Stores the most recent token in the field `parser.previous` and advances the current index.
// `parser.current` index always points at the token that is about to be parsed.
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

// Checks whether the current token type matches the type passed in as a parameter.
// If the types match, it will eat the token and advance the `current` index, otherwise it will
// return false.
static bool matchAndEatToken(TokenType type) {
		if(reachedEOF() || peekToken()->type != type) return false;
		eatToken();
		return true;
}

static bool stringEquals(char *s, const char *t) {
		return strcmp(s, t) == 0;
}

/*
 * The functions down below represent the implementation of a recursive descent parser.
 * It is based on the following production rules:
 *
 *   + expression        -->  equality
 *
 *   + equality          -->  comparison | comparison ( '==' | '!=' ) comparison
 *
 *   + comparison        -->  term | term ( '+' | '-' ) term
 *
 *   + term              -->  factor | factor ( '*' | '/' ) factor
 *
 *   + unary             -->  ('!' | '-') unary | primary
 *
 *   + primary           -->  '(' expression ')' | number_literal | string_literal | 
 *   													 boolean_literal | identifier
 *
 * The priority of the production rules are in ascending order.
 * In other words, `primary` has the highest priority and `expression` has the lowest.
 * */

static void expression() {
		equality();
}

static void equality() {
		comparison();
		while(matchAndEatToken(TOKEN_EQUAL_EQUAL) || matchAndEatToken(TOKEN_BANG_EQUAL)) {
				char *operator = parser.previous->lexeme;
				comparison();

				// Actions associated with the production `equality`
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

				// Actions associated with the production `comparison`
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

				// Actions associated with the production `term`
				if(stringEquals(operator, "+")) writeByteArray(&vm.code, OP_ADD);
				if(stringEquals(operator, "-")) writeByteArray(&vm.code, OP_SUBSTRACT);
		}
}

static void factor() {
		unary();
		while(matchAndEatToken(TOKEN_STAR) || matchAndEatToken(TOKEN_SLASH)) {
				char *operator = parser.previous->lexeme;
				unary();

				// Actions associated with the production `factor`
				if(stringEquals(operator, "*")) writeByteArray(&vm.code, OP_MULTIPLY);
				if(stringEquals(operator, "/")) writeByteArray(&vm.code, OP_DIVIDE);
		}
}

static void unary() {
		while(matchAndEatToken(TOKEN_BANG) || matchAndEatToken(TOKEN_MINUS)) {
				char *operator = parser.previous->lexeme;
				unary();

				// Actions associated with the production `unary`
				if(stringEquals(operator, "!")) writeByteArray(&vm.code, OP_NOT);
				if(stringEquals(operator, "-")) writeByteArray(&vm.code, OP_NEGATE);
		}
		primary();
}

static void primary() {
		if(reachedEOF()) return;

		// Actions associated with the terminal tokens.
		if(matchAndEatToken(TOKEN_LEFT_PAREN)) {
				expression();
				eatTokenOrReturnError(TOKEN_RIGHT_PAREN, "Expected ')' after the end of the expression");
		}
		// We currently can't handle more than 255 values in our program because we only use one byte
		// to encode the position of our value in the value_array of the virtual machine.
		// This is why the variable `pos_on_value_array` is of type uint8_t.
		else if(matchAndEatToken(TOKEN_NUMBER)) {
				double number = strtod(parser.previous->lexeme, /*endPtr = */ NULL);
				Value value = CREATE_NUMBER(number);
				uint8_t pos_on_value_array = writeValueArray(&vm.value_array, value); 
				writeByteArray(&vm.code, OP_VALUE);
				writeByteArray(&vm.code, pos_on_value_array);
		}
		else if(matchAndEatToken(TOKEN_TRUE) || matchAndEatToken(TOKEN_FALSE)) {
				bool boolean = stringEquals(parser.previous->lexeme, "true");
				Value value = CREATE_BOOLEAN(boolean);
				uint8_t pos_on_value_array = writeValueArray(&vm.value_array, value);
				writeByteArray(&vm.code, OP_VALUE);
				writeByteArray(&vm.code, pos_on_value_array);
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
