#include "tokenizer.h"
#include "error.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/* 
 * Forward declarations of all the static functions in the file.
 * There will be no duplicate declarations since all of the static functions are restricted to 
 * this specific file.
 * */
static char *readFile(const char *source_file);
static void addToken();
static bool reachedEOF();
static char eatChar();
static char peekChar();
static char *sliceString(int start, int end);
static void skipWhiteSpace();
static bool matchAndEatChar(char c);
static void createAndPushToken(TokenType type);
static void createAndPushIdentifier();
static void createAndPushDigit();
static void createAndPushString();
static bool isDigit(char c);
static bool isAlpha(char c);

Tokenizer tokenizer;
char *keywords[] = {
		"and",
		"class",
		"else",
		"false",
		"for",
		"fun",
		"if",
		"nil",
		"or",
		"print",
		"return",
		"super",
		"this",
		"true",
		"var",
		"while"
};

TokenType token_of_keyword[] = {
  TOKEN_AND,
	TOKEN_CLASS,
	TOKEN_ELSE,
	TOKEN_FALSE,
  TOKEN_FOR,
	TOKEN_FUN,
	TOKEN_IF,
	TOKEN_NIL,
	TOKEN_OR,
  TOKEN_PRINT, 
	TOKEN_RETURN,
	TOKEN_SUPER,
	TOKEN_THIS,
  TOKEN_TRUE,
	TOKEN_VAR,
	TOKEN_WHILE
};

void initToken(Token *token, TokenType type, char *lexeme, int line) {
		token->type = type;
		token->lexeme = lexeme;
		token->line = line;
}

void freeToken(Token *token) {
		free(token->lexeme);
		initToken(token, /*type = */TOKEN_UNINITIALIZED,/*lexeme = */ NULL,/*line = */ -1);
}

bool tokenEquals(Token *token_a, Token *token_b) {
		// if one of the tokens is TOKEN_EOF there is not need to compare the lexemes.
		if(token_a->type == TOKEN_EOF || token_b->type == TOKEN_EOF) {
				return token_a->type == TOKEN_EOF && token_b->type == TOKEN_EOF;
		}

		return strcmp(token_a->lexeme, token_b->lexeme) == 0 && 
				token_a->type == token_b->type;
}

void initTokenizer(Tokenizer *tokenizer) {
		tokenizer->start = 0;
		tokenizer->current = 0;
		tokenizer->line = 1;
		tokenizer->source_file = NULL;
		initTokenizerArray(&tokenizer->token_array);
}

void freeTokenizer(Tokenizer *tokenizer) {
		freeTokenizerArray(&tokenizer->token_array);
		free(tokenizer->source_file);
		initTokenizer(tokenizer);
}

void initTokenizerArray(TokenizerArray *tokenizer_array) {
		tokenizer_array->array = NULL;
		tokenizer_array->count = 0;
		tokenizer_array->capacity = 0;
}

void freeTokenizerArray(TokenizerArray *tokenizer_array) {
		for(int i = 0; i < tokenizer_array->count; ++i) {
				freeToken(&tokenizer_array->array[i]);
		}
		free(tokenizer_array->array);
		initTokenizerArray(tokenizer_array);
}

void TokenizerArray_push(TokenizerArray *tokenizer_array, Token token) {
		if(tokenizer_array->count + 1 > tokenizer_array->capacity) {
				tokenizer_array->capacity = tokenizer_array->capacity > 0 ? 2 * tokenizer_array->capacity : 8;
				tokenizer_array->array = (Token *) realloc(tokenizer_array->array,
								sizeof(Token) * tokenizer_array->capacity);
				CHECK(tokenizer_array->array != NULL, "Failed allocating memory");
		}

		tokenizer_array->array[tokenizer_array->count] = token;
		tokenizer_array->count++;
}

// Assumes that the global instance of the tokenizer has been initialized.
void tokenize(const char *filepath) {
		tokenizer.source_file = readFile(filepath);

		while(!reachedEOF()) {
				addToken();
		}
}

static bool reachedEOF() {
		return tokenizer.current >= strlen(tokenizer.source_file);
}

static char *sliceString(int start, int end) {
		int string_size = end - start;

		char *buffer = malloc(string_size + 1);
		CHECK(buffer != NULL, "Failed to allocate memory");

		memcpy(buffer, tokenizer.source_file + tokenizer.start, string_size);

		buffer[string_size] = '\0';
		return buffer;
}

/*
 * This is the function where all the tokens are created and pushed to 
 * the array of tokens of the global tokenizer.
 * */
static void createAndPushToken(TokenType type) {
		Token token;
		token.line = tokenizer.line;
		token.lexeme = sliceString(tokenizer.start, tokenizer.current);
		token.type = type;
		TokenizerArray_push(&tokenizer.token_array, token);
}

static void skipWhiteSpace() {
		while(!reachedEOF() && isspace(peekChar())) {
				tokenizer.line += eatChar() == '\n';
		}
}

