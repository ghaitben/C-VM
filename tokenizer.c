#include "tokenizer.h"
#include "error.h"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

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
		tokenizer->col_start = 0;
		tokenizer->col_current = 0;
		tokenizer->line = 1;
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

Token *tokenize(const char *source_file) {
}
