#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "scanner.h"

#define NKEYS (sizeof KEYWORDS / sizeof(KEYWORDS[0]))

// Keep it ordered
Keyword KEYWORDS[] = {
    { "and", TOKEN_AND },
    { "class", TOKEN_CLASS },
    { "else", TOKEN_ELSE },
    { "false", TOKEN_FALSE },
    { "for", TOKEN_FOR },
    { "fun", TOKEN_FUN },
    { "if", TOKEN_IF },
    { "nil", TOKEN_NIL },
    { "or", TOKEN_OR },
    { "print", TOKEN_PRINT },
    { "return", TOKEN_RETURN },
    { "super", TOKEN_SUPER },
    { "this", TOKEN_THIS },
    { "true", TOKEN_TRUE },
    { "var", TOKEN_VAR },
    { "while", TOKEN_WHILE },
};

Token *get_token(int c, FILE *s);
Token *new_str_token(FILE *ifp);
Token *new_num_token(FILE *ifp);
Token *new_id_token(FILE *ifp);
Keyword *binsearch(char *kwrd, Keyword *tab, int n);


int scan(FILE *input, Token **tokens)
{
    int n;
    char c, next_c;

    n = 0;
    while ((c = getc(input)) != EOF) {
        if (input == stdin && c == '\n')
            return n;

        if (isspace(c))
            continue;

        if (c == '/') {
            if ((next_c = getc(input)) == '/') {
                while (getc(input) != '\n')
                    ;
                continue;
            } else {
                ungetc(next_c, input);
            }
        }

        tokens[n++] = get_token(c, input);
    }

    return n;
}


Token *get_token(int c, FILE *ifp)
{
    int next_c;

    switch(c) {
        case '(':
            return new_token(TOKEN_LEFT_PAREN, "(", 1);
        case ')':
            return new_token(TOKEN_RIGHT_PAREN, ")", 1);
        case '{':
            return new_token(TOKEN_LEFT_BRACE, "{", 1);
        case '}':
            return new_token(TOKEN_RIGHT_BRACE, "}", 1);
        case ',':
            return new_token(TOKEN_COMMA, ",", 1);
        case '.':
            return new_token(TOKEN_DOT, ".", 1);
        case ';':
            return new_token(TOKEN_SEMICOLON, ";", 1);
        case '-':
            return new_token(TOKEN_MINUS, "-", 1);
        case '+':
            return new_token(TOKEN_PLUS, "+", 1);
        case '*':
            return new_token(TOKEN_STAR, "*", 1);
        case '/':
            return new_token(TOKEN_SLASH, "/", 1);
        case '!':
            if ((next_c = getc(ifp)) == '=')
                return new_token(TOKEN_BANG_EQUAL, "!=", 2);
            else {
                ungetc(next_c, ifp);
                return new_token(TOKEN_BANG, "!", 1);
            }
        case '=':
            if ((next_c = getc(ifp)) == '=')
                return new_token(TOKEN_EQUAL_EQUAL, "==", 2);
            else {
                ungetc(next_c, ifp);
                return new_token(TOKEN_EQUAL, "=", 1);
            }
        case '<':
            if ((next_c = getc(ifp)) == '=')
                return new_token(TOKEN_LESS_EQUAL, "<=", 2);
            else {
                ungetc(next_c, ifp);
                return new_token(TOKEN_LESS, "<", 1);
            }
        case '>':
            if ((next_c = getc(ifp)) == '=')
                return new_token(TOKEN_GREATER_EQUAL, ">=", 2);
            else {
                ungetc(next_c, ifp);
                return new_token(TOKEN_GREATER, ">", 1);
            }
        case '"':
            return new_str_token(ifp);
        default:
            if (isdigit(c)) {
                ungetc(c, ifp);
                return new_num_token(ifp);
            } else if (isalpha(c) || c == '_') {
                ungetc(c, ifp);
                return new_id_token(ifp);
            }
            return new_token(TOKEN_ERROR, NULL, 0);
   }
}


Token *new_token(int type, char *lexeme, int len)
{
    Token *token = (Token *) malloc(sizeof(Token));
    
    token->type = type;
    token->lexeme = lexeme;
    token->lineno = 1;
    token->len = len;

    return token;
}


Token *new_str_token(FILE *ifp)
{
    int c, len;
    char s[128];
    char *p = s;

    len = 0;
    while ((*p++ = c = getc(ifp)) != EOF && c != '"')
        len++;
    *--p = '\0';

    if (c != '"')
        return new_token(TOKEN_ERROR, NULL, 0);

    return new_token(TOKEN_STRING, strdup(s), len);
}


Token *new_num_token(FILE *ifp)
{
    int c;
    char s[128];
    char *p = s;

    while ((c = getc(ifp)) != EOF && c != '\n' && (isdigit(c) || c == '.'))
        *p++ = c;
    *p = '\0';

    if (c != EOF) {
        ungetc(c, ifp);    
    }

    return new_token(TOKEN_NUMBER, strdup(s), strlen(s));
}


Token *new_id_token(FILE *ifp)
{
    int c;
    char s[128];
    char *p = s;

    while ((c = getc(ifp)) != EOF && c != '\n' && ((isalnum(c)) || c == '_')) {
        *p++ = c;
    }
    *p = '\0';

    if (c != EOF) {
        ungetc(c, ifp);
    }

    Keyword *kwrd = binsearch(s, KEYWORDS, NKEYS);
    if (kwrd != NULL)
        return new_token(kwrd->type, strdup(s), strlen(s));

    return new_token(TOKEN_IDENTIFIER, strdup(s), strlen(s));
}


Keyword *binsearch(char *kwrd, Keyword *tab, int n)
{
    int cond;
    Keyword *low = &tab[0];
    Keyword *high = &tab[n]; 
    Keyword *mid;

    while (low < high) {
        mid = low + (high - low) / 2;
        if ((cond = strcmp(kwrd, mid->name)) < 0)
            high = mid; 
        else if (cond > 0)
            low = mid + 1;
        else
            return mid;
    }

    return NULL;
}
