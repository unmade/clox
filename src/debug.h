#ifndef clox_debug_h
#define clox_debug_h

#include "chunk.h"


void Chunk_Disassemble(Chunk *chunk, const char* name);
int Chunk_DisassembleInstruction(Chunk *chunk, int offset);


#endif
