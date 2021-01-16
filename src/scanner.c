#include <stdbool.h>
#include <string.h>

#include "scanner.h"


Scanner scanner;

static bool Scanner_IsAtEnd();
static char Scanner_GetChar();
static char Scanner_PeekChar();
static char Scanner_PeekNextChar();


static bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}


static Token Token_Make(TokenType type)
{
    Token token;

    token.type = type;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;

    return token;
}


static Token Token_Error(const char *message)
{
    Token token;

    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner.line;

    return token;
}


static Token Token_String()
{
    char c;

    while ((c = Scanner_PeekChar()) != '"' && !Scanner_IsAtEnd()) {
        if (c == '\n') {
            scanner.line++;
        }
        Scanner_GetChar();
    }

    if (Scanner_IsAtEnd()) {
        return Token_Error("Unterminated string.");
    }

    Scanner_GetChar();

    return Token_Make(TOKEN_STRING);
}


static Token Token_Number()
{
    while (is_digit(Scanner_PeekChar())) {
        Scanner_GetChar();
    }

    if (Scanner_PeekChar() == '.' && is_digit(Scanner_PeekNextChar())) {
        Scanner_GetChar();

        while (is_digit(Scanner_PeekChar())) {
            Scanner_GetChar();
        }
    }

    return Token_Make(TOKEN_NUMBER);
}


static TokenType check_keyword(int start, int length, const char *rest, TokenType type) {
    if (scanner.current - scanner.start == start + length && 
        memcmp(scanner.start + start, rest, length) == 0) {
        return type;
    }

    return TOKEN_IDENTIFIER;
}


static TokenType identifier_type()
{
    switch (*scanner.start) {
        case 'a':
            return check_keyword(1, 2, "nd", TOKEN_AND);
        case 'c':
            return check_keyword(1, 4, "lass", TOKEN_CLASS);
        case 'e':
            return check_keyword(1, 3, "lse", TOKEN_ELSE);
        case 'f':
            if (scanner.current - scanner.start > 1) {
                switch (*(scanner.start + 1)) {
                    case 'a':
                        return check_keyword(2, 3, "lse", TOKEN_FALSE);
                    case 'o':
                        return check_keyword(2, 1, "r", TOKEN_FOR);
                    case 'u':
                        return check_keyword(2, 1, "n", TOKEN_FUN);
                }
            }
            break;
        case 'i':
            return check_keyword(1, 1, "f", TOKEN_IF);
        case 'n':
            return check_keyword(1, 2, "il", TOKEN_NIL);
        case 'o':
            return check_keyword(1, 1, "r", TOKEN_OR);
        case 'p':
            return check_keyword(1, 4, "rint", TOKEN_PRINT);
        case 'r':
            return check_keyword(1, 5, "eturn", TOKEN_RETURN);
        case 's':
            return check_keyword(1, 4, "uper", TOKEN_SUPER);
        case 't':
            if (scanner.current - scanner.start > 1) {
                switch(*(scanner.start + 1)) {
                    case 'h':
                        return check_keyword(2, 2, "is", TOKEN_THIS);
                    case 'r':
                        return check_keyword(2, 2, "ue", TOKEN_TRUE);
                }
            }
            break;
        case 'v':
            return check_keyword(1, 2, "ar", TOKEN_VAR);
        case 'w':
            return check_keyword(1, 4, "hile", TOKEN_WHILE);
    }

    return TOKEN_IDENTIFIER;
}


static Token Token_Identifier()
{
    char c;

    while (is_alpha(c = Scanner_PeekChar()) || is_digit(c)) {
        Scanner_GetChar();
    }

    return Token_Make(identifier_type());
}


void Scanner_Init(const char *source)
{
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}


static bool Scanner_IsAtEnd()
{
    return (*scanner.current) == '\0';
}


static char Scanner_GetChar()
{
    return *scanner.current++;
}


static char Scanner_PeekChar()
{
    return *scanner.current;
}


static char Scanner_PeekNextChar()
{
    if (Scanner_IsAtEnd()) {
        return '\0';
    }
    return scanner.current[1];
}


static void Scanner_SkipWhitespace()
{
    char c;

    for (;;) {
        switch (c = Scanner_PeekChar()) {
            case ' ':
            case '\r':
            case '\t': {
                Scanner_GetChar();
                break;
            }
            case '\n':
                scanner.line++;
                Scanner_GetChar();
                break;
            case '/':
                if (Scanner_PeekNextChar() == '/') {
                    while (Scanner_PeekChar() != '\n' && !Scanner_IsAtEnd()) {
                        Scanner_GetChar();
                    } 
                    break;
                } else {
                    return;
                }
            default:
                return;
        }
    }
}


static bool Scanner_MatchChar(char c)
{
    if (Scanner_PeekChar() == c) {
        Scanner_GetChar();
        return true;
    }

    return false;
}


Token Scanner_GetToken()
{
    char c;

    Scanner_SkipWhitespace();
    scanner.start = scanner.current;

    if (Scanner_IsAtEnd()) {
        return Token_Make(TOKEN_EOF);
    }

    c = Scanner_GetChar();

    if (is_alpha(c)) {
        return Token_Identifier();
    }

    if (is_digit(c)) {
        return Token_Number();
    }

    switch (c) {
        case '(':
            return Token_Make(TOKEN_LEFT_PAREN);
        case ')':
            return Token_Make(TOKEN_RIGHT_PAREN);
        case '{':
            return Token_Make(TOKEN_LEFT_BRACE);
        case '}':
            return Token_Make(TOKEN_RIGHT_BRACE);
        case ';':
            return Token_Make(TOKEN_SEMICOLON);
        case ',':
            return Token_Make(TOKEN_COMMA);
        case '.':
            return Token_Make(TOKEN_DOT);
        case '-':
            return Token_Make(TOKEN_MINUS);
        case '+':
            return Token_Make(TOKEN_PLUS);
        case '/':
            return Token_Make(TOKEN_SLASH);
        case '*':
            return Token_Make(TOKEN_STAR);
        case '!':
            return Token_Make(Scanner_MatchChar('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=':
            return Token_Make(Scanner_MatchChar('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<':
            return Token_Make(Scanner_MatchChar('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>':
            return Token_Make(Scanner_MatchChar('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
        case '"':
            return Token_String();
    }

    return Token_Error("Unexpected character.");
}
