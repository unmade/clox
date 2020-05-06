#include <stdio.h>
#include <stdlib.h>

#include "expr.h"
#include "parser.h"
#include "scanner.h"

int main(int argc, char *argv[])
{
    FILE *source;
    Token *tokens;
    Expr *expr;

    if (argc == 2) {
        source = fopen(argv[1], "rb");
        if (source == NULL) {
            fprintf(stderr, "Could not open file \"%s\".\n", argv[1]);
        }
    } else if (argc == 1) {
        source = stdin;
    } else {
        fprintf(stderr, "Usage: clox [path]\n");
        exit(1);
    }

    char s[128];
    for (;;) {
        printf("lox > ");

        tokens = scan(source);
        expr = parse(tokens);

        s[0] = '\0';
        if (expr != NULL) {
            str_expr(s, expr);
            printf("expr = %s\n", s);
        }

        if (feof(source))
            break;
    }

    fclose(source);

    return 0;
}