static void createAndPushIdentifier() {
		while(!reachedEOF() && (isAlpha(peekChar()) || isDigit(peekChar()))) eatChar();
		
		// Compares the lexeme (char *) with all the reserved keywords and returns the corresponding TokenType
		// if there is any match. It returns TOKEN_IDENTIFIER otherwise (If there is no match).
		size_t keywords_size = sizeof(keywords) / sizeof(keywords[0]);
		int lexeme_size = tokenizer.current - tokenizer.start;
		for(int i = 0; i < keywords_size; ++i) {
				bool same_size = lexeme_size == strlen(keywords[i]);
				bool same_string = memcmp(tokenizer.source_file + tokenizer.start, 
								keywords[i], lexeme_size) == 0;

				if(same_size && same_string) {
						createAndPushToken(token_of_keyword[i]);
						return;
				}
		}
		createAndPushToken(TOKEN_IDENTIFIER);
}

static void createAndPushDigit() {
		while(!reachedEOF() && isDigit(peekChar())) eatChar();

		if(matchAndEatChar('.')) {
				while(!reachedEOF() && isDigit(peekChar())) eatChar();
		}
		
		createAndPushToken(TOKEN_NUMBER);
}

static void createAndPushString() {
		while(!reachedEOF() && peekChar() != '"') eatChar();
		CHECK(!reachedEOF(), "Expected closing '\"' for a string literal");

		// Eat the starting '"' of the string.
		tokenizer.start++;
		createAndPushToken(TOKEN_STRING);

		// Eat the enclosing '"' of the string literal.
		eatChar();
}

static bool isAlpha(char c) {
		return (c >= 'a' && c <= 'z') || 
				  (c >= 'A' && c <= 'Z') ||
					c == '_';
}

static bool isDigit(char c) {
		return c >= '0' && c <= '9';
}

static void addToken() {
		skipWhiteSpace();
		tokenizer.start = tokenizer.current;
		
		char c = eatChar();
		switch(c) {
				case '(':
						createAndPushToken(TOKEN_LEFT_PAREN);
						break;
				case ')':
						createAndPushToken(TOKEN_RIGHT_PAREN);
						break;
				case '*':
						createAndPushToken(TOKEN_STAR);
						break;
				case '{':
						createAndPushToken(TOKEN_LEFT_BRACE);
						break;
				case '}':
						createAndPushToken(TOKEN_RIGHT_BRACE);
						break;
				case '.':
						createAndPushToken(TOKEN_DOT);
						break;
				case ',':
						createAndPushToken(TOKEN_COMMA);
						break;
				case ';':
						createAndPushToken(TOKEN_SEMICOLON);
						break;
				case '+':
						createAndPushToken(TOKEN_PLUS);
						break;
				case '-':
						createAndPushToken(TOKEN_MINUS);
						break;
				case '=':
						if(matchAndEatChar('=')) createAndPushToken(TOKEN_EQUAL_EQUAL);
						else createAndPushToken(TOKEN_EQUAL);
						break;
				case '/':
						createAndPushToken(TOKEN_SLASH);
						break;
				case '<':
						if(matchAndEatChar('=')) createAndPushToken(TOKEN_LESS_EQUAL);
						else createAndPushToken(TOKEN_LESS);
						break;
				case '>':
						if(matchAndEatChar('=')) createAndPushToken(TOKEN_GREATER_EQUAL);
						else createAndPushToken(TOKEN_GREATER);
						break;
				case '!':
						if(matchAndEatChar('=')) createAndPushToken(TOKEN_BANG_EQUAL);
						else createAndPushToken(TOKEN_BANG);
						break;
				case '"':
						createAndPushString();
						break;
				case '\0':
						createAndPushToken(TOKEN_EOF);
						break;

				default:
						if(isAlpha(c)) {
								createAndPushIdentifier();
						}
						else if(isDigit(c)) {
								createAndPushDigit();
						}
						else {
								// Error: Unknown Token
								CHECK(false, "Uknown Token.");
						}
		}
}

static bool matchAndEatChar(char c) {
		if(reachedEOF() || peekChar() != c) return false;
		eatChar();
		return true;
}

static char peekChar() {
		return tokenizer.source_file[tokenizer.current];
}

static char eatChar() {
		tokenizer.current++;
		return tokenizer.source_file[tokenizer.current - 1];
}

static char *readFile(const char *filepath) {
		FILE *file = fopen(filepath, "r");
		CHECK(file != NULL, "Failed to open the file");

		// Get the size of the source file
		fseek(file, /*offset = */ 0L, SEEK_END);
		long file_size = ftell(file);
		rewind(file);

		char *buffer = malloc(file_size + 1);
		CHECK(buffer != NULL, "Failed to allocate memory.");

		// Read the contents of the file into the buffer.
		size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
		CHECK(bytes_read == file_size, "Failed to read the source file");

		buffer[file_size] = '\0';
		fclose(file);
		return buffer;
}
