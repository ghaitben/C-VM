#include "parser.h"
#include "error.h"
#include "value.h"
#include "debug.h"
#include "utility.h"
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
static bool or();
static bool call();
static void assignment();
static void declaration();
static void statement();
static void expressionStatement();
static void block();
static void varDeclaration();
static void funDeclaration();
static void ifStatement();
static bool matchAndEatToken(TokenType type);
static Token *eatTokenOrReturnError(TokenType type, const char *message);
static int setCheckPoint(OpCode op_code);
static void setJumpSize(int jump);
static bool reachedEOF();
static bool stringEquals();
static char *dynamicStrCpy(char *s);
static int resolveLocal(char *lexeme);

Parser parser;
Function *current_function;

void initParser(Parser *parser) {
		parser->previous = NULL;
		parser->current = 0;
}

void freeParser(Parser *parser) {
		initParser(parser);
}

// Checks whether the current token type matches the types passed in as a parameter.
// If the types match, it eats the token, otherwise it returns an error and exits the system.
static Token *eatTokenOrReturnError(TokenType type, const char *message) {
		CHECK(!reachedEOF() && peekToken()->type == type, message);
		return eatToken();
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
 *   + assignment        -->  or ( '=' ) assignment
 *
 *   + or                --> and 'or' assignment
 *
 *   + and               -->  equality 'and' assignment
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
		bool can_assign = or();
		
		if(!can_assign && peekToken()->type == TOKEN_EQUAL) {
				CHECK(false, "Invalid assignment target");
		}

		char *lexeme = parser.previous->lexeme;
		if(can_assign && matchAndEatToken(TOKEN_EQUAL)) {
				assignment();

				writeByteArray(&current_function->code, OP_ASSIGN);
				WRITE_VALUE(CREATE_NUMBER, resolveLocal(lexeme));
		}
}

static bool or() {
		bool can_assign = and();
		
		if(matchAndEatToken(TOKEN_OR)) {
				can_assign = false;

				int next_operand_jump = setCheckPoint(OP_JUMP_IF_FALSE);
				int exit_jump = setCheckPoint(OP_JUMP);
				setJumpSize(next_operand_jump);
				assignment();

				setJumpSize(exit_jump);
		}
		return can_assign;

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
				if(stringEquals(operator, "==")) writeByteArray(&current_function->code, OP_EQUAL_EQUAL);
				if(stringEquals(operator, "!=")) writeByteArray(&current_function->code, OP_BANG_EQUAL);
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
				if(stringEquals(operator, ">=")) writeByteArray(&current_function->code, OP_GREATER_EQUAL);
				if(stringEquals(operator, "<=")) writeByteArray(&current_function->code, OP_LESS_EQUAL);
				if(stringEquals(operator, ">")) writeByteArray(&current_function->code, OP_GREATER);
				if(stringEquals(operator, "<")) writeByteArray(&current_function->code, OP_LESS);
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
				if(stringEquals(operator, "+")) writeByteArray(&current_function->code, OP_ADD);
				if(stringEquals(operator, "-")) writeByteArray(&current_function->code, OP_SUBSTRACT);
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
				if(stringEquals(operator, "*")) writeByteArray(&current_function->code, OP_MULTIPLY);
				if(stringEquals(operator, "/")) writeByteArray(&current_function->code, OP_DIVIDE);
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
				if(stringEquals(operator, "!")) writeByteArray(&current_function->code, OP_NOT);
				if(stringEquals(operator, "-")) writeByteArray(&current_function->code, OP_NEGATE);
		}
		return can_assign && call();
}

static bool call() {
		bool can_assign = primary();
		if(!matchAndEatToken(TOKEN_LEFT_PAREN)) return can_assign;

		int arity = 0;
		do {
				if(peekToken()->type == TOKEN_RIGHT_PAREN) break;
				expression();
				arity++;
		} while(matchAndEatToken(TOKEN_COMMA));

		eatTokenOrReturnError(TOKEN_RIGHT_PAREN, "Expected ')' after the end of the call");
		writeByteArray(&current_function->code, OP_CALL);
		WRITE_VALUE(CREATE_NUMBER, arity);

		return false;
}

