#include <stdio.h>
#include <stdlib.h>

#include "expr.h"
#include "interpreter.h"
#include "parser.h"
#include "scanner.h"
#include "stmt.h"

int main(int argc, char *argv[])
{
    FILE *source;
    Token *tokens;
    Stmt **stmts;
    LoxObj *obj;

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

        if ((stmts = parse(tokens)) == NULL) {
            fprintf(stderr, "Error: invalid statement\n"); 
            continue;
        }

        if (interpret(stmts) != 0) {
            fprintf(stderr, "RuntimeError\n");
            continue;
        }
    }

    fclose(source);

    return 0;
}
