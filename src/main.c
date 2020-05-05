#include <stdio.h>
#include <stdlib.h>

#include "scanner.h"

int main(int argc, char *argv[])
{
    int i, n;
    FILE *source;
    Token *tokens[MAXTOKEN];

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
        n = scan(source, tokens);

        for (i = 0; i < n; i++) {
            printf("Token(type=%d, repr='%s')\n", tokens[i]->type, tokens[i]->repr);
        }

        if (feof(source))
            break;
    }

    fclose(source);

    return 0;
}