static char *dynamicStrCpy(char *s) {
		// Account for the terminating byte by allocating `strlen(s) + 1` bytes
		// instead of `strlen(s)`.
		char *ret = malloc(strlen(s) + 1);
		CHECK(ret != NULL, "Failed to allocate memory");

		strcpy(ret, s);
		return ret;
}

static int resolveLocal(char *lexeme) {
		for(int i = current_function->local_top - 1; i >= 0; --i) {
				if(strcmp(lexeme, current_function->locals[i].name)) continue;
				return i;
		}
		printf("%s\n", lexeme);
		CHECK(false, "undefined variable");
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
				if(peekToken()->type == TOKEN_EQUAL) return true;

				// Check for reflexive assignment
				for(int i = current_function->local_top - 1; i >= 0; --i) {
						if(strcmp(parser.previous->lexeme, current_function->locals[i].name)) continue;
						CHECK(current_function->locals[i].scope != -1, "Relfexive assignment is not allowed");
				}

				writeByteArray(&current_function->code, OP_GET);
				WRITE_VALUE(CREATE_NUMBER, resolveLocal(parser.previous->lexeme));
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
		current_function = createFunction("__main__");
		while(!reachedEOF()) {
				declaration();
		}
}

static void declaration() {
		if(matchAndEatToken(TOKEN_FUN)) {
				funDeclaration();
		}
		else if(matchAndEatToken(TOKEN_VAR)) {
				varDeclaration();
		}
		else {
				statement();
		}
}

static void varDeclaration() {
		eatTokenOrReturnError(TOKEN_IDENTIFIER, "Expected Identifier after 'var'.");

		// Check if a variable was already defined before.

		for(int i = current_function->local_top - 1; i >= 0 && current_function->locals[i].scope == vm.scope; --i) {
				if(strcmp(current_function->locals[i].name, parser.previous->lexeme)) continue;
				CHECK(false, "Variable already defined");
		}

		Local *local = &current_function->locals[current_function->local_top++];
		local->name = dynamicStrCpy(parser.previous->lexeme);
		// Mark as uninitialized
		local->scope = -1;

		if(matchAndEatToken(TOKEN_EQUAL)) {
				expression();
		}
		else {
				WRITE_VALUE(CREATE_NIL);
		}
		// Mark as initialized
		local->scope = vm.scope;

		eatTokenOrReturnError(TOKEN_SEMICOLON, "Expected ';' after var declaration");
}

static void funDeclaration() {
		// function name
		char *function_name = eatTokenOrReturnError(TOKEN_IDENTIFIER, 
						"Expected identifier after fun clause")->lexeme;
		Function *new_function = createFunction(function_name);
		Function *previous_function = current_function;
		current_function = new_function;

		// Open parenthesis
		eatTokenOrReturnError(TOKEN_LEFT_PAREN, "Expected '(' after function name");
		
		//arguments
		do {
				if(peekToken()->type == TOKEN_RIGHT_PAREN) break;

				Local *local = &current_function->locals[current_function->local_top++];
				local->name = dynamicStrCpy(eatTokenOrReturnError(TOKEN_IDENTIFIER,
								"Expected identifier")->lexeme);
				local->scope = vm.scope;
				current_function->arity++;

		}	while(matchAndEatToken(TOKEN_COMMA));

		eatTokenOrReturnError(TOKEN_RIGHT_PAREN, "Expected ')' after arguments");
		eatTokenOrReturnError(TOKEN_LEFT_BRACE, "Expected '{' at the beginning of fun declaration");

		// Function body.
		block();

		// Go back to the outer function once we are done parsing the inner one.
		current_function = previous_function;

		Local *local = &current_function->locals[current_function->local_top++];
		local->name = dynamicStrCpy(function_name);
		local->scope = vm.scope;

		WRITE_VALUE(CREATE_FUNCTION, new_function);
}

static void expressionStatement() {
		expression();
		writeByteArray(&current_function->code, OP_POP);
		eatTokenOrReturnError(TOKEN_SEMICOLON, "Expected ';' at the end of the expression");
}

