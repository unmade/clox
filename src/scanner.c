#include <ctype.h>
#include "stdbool.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "logger.h"
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

static Token *new_token(int type, char *lexeme);
static void free_token(Token *token);

static Token *get_token(int c, FILE *s);
static Token *new_str_token(FILE *ifp);
static Token *new_num_token(FILE *ifp);
static Token *new_id_token(FILE *ifp);
static char *read_while(FILE *ifp, bool (*cond)(char c));
static Keyword *binsearch(char *kwrd, Keyword *tab, int n);


Token *scan(FILE *input)
{
    char c, next_c;
    bool is_stdin;
    Token *token, *first;

    is_stdin = (input == stdin);

    first = token = NULL;
    while ((c = getc(input)) != EOF) {
        if (is_stdin && c == '\n')
            break;

        if (isspace(c))
            continue;

        if (c == '/') {
            if ((next_c = getc(input)) == '/') {
                while ((c = getc(input)) != '\n' && c != EOF)
                    ;
                continue;
            } else {
                ungetc(next_c, input);
            }
        }

        if (token == NULL) {
            first = token = get_token(c, input);
        } else {
            token->next = get_token(c, input);
            token = token->next;
        }
    }

    for (token = first; token != NULL; token = token->next)
        if (token->type == TOKEN_ERROR) {
            free_tokens(first);
            return NULL;
        }

    return first;
}


void free_tokens(Token *tokens)
{
    Token *token, *next;

    for (token = tokens; token != NULL; token = next) {
        next = token->next;
        free_token(token);
    }
}


static Token *get_token(int c, FILE *ifp)
{
    int next_c;

    switch(c) {
        case '(':
            return new_token(TOKEN_LEFT_PAREN, "(");
        case ')':
            return new_token(TOKEN_RIGHT_PAREN, ")");
        case '{':
            return new_token(TOKEN_LEFT_BRACE, "{");
        case '}':
            return new_token(TOKEN_RIGHT_BRACE, "}");
        case ',':
            return new_token(TOKEN_COMMA, ",");
        case '.':
            return new_token(TOKEN_DOT, ".");
        case ';':
            return new_token(TOKEN_SEMICOLON, ";");
        case '-':
            return new_token(TOKEN_MINUS, "-");
        case '+':
            return new_token(TOKEN_PLUS, "+");
        case '*':
            return new_token(TOKEN_STAR, "*");
        case '/':
            return new_token(TOKEN_SLASH, "/");
        case '!':
            if ((next_c = getc(ifp)) == '=')
                return new_token(TOKEN_BANG_EQUAL, "!=");
            else {
                ungetc(next_c, ifp);
                return new_token(TOKEN_BANG, "!");
            }
        case '=':
            if ((next_c = getc(ifp)) == '=')
                return new_token(TOKEN_EQUAL_EQUAL, "==");
            else {
                ungetc(next_c, ifp);
                return new_token(TOKEN_EQUAL, "=");
            }
        case '<':
            if ((next_c = getc(ifp)) == '=')
                return new_token(TOKEN_LESS_EQUAL, "<=");
            else {
                ungetc(next_c, ifp);
                return new_token(TOKEN_LESS, "<");
            }
        case '>':
            if ((next_c = getc(ifp)) == '=')
                return new_token(TOKEN_GREATER_EQUAL, ">=");
            else {
                ungetc(next_c, ifp);
                return new_token(TOKEN_GREATER, ">");
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
            log_error(LOX_SYNTAX_ERR, "unexpected character: '%c'.", c);
            return new_token(TOKEN_ERROR, NULL);
   }
}


static Token *new_token(int type, char *lexeme)
{
    Token *token = (Token *) malloc(sizeof(Token));
    
    token->next = NULL;

    token->type = type;
    token->lexeme = lexeme;
    token->lineno = 1;

    return token;
}


static void free_token(Token *token)
{
    switch (token->type) {
        case TOKEN_NUMBER:
        case TOKEN_STRING:
        case TOKEN_IDENTIFIER:
        case TOKEN_ERROR:
            if (token->lexeme != NULL)
                free(token->lexeme);
            break;
        default:
            if ((binsearch(token->lexeme, KEYWORDS, NKEYS)) != NULL) 
                free(token->lexeme);
            break;
    }
 
    free(token);
}


static bool is_char(char c)       { return (c != '"'); }
static bool is_num(char c)        { return (c != '\n' && (isdigit(c) || c == '.')); }
static bool is_id_or_kwrd(char c) { return (c != '\n' && ((isalnum(c)) || c == '_')); }


static Token *new_str_token(FILE *ifp)
{
    char c;
    char *s;

    s = read_while(ifp, is_char);

    if ((c = getc(ifp)) != '"') {
        log_error(LOX_SYNTAX_ERR, "EOF while scanning string literal.");
        return new_token(TOKEN_ERROR, s);
    }

    return new_token(TOKEN_STRING, s);
}


static Token *new_num_token(FILE *ifp)
{
    char *s;

    s = read_while(ifp, is_num);

    return new_token(TOKEN_NUMBER, s);
}


static Token *new_id_token(FILE *ifp)
{
    char *s;
    Keyword *kwrd;

    s = read_while(ifp, is_id_or_kwrd);

    if ((kwrd = binsearch(s, KEYWORDS, NKEYS)) != NULL)
        return new_token(kwrd->type, s);
    
    return new_token(TOKEN_IDENTIFIER, s);
}


static char *read_while(FILE *ifp, bool (*cond)(char c))
{ 
    int c;
    char *s;
    unsigned i, maxlen;
    
    maxlen = 8;
    s = (char *) malloc(maxlen * sizeof(char));

    i = 0;
    while ((c = getc(ifp)) != EOF && (*cond)(c)) {
        s[i++] = c;
        if (i > maxlen - 1) {
            maxlen *= 2;
            s = (char *) realloc(s, maxlen);
        }
    }
    s[i] = '\0';

    if (c != EOF)
        ungetc(c, ifp);

    return s;
}


static Keyword *binsearch(char *kwrd, Keyword *tab, int n)
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
