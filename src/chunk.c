#include "chunk.h"
#include "memory.h"


static int Chunk_AddConstant(Chunk* chunk, Value value);


static void LineArray_Init(LineArray *lines)
{
    lines->count = 0;
    lines->capacity = 0;
    lines->lines = NULL;
}


static void LineArray_Free(LineArray *lines)
{
    FREE_ARRAY(int, lines->lines, lines->capacity);
    LineArray_Init(lines);
}


static void LineArray_Write(LineArray *lines, int line)
{
    if (lines->capacity < lines->count + 2) {
        int old_capacity = lines->capacity;
        lines->capacity = GROW_CAPACITY(old_capacity);
        lines->lines = GROW_ARRAY(int, lines->lines, old_capacity, lines->capacity);
    }

    if (lines->count > 2 && lines->lines[lines->count - 2] == line) {
        lines->lines[lines->count - 1] += 1;
    } else {
        lines->lines[lines->count] = line;
        lines->lines[lines->count + 1] = 1;
        lines->count += 2;
    }
}


static int LineArray_Get(LineArray *lines, int offset)
{
    int prev_count, next_count;
    int *line;

    line = lines->lines;
    prev_count = 0;
    next_count = *(line + 1);
    while ((line - lines->lines) < lines->count) {
        if ((offset >= prev_count) && (offset < next_count)) {
            return *line;
        }
        line += 2;
        prev_count = next_count;
        next_count += *(line + 1);
    }
}


void Chunk_Init(Chunk *chunk)
{
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;

    LineArray_Init(&chunk->lines);
    ValueArray_Init(&chunk->constants);
}


void Chunk_Write(Chunk *chunk, uint8_t byte, int line)
{
    if (chunk->capacity < chunk->count + 1) {
        int old_capacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(old_capacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, old_capacity, chunk->capacity);
    }

    chunk->code[chunk->count] = byte;
    chunk->count++;
    LineArray_Write(&chunk->lines, line);
}


void Chunk_WriteConstant(Chunk *chunk, Value value, int line)
{
#define MAX_CONSTANTS 255
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


int Chunk_GetLine(Chunk *chunk, int offset)
{
    return LineArray_Get(&chunk->lines, offset);
}


void Chunk_Free(Chunk *chunk)
{
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    ValueArray_Free(&chunk->constants);
    LineArray_Free(&chunk->lines);
    Chunk_Init(chunk);
}


static int Chunk_AddConstant(Chunk* chunk, Value value) 
{
    ValueArray_Write(&chunk->constants, value);
    return chunk->constants.count - 1;
}
