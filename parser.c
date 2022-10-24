#include "parser.h"
#include "error.h"
#include <stddef.h>
#include <stdio.h>

// Forward declaration of static helper functions.
static Token *eatToken();
static Token *peekToken();
static void expression();
static void term();
static void factor();
static void equality();
static void primary();
static void comparison();
static bool matchAndEatToken(TokenType type);
static void eatTokenOrReturnError(TokenType type, const char *message);
static bool reachedEOF();

Parser parser;

void initParser(Parser *parser) {
		parser->previous = NULL;
		parser->current = 0;
}

void freeParser(Parser *parser) {
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

static void expression() {
		equality();
}

static void equality() {
		comparison();
		while(matchAndEatToken(TOKEN_EQUAL_EQUAL) || matchAndEatToken(TOKEN_BANG_EQUAL)) {
				char *operator = parser.previous->lexeme;
				comparison();
				printf("%s\n", operator);
		}
}

static void comparison() {
		term();
		while(matchAndEatToken(TOKEN_LESS_EQUAL) || matchAndEatToken(TOKEN_LESS) || 
						matchAndEatToken(TOKEN_GREATER) || matchAndEatToken(TOKEN_GREATER_EQUAL))
		{
				char *operator = parser.previous->lexeme;
				term();
				printf("%s\n", operator);
		}
}

static void term() {
		factor();
		while(matchAndEatToken(TOKEN_PLUS) || matchAndEatToken(TOKEN_MINUS)) {
				char *operator = parser.previous->lexeme;
				factor();
				printf("%s\n", operator);
		}
}

static void factor() {
		primary();
		while(matchAndEatToken(TOKEN_STAR) || matchAndEatToken(TOKEN_SLASH)) {
				char *operator = parser.previous->lexeme;
				primary();
				printf("%s\n", operator);
		}
}

static void primary() {
		if(matchAndEatToken(TOKEN_LEFT_PAREN)) {
				expression();
				eatTokenOrReturnError(TOKEN_RIGHT_PAREN, "Expected ')' after the end of the expression");
		}
		else if(matchAndEatToken(TOKEN_NUMBER)) {
				printf("%s\n", parser.previous->lexeme);
		}
		else if(matchAndEatToken(TOKEN_STRING)) {
				printf("%s\n", parser.previous->lexeme);
		}
		else CHECK(false, "Unknown token");
}

void parse() {
		expression();
}
