#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"


typedef enum {
    OP_CONSTANT,
    OP_CONSTANT_LONG,
    OP_ADD,
    OP_SUBSTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NEGATE,
    OP_RETURN,
} OpCode;


typedef struct {
    int count;
    int capacity;
    uint8_t *code;
    int *lines;
    ValueArray constants;
} Chunk;


void Chunk_Init(Chunk *chunk);
void Chunk_Write(Chunk *chunk, uint8_t byte, int line);
void Chunk_WriteConstant(Chunk *chunk, Value value, int line);
void Chunk_Free(Chunk *chunk);

#endif
