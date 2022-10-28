#include "parser.h"
#include "error.h"
#include "value.h"
#include "debug.h"
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
static bool term();
static bool factor();
static bool equality();
static bool primary();
static bool unary();
static bool comparison();
static bool and();
static void assignment();
static void declaration();
static void statement();
static void expressionStatement();
static void block();
static void varDeclaration();
static void funDeclaration();
static void ifStatement();
static bool matchAndEatToken(TokenType type);
static void eatTokenOrReturnError(TokenType type, const char *message);
static int setCheckPoint(OpCode op_code);
static void setJumpSize(int jump);
static bool reachedEOF();
static bool stringEquals();
static char *dynamicStrCpy(char *s);

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
 * The functions down below represent the implementation of a recursive descent parser
 * for parsing expressions.
 * It is based on the following production rules:
 *
 *   + expression        -->  assignment
 *
 *   + assignment        -->  and ( '=' ) assignment
 *
 *   + and               --> equality 'and' assignment
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
		assignment();
}

static void assignment() {
		bool can_assign = and();
		
		if(!can_assign && peekToken()->type == TOKEN_EQUAL) {
				CHECK(false, "Invalid assignment target");
		}

		char *lexeme = dynamicStrCpy(parser.previous->lexeme);
		if(can_assign && matchAndEatToken(TOKEN_EQUAL)) {
				assignment();

				writeByteArray(&vm.code, OP_ASSIGN);
				WRITE_VALUE(CREATE_STRING, lexeme);
		}
		else {
				free(lexeme);
		}
}

static bool and() {
		bool can_assign = equality();

		if(matchAndEatToken(TOKEN_AND)) {
				can_assign = false;
				int exit_jump = setCheckPoint(OP_JUMP_IF_FALSE);
				assignment();
				setJumpSize(exit_jump);
		}

		return can_assign;
}

static bool equality() {
		bool can_assign = comparison();
		while(matchAndEatToken(TOKEN_EQUAL_EQUAL) || matchAndEatToken(TOKEN_BANG_EQUAL)) {
				can_assign = false;

				char *operator = parser.previous->lexeme;
				comparison();

				// Actions associated with the production `equality`
				if(stringEquals(operator, "==")) writeByteArray(&vm.code, OP_EQUAL_EQUAL);
				if(stringEquals(operator, "!=")) writeByteArray(&vm.code, OP_BANG_EQUAL);
		}
		return can_assign;
}

static bool comparison() {
		bool can_assign = term();
		while(matchAndEatToken(TOKEN_LESS_EQUAL) || matchAndEatToken(TOKEN_LESS) || 
						matchAndEatToken(TOKEN_GREATER) || matchAndEatToken(TOKEN_GREATER_EQUAL))
		{
				can_assign = false;

				char *operator = parser.previous->lexeme;
				term();

				// Actions associated with the production `comparison`
				if(stringEquals(operator, ">=")) writeByteArray(&vm.code, OP_GREATER_EQUAL);
				if(stringEquals(operator, "<=")) writeByteArray(&vm.code, OP_LESS_EQUAL);
				if(stringEquals(operator, ">")) writeByteArray(&vm.code, OP_GREATER);
				if(stringEquals(operator, "<")) writeByteArray(&vm.code, OP_LESS);
		}
		return can_assign;
}

static bool term() {
		bool can_assign = factor();
		while(matchAndEatToken(TOKEN_PLUS) || matchAndEatToken(TOKEN_MINUS)) {
				can_assign = false;

				char *operator = parser.previous->lexeme;
				factor();

				// Actions associated with the production `term`
				if(stringEquals(operator, "+")) writeByteArray(&vm.code, OP_ADD);
				if(stringEquals(operator, "-")) writeByteArray(&vm.code, OP_SUBSTRACT);
		}
		return can_assign;
}

static bool factor() {
		bool can_assign = unary();
		while(matchAndEatToken(TOKEN_STAR) || matchAndEatToken(TOKEN_SLASH)) {
				can_assign = false;

				char *operator = parser.previous->lexeme;
				unary();

				// Actions associated with the production `factor`
				if(stringEquals(operator, "*")) writeByteArray(&vm.code, OP_MULTIPLY);
				if(stringEquals(operator, "/")) writeByteArray(&vm.code, OP_DIVIDE);
		}
		return can_assign;
}

static bool unary() {
		bool can_assign = true;
		while(matchAndEatToken(TOKEN_BANG) || matchAndEatToken(TOKEN_MINUS)) {
				can_assign = false;

				char *operator = parser.previous->lexeme;
				unary();

				// Actions associated with the production `unary`
				if(stringEquals(operator, "!")) writeByteArray(&vm.code, OP_NOT);
				if(stringEquals(operator, "-")) writeByteArray(&vm.code, OP_NEGATE);
		}
		return can_assign && primary();
}

static char *dynamicStrCpy(char *s) {
		// Account for the terminating byte by allocating `strlen(s) + 1` bytes
		// instead of `strlen(s)`.
		char *ret = malloc(strlen(s) + 1);
		CHECK(ret != NULL, "Failed to allocate memory");

		strcpy(ret, s);
		return ret;
}

