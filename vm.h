//
// Created by fathi on 10/21/2024.
//

#ifndef TIGE_VM_H
#define TIGE_VM_H

#include <stdint.h>
#include <stdlib.h>
#include "opcode.h"
#include "value.h"
#include "bytecode_buffer.h"
#include "memory.h"

#define uimplemented() fprintf(stderr, "%s is not implemented in %s at line %d", __FUNCTION__, __FILE_NAME__, __LINE__); exit(EXIT_FAILURE)

#define STACK_SIZE 2048
#define MAX_REGISTERS 512
#define SP get_vm()->stack->sp

typedef struct VM VM;
typedef struct Context Context;

// VM structure
struct VM {
    Context *context;
    uint8_t *bytecode;
    BytecodeChunk *chunk;  // Pointer to the current executing chunk
    BytecodeBuffer *buffer;
    size_t size;        // Size of bytecode
    size_t ip;          // Instruction pointer

    Value registers[MAX_REGISTERS];
    Heap* heap;
    Stack* stack;
    int sp;             // Stack pointer
    int sp_reset;
};

// Function prototypes
VM *create_vm(Context *context);
void destroy_vm(VM *vm);
void vm_push(VM *vm, Value value);
Value vm_pop(VM *vm);
Value vm_execute(VM *vm);

// chunks manipulation
bool vm_jump_to_chunk(VM *vm, size_t chunk_id);
void vm_jump_to_chunk_adr(VM *vm, uintptr_t chunk_ptr);

// memory reads
uint8_t vm_read_byte(VM *vm);
char *vm_read_string(VM *vm);
uint64_t vm_read_uint(VM *vm);
uint16_t vm_read_uint16(VM *vm);
int64_t vm_read_int(VM *vm);
size_t vm_read_offset(VM *vm);
uintptr_t vm_read_ptr(VM *vm);

VM* get_vm(void);

#endif //TIGE_VM_H
