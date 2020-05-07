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
    Token *tok;
    for (;;) {
        printf("lox > ");

        tokens = scan(source);
        for (tok = tokens; tok != NULL; tok = tok->next)
            printf("Token(type=%d, lexeme='%s')\n", tok->type, tok->lexeme);
        expr = parse(tokens);

        s[0] = '\0';
        if (expr != NULL) {
            str_expr(s, expr);
            printf("expr = %s\n", s);
        } else {
            fprintf(stderr, "Error: invalid expression\n");
        }

        if (feof(source))
            break;
    }

    fclose(source);

    return 0;
}
