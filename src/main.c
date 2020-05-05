#include <stdio.h>
#include <stdlib.h>

#include "expr.h"
#include "scanner.h"

int main(int argc, char *argv[])
{
    FILE *source;
    Token *token;

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
        printf("lox > ");

        for (token = scan(source); token != NULL; token = token->next)
            printf("Token(type=%d, lexeme='%s')\n", token->type, token->lexeme);

        if (feof(source))
            break;
    }

    fclose(source);

    return 0;
}
