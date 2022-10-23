#include "tokenizer.h"
#include "error.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
static void pushToken(TokenType type);
static char *sliceString(int start, int end);

Tokenizer tokenizer;

void initToken(Token *token) {
		token->lexeme = NULL;
		token->type = TOKEN_UNINITIALIZED;
		token->line = -1;
}

void freeToken(Token *token) {
		free(token->lexeme);
		initToken(token);
}

void initTokenizer(Tokenizer *tokenizer) {
		tokenizer->start = 0;
		tokenizer->current = 0;
		tokenizer->line = 1;
		tokenizer->source_file = NULL;
		initTokenizerArray(&tokenizer->token_array);
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
				tokenizer_array->capacity *= 2;
				tokenizer_array->array = (Token *) realloc(tokenizer_array, tokenizer_array->capacity);
				CHECK(tokenizer_array->array != NULL, "Failed allocating memory");
		}

		tokenizer_array->array[tokenizer_array->count] = token;
		tokenizer_array->count++;
}

// Assumes that the global instance of the tokenizer has been initialized.
void tokenize(const char *filepath) {
		tokenizer.source_file = readFile(filepath);

		while(!reachedEOF()) {
				tokenizer.start = tokenizer.current;
				addToken();
		}
}

static bool reachedEOF() {
		return tokenizer.current >= strlen(tokenizer.source_file);
}

static char *sliceString(int start, int end) {
		int string_size = end - start;

		char *buffer = malloc(string_size);
		CHECK(buffer != NULL, "Failed to allocate memory");

		memcpy(buffer, tokenizer.source_file + tokenizer.start, string_size);

		buffer[string_size] = '\0';
		return buffer;
}

static void pushToken(TokenType type) {
		Token token;
		token.type = type;
		token.line = tokenizer.line;
		token.lexeme = sliceString(tokenizer.start, tokenizer.current);
		TokenizerArray_push(&tokenizer.token_array, token);
}

static void addToken() {
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
		return buffer;
}
