#include <stdio.h>

#include "debug.h"
#include "value.h"


static int simple_instruction(char *name, int offset);
static int constant_instruction(char *name, Chunk *chunk, int offset);
static int constant_long_instruction(char *name, Chunk *chunk, int offset);


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

    if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1]) {
        printf("   | ");
    } else {
        printf("%4d ", chunk->lines[offset]);
    }

    switch (instruction = chunk->code[offset]) {
        case OP_CONSTANT:
            return constant_instruction("OP_CONSTANT", chunk, offset);
        case OP_CONSTANT_LONG:
            return constant_long_instruction("OP_CONSTANT_LONG", chunk, offset);
        case OP_ADD:
            return simple_instruction("OP_ADD", offset);
        case OP_SUBSTRACT:
            return simple_instruction("OP_SUBSTRACT", offset);
        case OP_MULTIPLY:
            return simple_instruction("OP_MULTIPLY", offset);
        case OP_DIVIDE:
            return simple_instruction("OP_DIVIDE", offset);
        case OP_NEGATE:
            return simple_instruction("OP_NEGATE", offset);
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
    printf("%-16s: %4d '", name, constant_idx);
    Value_Print(chunk->constants.values[constant_idx]);
    printf("'\n");

    return offset + 2;
}


static int constant_long_instruction(char *name, Chunk *chunk, int offset)
{
    int constant_idx;

    constant_idx = (
        chunk->code[offset + 1] | 
        chunk->code[offset + 2] << 8 |
        chunk->code[offset + 3] << 16
    );
    printf("%-16s: %4d '", name, constant_idx);
    Value_Print(chunk->constants.values[constant_idx]);
    printf("'\n");

    return offset + 4;
}