static bool primary() {
		if(reachedEOF()) return false;

		// Actions associated with the terminal tokens.
		if(matchAndEatToken(TOKEN_LEFT_PAREN)) {
				expression();
				eatTokenOrReturnError(TOKEN_RIGHT_PAREN, "Expected ')' after the end of the expression");
				return false;
		}
		// We currently can't handle more than 255 values in our program because we only use one byte
		// to encode the position of our value in the value_array of the virtual machine.
		// This is why the variable `pos_on_value_array` is of type uint8_t.
		else if(matchAndEatToken(TOKEN_NUMBER)) {
				double number = strtod(parser.previous->lexeme, /*endPtr = */ NULL);
				WRITE_VALUE(CREATE_NUMBER, number);
				return false;
		}
		else if(matchAndEatToken(TOKEN_TRUE) || matchAndEatToken(TOKEN_FALSE)) {
				bool boolean = stringEquals(parser.previous->lexeme, "true");
				WRITE_VALUE(CREATE_BOOLEAN, boolean);
				return false;
		}
		else if(matchAndEatToken(TOKEN_STRING)) {
				// Create a string Value from a copy of the lexeme.
				// We copy the lexeme because our string_value will outlive the lexeme owned by Token.
			  char *copy_lexeme = dynamicStrCpy(parser.previous->lexeme);
				WRITE_VALUE(CREATE_STRING, copy_lexeme);
				return false;
		}
		else if(matchAndEatToken(TOKEN_NIL)) {
				WRITE_VALUE(CREATE_NIL);
				return false;
		}
		else if(matchAndEatToken(TOKEN_IDENTIFIER)) {
				// Check if equal is on the right hand side of the identifier.
				int idx = parser.current;
				while(idx < tokenizer.token_array.count && 
								tokenizer.token_array.array[idx].type != TOKEN_SEMICOLON)
				{
						if(tokenizer.token_array.array[idx].type == TOKEN_EQUAL) return true;
						idx++;
				}

				char *copy_lexeme = dynamicStrCpy(parser.previous->lexeme);
				writeByteArray(&vm.code, OP_GET);
				WRITE_VALUE(CREATE_STRING, copy_lexeme);
				return true;
		}
		else {
				// Expected an expression but found something else. We return an error.
				printf("%s\n", tokenizer.token_array.array[parser.current].lexeme);
				CHECK(/*condition = */false, "Unexpected token");
				return false;
		}
}

void parse() {
		while(!reachedEOF()) {
				declaration();
		}
}

static void declaration() {
		if(matchAndEatToken(TOKEN_VAR)) {
				varDeclaration();
		}
		else if(matchAndEatToken(TOKEN_FUN)) {
				funDeclaration();
		}
		else {
				statement();
		}
}

static void varDeclaration() {
		eatTokenOrReturnError(TOKEN_IDENTIFIER, "Expected Identifier after 'var'.");

		Local *local = &vm.locals[vm.local_top++];
		local->name = dynamicStrCpy(parser.previous->lexeme);
		local->scope = vm.scope;

		if(matchAndEatToken(TOKEN_EQUAL)) {
				expression();
		}
		else {
				WRITE_VALUE(CREATE_NIL);
		}

		eatTokenOrReturnError(TOKEN_SEMICOLON, "Expected ';' after var declaration");
}

static void funDeclaration() {

}

static void expressionStatement() {
		expression();
		writeByteArray(&vm.code, OP_POP);
		eatTokenOrReturnError(TOKEN_SEMICOLON, "Expected ';' at the end of the expression");
}

static int setCheckPoint(OpCode op_code) {
		writeByteArray(&vm.code, op_code);
		writeByteArray(&vm.code, 0xff);
		writeByteArray(&vm.code, 0xff);

		return vm.code.count - 3;
}

static void setJumpSize(int jump) {
		int correct_jump_size = vm.code.count - jump;

		vm.code.array[jump + 1] = (correct_jump_size >> 8) & 0xff;
		vm.code.array[jump + 2] = correct_jump_size & 0xff;
}

static void ifStatement() {
		eatTokenOrReturnError(TOKEN_LEFT_PAREN, "Expected '(' after 'if'");
		expression();
		eatTokenOrReturnError(TOKEN_RIGHT_PAREN, "Expected ')' after if expression");

		int jump_then = setCheckPoint(OP_JUMP_IF_FALSE);
		statement();
		int jump_else = setCheckPoint(OP_JUMP);

		setJumpSize(jump_then);

		if(matchAndEatToken(TOKEN_ELSE)) {
				statement();
		}
		setJumpSize(jump_else);

}

static void block() {
		vm.scope++;
		while(!reachedEOF() && !(peekToken()->type == TOKEN_RIGHT_BRACE)) {
				declaration();
		}
		vm.scope--;
		eatTokenOrReturnError(TOKEN_RIGHT_BRACE, "Expected '}' after the block");
}

static void statement() {
		if(matchAndEatToken(TOKEN_LEFT_BRACE)) {
				block();
		}
		else if(matchAndEatToken(TOKEN_IF)) {
				ifStatement();
		}
		else {
				expressionStatement();
		}
}
