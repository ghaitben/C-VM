typedef enum TokenType TokenType;
typedef struct Token Token;
typedef struct Tokenizer Tokenizer;
typedef struct TokenizerArray TokenizerArray;

enum TokenType {
  TOKEN_LEFT_PAREN,
	TOKEN_RIGHT_PAREN,
  TOKEN_LEFT_BRACE,  
	TOKEN_RIGHT_BRACE, 
  TOKEN_COMMA,      
	TOKEN_DOT,
	TOKEN_MINUS,
	TOKEN_PLUS,
  TOKEN_SEMICOLON,
	TOKEN_SLASH,
	TOKEN_STAR,
  TOKEN_BANG,
	TOKEN_BANG_EQUAL,
  TOKEN_EQUAL,
	TOKEN_EQUAL_EQUAL,
  TOKEN_GREATER,
	TOKEN_GREATER_EQUAL,
  TOKEN_LESS,
	TOKEN_LESS_EQUAL,
  TOKEN_IDENTIFIER,
	TOKEN_STRING,
	TOKEN_NUMBER,
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
	TOKEN_WHILE,
	TOKEN_UNINITIALIZED,
	TOKEN_EOF
};

struct Token {
		/*
		 * String representation of the token.
		 * Token owns the memory of the field:
		 *    + lexeme
		 * */
		char *lexeme;
		TokenType type;
		int line;
};
void initToken(Token *token);
void freeToken(Token *token);

struct TokenizerArray {
		int count;
		int capacity;
		Token *array;
};
void initTokenizerArray(TokenizerArray *tokenizer_array);
void freeTokenizerArray(TokenizerArray *tokenizer_array);
void TokenizerArray_push(TokenizerArray *tokenizer_array, Token token);

/*
 * There will be one global instance of this struct.
 * This will help keep track of our state (i.e src file, start column, current column, and the current line).
 * The global instance of this struct will remain in memory throughout the whole compilation process.
 * 
 * Tokenizer owns the memory of the fields:
 *    + token_array
 *    + source_file
 * */
struct Tokenizer {
		char *source_file;
		TokenizerArray token_array;
		int start;
		int current;
		int line;
};
void initTokenizer(Tokenizer *tokenizer);
void freeTokenizer(Tokenizer *tokenizer);


/* 
 * Loads the source file to memory and saves it to the tokenizer.source_file field.
 * Creates tokens from the source file and saves them to tokenizer.token_array.
 * */
void tokenize(const char *filepath);

extern Tokenizer tokenizer;
