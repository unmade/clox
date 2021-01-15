#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"

typedef struct {
    Chunk *chunk;
    uint8_t *ip;
} VM;


typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} InterpretResult;


void VM_Init();
void VM_Free();
InterpretResult VM_Interpret(Chunk *chunk);

#endif
