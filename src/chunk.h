#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"


typedef enum {
    OP_RETURN,
} OpCode;


typedef struct {
    int count;
    int capacity;
    uint8_t *code;
} Chunk;


void Chunk_Init(Chunk *chunk);
void Chunk_Write(Chunk *chunk, uint8_t byte);
void Chunk_Free(Chunk *chunk);

#endif
