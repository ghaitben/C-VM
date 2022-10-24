#include "tokenizer.c"
#include "error.h"

void PushToken(TokenType type, char *lexeme, TokenizerArray *arr) {
		Token token;

		// Ownership of lex is going to be transfered to Token.
		// Token will be responsible for freeing lex.
		char *lex = malloc(strlen(lexeme) + 1);
		strcpy(lex, lexeme);
		lex[strlen(lexeme)] = '\0';
		initToken(&token, type, lex, /*line = */-1);

		TokenizerArray_push(arr, token);
}

bool test00() {
		bool result = true;
		initTokenizer(&tokenizer);
		tokenize("../test_data/tokenizer_test00.txt");

		TokenizerArray true_array;
		initTokenizerArray(&true_array);

		PushToken(TOKEN_VAR, "var", &true_array);
		PushToken(TOKEN_IDENTIFIER, "a", &true_array);
		PushToken(TOKEN_EQUAL, "=", &true_array);
		PushToken(TOKEN_STRING, "\"test01\"", &true_array);
		PushToken(TOKEN_SEMICOLON, ";", &true_array);
		PushToken(TOKEN_IF, "if", &true_array);
		PushToken(TOKEN_LEFT_PAREN, "(", &true_array);
		PushToken(TOKEN_IDENTIFIER, "a", &true_array);
		PushToken(TOKEN_EQUAL_EQUAL, "==", &true_array);
		PushToken(TOKEN_STRING, "\"test01\"", &true_array);
		PushToken(TOKEN_RIGHT_PAREN, ")", &true_array);
		PushToken(TOKEN_PRINT, "print", &true_array);
		PushToken(TOKEN_STRING, "\"test02\"", &true_array);
		PushToken(TOKEN_SEMICOLON, ";", &true_array);
		PushToken(TOKEN_EOF, "\0", &true_array);

		TokenizerArray arr = tokenizer.token_array;
		for(int i = 0; i < arr.count; ++i) {
				result = result && tokenEquals(&arr.array[i], &true_array.array[i]);
		}

		freeTokenizerArray(&true_array);
		freeTokenizer(&tokenizer);
		return result;
}

int main(int argc, char **argv) {
		CHECK(test00(), "Failed test00");	
		printf("Tests Suceeded!\n");
}
