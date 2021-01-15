#include "debug.h"
#include "common.h"
#include "chunk.h"
#include "vm.h"


int main (int argc, char *argv[])
{
    VM_Init();

    Chunk chunk;

    Chunk_Init(&chunk);

    int constant_idx = Chunk_AddConstant(&chunk, 1.2);
    Chunk_Write(&chunk, OP_CONSTANT, 123);
    Chunk_Write(&chunk, constant_idx, 123);

    Chunk_Write(&chunk, OP_RETURN, 123);
    Chunk_Disassemble(&chunk, "test chunk");
    VM_Interpret(&chunk);
    Chunk_Free(&chunk);

    VM_Free();

    return 0;
}

