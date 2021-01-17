#include "chunk.h"
#include "memory.h"


static int Chunk_AddConstant(Chunk* chunk, Value value);


void Chunk_Init(Chunk *chunk)
{
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    chunk->lines = NULL;

    ValueArray_Init(&chunk->constants);
}


void Chunk_Write(Chunk *chunk, uint8_t byte, int line)
{
    if (chunk->capacity < chunk->count + 1) {
        int old_capacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(old_capacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, old_capacity, chunk->capacity);
        chunk->lines = GROW_ARRAY(int, chunk->lines, old_capacity, chunk->capacity);
    }

    chunk->code[chunk->count] = byte;
    chunk->lines[chunk->count] = line;
    chunk->count++;
}


void Chunk_WriteConstant(Chunk *chunk, Value value, int line)
{
#define MAX_CONSTANTS 254
    int constant_idx;

    constant_idx = Chunk_AddConstant(chunk, value);
    if (constant_idx > MAX_CONSTANTS) {
        Chunk_Write(chunk, OP_CONSTANT_LONG, line);
        Chunk_Write(chunk, (uint8_t)(constant_idx), line);
        Chunk_Write(chunk, (uint8_t)((constant_idx >> 8)), line);
        Chunk_Write(chunk, (uint8_t)((constant_idx >> 16)), line);
    } else {
        Chunk_Write(chunk, OP_CONSTANT, line);
        Chunk_Write(chunk, constant_idx, line);
    }
#undef MAX_CONSTANTS
}


void Chunk_Free(Chunk *chunk)
{
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    FREE_ARRAY(int, chunk->lines, chunk->capacity);
    ValueArray_Free(&chunk->constants);
    Chunk_Init(chunk);
}


static int Chunk_AddConstant(Chunk* chunk, Value value) 
{
    ValueArray_Write(&chunk->constants, value);
    return chunk->constants.count - 1;
}
