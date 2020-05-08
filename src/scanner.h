#ifndef clox_scanner_h 
#define clox_scanner_h 

#include <stdio.h>

#define MAXTOKEN 100

typedef enum {
    TOKEN_LEFT_PAREN = 0,
    TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_SEMICOLON,
    TOKEN_MINUS,
    TOKEN_PLUS,
    TOKEN_STAR,
    TOKEN_BANG_EQUAL,
    TOKEN_BANG,
    TOKEN_SLASH,
    TOKEN_EQUAL_EQUAL,
    TOKEN_EQUAL,
    TOKEN_LESS_EQUAL,
    TOKEN_LESS,
    TOKEN_GREATER_EQUAL,
    TOKEN_GREATER,
    TOKEN_STRING,
    TOKEN_NUMBER,
    TOKEN_IDENTIFIER,
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
    TOKEN_ERROR,
} TokenType;


typedef struct {
    char *name;
    TokenType type;
} Keyword;


typedef struct token {
    struct token *next;
    struct token *prev;
    TokenType type;
    char *lexeme;
    int lineno;
} Token;


Token *scan(FILE *input);

#endif