static int setCheckPoint(OpCode op_code) {
		writeByteArray(&current_function->code, op_code);
		writeByteArray(&current_function->code, 0xff);
		writeByteArray(&current_function->code, 0xff);

		return current_function->code.count - 3;
}

static void setJumpSize(int jump) {
		int correct_jump_size = current_function->code.count - jump;

		current_function->code.array[jump + 1] = (correct_jump_size >> 8) & 0xff;
		current_function->code.array[jump + 2] = correct_jump_size & 0xff;
}

static void setBackWardJumpSize(int jump, int loop_start) {
		int correct_jump_size = current_function->code.count - loop_start - 3;

		current_function->code.array[jump + 1] = (correct_jump_size >> 8) & 0xff;
		current_function->code.array[jump + 2] = correct_jump_size & 0xff;
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

static void deleteOutOfScopeVariables() {
		while(current_function->local_top > 0 && 
						current_function->locals[current_function->local_top - 1].scope > vm.scope)
		{
				writeByteArray(&current_function->code, OP_POP);
				current_function->local_top--;
				free(current_function->locals[current_function->local_top].name);
		}
}

static void block() {
		vm.scope++;
		while(!reachedEOF() && !(peekToken()->type == TOKEN_RIGHT_BRACE)) {
				declaration();
		}
		vm.scope--;
		deleteOutOfScopeVariables();
		eatTokenOrReturnError(TOKEN_RIGHT_BRACE, "Expected '}' after the block");
}

static void whileStatement() {
		eatTokenOrReturnError(TOKEN_LEFT_PAREN, "Expected '(' after 'while'");
		int loop_start = current_function->code.count;
		expression();
		eatTokenOrReturnError(TOKEN_RIGHT_PAREN, "Expected ')' after while expression");

		int exit_jump = setCheckPoint(OP_JUMP_IF_FALSE);
		statement();
		int go_back = setCheckPoint(OP_JUMP_BACKWARD);

		setJumpSize(exit_jump);
		setBackWardJumpSize(go_back, loop_start);
}

static void forStatement() {
		vm.scope++;
		eatTokenOrReturnError(TOKEN_LEFT_PAREN, "Expected '(' after for statement");

		// Init
		if(matchAndEatToken(TOKEN_VAR)) {
				varDeclaration();
		}
		else if(matchAndEatToken(TOKEN_SEMICOLON)) {
				// no initializer
		}
		else {
				expressionStatement();
		}

		int condition_index = current_function->code.count;
		// Condition
		expression();
		eatTokenOrReturnError(TOKEN_SEMICOLON, "Expected ';' after the condition expression");
		
		int jump_out_body = setCheckPoint(OP_JUMP_IF_FALSE);
		int jump_to_body = setCheckPoint(OP_JUMP);

		int increment_index = current_function->code.count;
		// increment
		expression();
		eatTokenOrReturnError(TOKEN_RIGHT_PAREN, "Expected ')' after the end of the for loop");

		int check_condition_idx = setCheckPoint(OP_JUMP_BACKWARD);
		setBackWardJumpSize(check_condition_idx, condition_index);

		setJumpSize(jump_to_body);
		// for loop body
		statement();

		int go_back_to_idx = setCheckPoint(OP_JUMP_BACKWARD);
		setBackWardJumpSize(go_back_to_idx, increment_index);

		vm.scope--;
		deleteOutOfScopeVariables();
		setJumpSize(jump_out_body);
}

static void printStatement() {
		expression();
		writeByteArray(&current_function->code, OP_PRINT);
		eatTokenOrReturnError(TOKEN_SEMICOLON, "Expected ';' after the end of a print statement");
}

static void statement() {
		if(matchAndEatToken(TOKEN_LEFT_BRACE)) {
				block();
		}
		else if(matchAndEatToken(TOKEN_IF)) {
				ifStatement();
		}
		else if(matchAndEatToken(TOKEN_WHILE)) {
				whileStatement();
		}
		else if(matchAndEatToken(TOKEN_FOR)) {
				forStatement();
		}
		else if(matchAndEatToken(TOKEN_PRINT)) {
				printStatement();
		}
		else {
				expressionStatement();
		}
}
