#include <stdbool.h>
#include <string.h>

#include "scanner.h"


Scanner scanner;

static TokenType Scanner_GetIdentifierType();
static TokenType Scanner_CheckKeyword(int start, int length, const char *rest, TokenType type);


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


static char Scanner_Advance()
{
    if (*scanner.current == '\n') {
        scanner.line++;
    }
    return *scanner.current++;
}


static char Scanner_Peek()
{
    return *scanner.current;
}


static char Scanner_PeekAhead()
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
        switch (c = Scanner_Peek()) {
            case ' ':
            case '\r':
            case '\t':
            case '\n':
                Scanner_Advance();
                break;
            case '/':
                if (Scanner_PeekAhead() == '/') {
                    while (Scanner_Peek() != '\n' && !Scanner_IsAtEnd()) {
                        Scanner_Advance();
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


static bool Scanner_Match(char c)
{
    if (Scanner_Peek() == c) {
        Scanner_Advance();
        return true;
    }

    return false;
}


static Token Scanner_ScanString()
{
    char c;

    while ((c = Scanner_Peek()) != '"' && !Scanner_IsAtEnd()) {
        Scanner_Advance();
    }

    if (Scanner_IsAtEnd()) {
        return Token_Error("Unterminated string.");
    }

    Scanner_Advance();

    return Token_Make(TOKEN_STRING);
}


static Token Scanner_ScanNumber()
{
    while (is_digit(Scanner_Peek())) {
        Scanner_Advance();
    }

    if (Scanner_Peek() == '.' && is_digit(Scanner_PeekAhead())) {
        Scanner_Advance();

        while (is_digit(Scanner_Peek())) {
            Scanner_Advance();
        }
    }

    return Token_Make(TOKEN_NUMBER);
}


static Token Scanner_ScanIdentifierOrKeyword()
{
    char c;

    while (is_alpha(c = Scanner_Peek()) || is_digit(c)) {
        Scanner_Advance();
    }

    return Token_Make(Scanner_GetIdentifierType());
}


static TokenType Scanner_GetIdentifierType()
{
    switch (*scanner.start) {
        case 'a':
            return Scanner_CheckKeyword(1, 2, "nd", TOKEN_AND);
        case 'c':
            return Scanner_CheckKeyword(1, 4, "lass", TOKEN_CLASS);
        case 'e':
            return Scanner_CheckKeyword(1, 3, "lse", TOKEN_ELSE);
        case 'f':
            if (scanner.current - scanner.start > 1) {
                switch (*(scanner.start + 1)) {
                    case 'a':
                        return Scanner_CheckKeyword(2, 3, "lse", TOKEN_FALSE);
                    case 'o':
                        return Scanner_CheckKeyword(2, 1, "r", TOKEN_FOR);
                    case 'u':
                        return Scanner_CheckKeyword(2, 1, "n", TOKEN_FUN);
                }
            }
            break;
        case 'i':
            return Scanner_CheckKeyword(1, 1, "f", TOKEN_IF);
        case 'n':
            return Scanner_CheckKeyword(1, 2, "il", TOKEN_NIL);
        case 'o':
            return Scanner_CheckKeyword(1, 1, "r", TOKEN_OR);
        case 'p':
            return Scanner_CheckKeyword(1, 4, "rint", TOKEN_PRINT);
        case 'r':
            return Scanner_CheckKeyword(1, 5, "eturn", TOKEN_RETURN);
        case 's':
            return Scanner_CheckKeyword(1, 4, "uper", TOKEN_SUPER);
        case 't':
            if (scanner.current - scanner.start > 1) {
                switch(*(scanner.start + 1)) {
                    case 'h':
                        return Scanner_CheckKeyword(2, 2, "is", TOKEN_THIS);
                    case 'r':
                        return Scanner_CheckKeyword(2, 2, "ue", TOKEN_TRUE);
                }
            }
            break;
        case 'v':
            return Scanner_CheckKeyword(1, 2, "ar", TOKEN_VAR);
        case 'w':
            return Scanner_CheckKeyword(1, 4, "hile", TOKEN_WHILE);
    }

    return TOKEN_IDENTIFIER;
}


static TokenType Scanner_CheckKeyword(int start, int length, const char *rest, TokenType type)
{
    if (scanner.current - scanner.start == start + length && 
        memcmp(scanner.start + start, rest, length) == 0) {
        return type;
    }

    return TOKEN_IDENTIFIER;
}


Token Scanner_GetToken()
{
    char c;

    Scanner_SkipWhitespace();
    scanner.start = scanner.current;

    if (Scanner_IsAtEnd()) {
        return Token_Make(TOKEN_EOF);
    }

    c = Scanner_Advance();

    if (is_alpha(c)) {
        return Scanner_ScanIdentifierOrKeyword();
    }

    if (is_digit(c)) {
        return Scanner_ScanNumber();
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
            return Token_Make(Scanner_Match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=':
            return Token_Make(Scanner_Match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<':
            return Token_Make(Scanner_Match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>':
            return Token_Make(Scanner_Match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
        case '"':
            return Scanner_ScanString();
    }

    return Token_Error("Unexpected character.");
}
