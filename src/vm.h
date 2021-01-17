#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "value.h"

// #define STACK_MAX 512


typedef struct {
    Chunk *chunk;
    uint8_t *ip;
    Value *stack;
    // Value stack[STACK_MAX];
    Value *stack_top;
    int stack_capacity;
} VM;


typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} InterpretResult;


void VM_Init();
void VM_Free();
void VM_Push(Value value);
Value VM_Pop();
InterpretResult VM_Interpret(const char *source);
InterpretResult run(Chunk *chunk);

#endif
