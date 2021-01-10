#include "debug.h"
#include "common.h"
#include "chunk.h"


int main (int argc, char *argv[])
{
    Chunk chunk;

    Chunk_Init(&chunk);
    Chunk_Write(&chunk, OP_RETURN);
    Chunk_Disassemble(&chunk, "test chunk");
//    Chunk_Free(&chunk);

    return 0;
}

