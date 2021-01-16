#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"


void compile(const char *source)
{
    Scanner_Init(source);
    int line = -1;

    for (;;) {
        Token token = Scanner_GetToken();
        if (token.line != line) {
            printf("%4d", token.line);
            line = token.line;
        } else {
            printf("   | ");
        }
        printf("%2d '%.*s'\n", token.type, token.length, token.start);

        if (token.type == TOKEN_EOF) {
            break;
        }
    }
}
