#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "common.h"
#include "chunk.h"
#include "vm.h"


static void repl()
{
    char line[1024];

    for (;;) {
        printf("> ");

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        VM_Interpret(line);
    }
}


int main (int argc, char *argv[])
{
    VM_Init();

    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        fprintf(stderr, "Scripts are not supported yet");
        exit(64);
    } else {
        fprintf(stderr, "Usage: clox\n");
        exit(64);
    }

    VM_Free();

    return 0;
}

