#include "debug.h"
#include "common.h"
#include "chunk.h"


int main (int argc, char *argv[])
{
    Chunk chunk;

    Chunk_Init(&chunk);

    int constant_idx = Chunk_AddConstant(&chunk, 1.2);
    Chunk_Write(&chunk, OP_CONSTANT, 123);
    Chunk_Write(&chunk, constant_idx, 123);

    Chunk_Write(&chunk, OP_RETURN, 123);
    Chunk_Disassemble(&chunk, "test chunk");
    Chunk_Free(&chunk);

    return 0;
}

