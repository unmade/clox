#include <stdio.h>

#include "debug.h"
#include "value.h"


static int simple_instruction(char *name, int offset);
static int constant_instruction(char *name, Chunk *chunk, int offset);


void Chunk_Disassemble(Chunk *chunk, const char *name)
{
    int offset;

    printf("== %s ==\n", name);  

    for (offset = 0; offset < chunk->count;) {
        offset = Chunk_DisassembleInstruction(chunk, offset);
    }
}


int Chunk_DisassembleInstruction(Chunk *chunk, int offset)
{
    uint8_t instruction;

    printf("%04d ", offset);

    switch (instruction = chunk->code[offset]) {
        case OP_CONSTANT:
            return constant_instruction("OP_CONSTANT", chunk, offset);
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


static int constant_instruction(char *name, Chunk *chunk, int offset)
{
    uint8_t constant_idx;

    constant_idx = chunk->code[offset + 1];
    printf("%-16s: %4d'", name, constant_idx);
    Value_Print(chunk->constants.values[constant_idx]);
    printf("'\n");

    return offset + 2;
}
