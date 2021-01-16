#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "vm.h"


VM vm;

static void reset_stack();
static InterpretResult run();


void VM_Init()
{
    reset_stack();
}


static void reset_stack()
{
    vm.stackTop = vm.stack;
}


void VM_Free()
{
}


void VM_Push(Value value)
{
    *vm.stackTop = value;
    vm.stackTop++;
}


Value VM_Pop()
{
    vm.stackTop--;
    return *vm.stackTop;
}


InterpretResult VM_Interpret(const char *source)
{
    compile(source);
    return INTERPRET_OK;
}


static InterpretResult run()
{
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(op) \
    do { \
        Value b = VM_Pop(); \
        Value a = VM_Pop(); \
        VM_Push(a op b); \
    } while (false)

    for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
        printf("          ");
        for (Value *slot = vm.stack; slot < vm.stackTop; slot++) {
            printf("[");
            Value_Print(*slot);
            printf("]");
        }
        printf("\n");
        Chunk_DisassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));    
#endif

        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                VM_Push(constant);
                break;
            }
            case OP_ADD: {
                BINARY_OP(+);
                break;
            }
            case OP_SUBSTRACT: {
                BINARY_OP(-);
                break;
            }
            case OP_MULTIPLY: {
                BINARY_OP(*);
                break;
            }
            case OP_DIVIDE: {
                BINARY_OP(/);
                break;
            }
            case OP_NEGATE: {
                VM_Push(-VM_Pop());
                break;
            }
            case OP_RETURN: {
                Value_Print(VM_Pop());
                printf("\n");
                return INTERPRET_OK;
            }
        }
    }
    return INTERPRET_RUNTIME_ERROR;

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

