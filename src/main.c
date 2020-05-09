#include <stdio.h>
#include <stdlib.h>

#include "expr.h"
#include "parser.h"
#include "scanner.h"
#include "interpreter.h"

int main(int argc, char *argv[])
{
    FILE *source;
    Token *tokens;
    Expr *expr;
    ExprResult *res;

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

    for (;;) {
        if (feof(source))
            break;

        printf("lox > ");

        if ((tokens = scan(source)) == NULL) 
            continue;

        expr = parse(tokens);

        if (expr != NULL) {
            if ((res = eval(expr)) != NULL)
                print_expr_res(res);
            else
                fprintf(stderr, "RuntimeError\n");
        } else {
            fprintf(stderr, "Error: invalid expression\n");
        }
    }

    fclose(source);

    return 0;
}
