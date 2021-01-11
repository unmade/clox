#include "chunk.h"
#include "memory.h"


void Chunk_Init(Chunk *chunk)
{
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;

    ValueArray_Init(&chunk->constants);
}


void Chunk_Write(Chunk *chunk, uint8_t byte)
{
    if (chunk->capacity < chunk->count + 1) {
        int old_capacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(old_capacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, old_capacity, chunk->capacity);
    }

    chunk->code[chunk->count] = byte;
    chunk->count++;
}


void Chunk_Free(Chunk *chunk)
{
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    ValueArray_Free(&chunk->constants);
    Chunk_Init(chunk);
}


int Chunk_AddConstant(Chunk* chunk, Value value) 
{
    ValueArray_Write(&chunk->constants, value);
    return chunk->constants.count - 1;
}
