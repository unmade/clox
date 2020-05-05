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

Token *gettoken(int c, FILE *s);
Token *makestrtoken(FILE *ifp);
Token *makenumtoken(FILE *ifp);
Token *makeidtoken(FILE *ifp);
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

        tokens[n++] = gettoken(c, input);
    }

    return n;
}


Token *gettoken(int c, FILE *ifp)
{
    int next_c;

    switch(c) {
        case '(':
            return maketoken(TOKEN_LEFT_PAREN, 1, "(");
        case ')':
            return maketoken(TOKEN_RIGHT_PAREN, 1, ")");
        case '{':
            return maketoken(TOKEN_LEFT_BRACE, 1, "{");
        case '}':
            return maketoken(TOKEN_RIGHT_BRACE, 1, "}");
        case ',':
            return maketoken(TOKEN_COMMA, 1, ",");
        case '.':
            return maketoken(TOKEN_DOT, 1, ".");
        case ';':
            return maketoken(TOKEN_SEMICOLON, 1, ";");
        case '-':
            return maketoken(TOKEN_MINUS, 1, "-");
        case '+':
            return maketoken(TOKEN_PLUS, 1, "+");
        case '*':
            return maketoken(TOKEN_STAR, 1, "*");
        case '/':
            return maketoken(TOKEN_SLASH, 1, "/");
        case '!':
            if ((next_c = getc(ifp)) == '=')
                return maketoken(TOKEN_BANG_EQUAL, 2, "!=");
            else {
                ungetc(next_c, ifp);
                return maketoken(TOKEN_BANG, 1, "!");
            }
        case '=':
            if ((next_c = getc(ifp)) == '=')
                return maketoken(TOKEN_EQUAL_EQUAL, 2, "==");
            else {
                ungetc(next_c, ifp);
                return maketoken(TOKEN_EQUAL, 1, "=");
            }
        case '<':
            if ((next_c = getc(ifp)) == '=')
                return maketoken(TOKEN_LESS_EQUAL, 2, "<=");
            else {
                ungetc(next_c, ifp);
                return maketoken(TOKEN_LESS, 1, "<");
            }
        case '>':
            if ((next_c = getc(ifp)) == '=')
                return maketoken(TOKEN_GREATER_EQUAL, 2, ">=");
            else {
                ungetc(next_c, ifp);
                return maketoken(TOKEN_GREATER, 1, ">");
            }
        case '"':
            return makestrtoken(ifp);
        default:
            if (isdigit(c)) {
                ungetc(c, ifp);
                return makenumtoken(ifp);
            } else if (isalpha(c) || c == '_') {
                ungetc(c, ifp);
                return makeidtoken(ifp);
            }
            return maketoken(TOKEN_ERROR, 0, NULL);
   }
}


Token *maketoken(int type, int length, char *repr)
{
    Token *token = (Token *) malloc(sizeof(Token));
    
    token->type = type;
    token->repr = repr;
    token->lineno = 1;
    token->length = length;

    return token;
}


Token *makestrtoken(FILE *ifp)
{
    int c, len;
    char s[128];
    char *p = s;

    len = 0;
    while ((*p++ = c = getc(ifp)) != EOF && c != '"')
        len++;
    *p = '\0';

    if (c != '"')
        return maketoken(TOKEN_ERROR, 0, NULL);

    if (c != EOF && c != '"')
        ungetc(c, ifp);

    return maketoken(TOKEN_STRING, len, strdup(s));
}


Token *makenumtoken(FILE *ifp)
{
    int c;
    char s[128];
    char *p = s;

    while ((c = getc(ifp)) != EOF && c != '\n' && (isdigit(c) || c == '.')) {
        *p++ = c;
    }
    *p = '\0';

    if (c != EOF) {
        ungetc(c, ifp);    
    }

    return maketoken(TOKEN_NUMBER, 0, strdup(s));
}


Token *makeidtoken(FILE *ifp)
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
        return maketoken(kwrd->type, 0, strdup(s));

    return maketoken(TOKEN_IDENTIFIER, 0, strdup(s));
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
