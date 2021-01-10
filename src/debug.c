#include <stdio.h>

#include "debug.h"


static int simple_instruction(char *name, int offset);


void Chunk_Disassemble(Chunk *chunk, const char *name)
{
    int offset;

    printf("==  ==\n", name);  

    for (offset = 0; offset < chunk->count; offset++) {
        offset = Chunk_DisassembleInstruction(chunk, offset);
    }
}


int Chunk_DisassembleInstruction(Chunk *chunk, int offset)
{
    uint8_t instruction;

    printf("%04d ", offset);

    switch (instruction = chunk->code[offset]) {
        case OP_RETURN:
            return simple_instruction("OP_RETURN", offset);
        default:
            printf("Unknown opcode: %d\n", instruction);
            return offset + 1;
    }
}


static int simple_instruction(char *name, int offset)
{
    printf("%s\n", name);
    return offset + 1;
}
