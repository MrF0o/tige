//
// Created by fathi on 10/21/2024.
//

#ifndef TIGE_MEMORY_H
#define TIGE_MEMORY_H

#include <stdlib.h>
#include <memory.h>
#include "object.h"

typedef struct Context Context;

typedef struct Heap Heap;
typedef struct HeapBlock HeapBlock;
typedef struct Stack Stack;

struct HeapBlock {
    Object *object;
    size_t size;
    HeapBlock *next;
};

struct Heap {
    HeapBlock *blocks;
    size_t total_allocated; // total bytes allocated
    size_t total_freed;     // total bytes freed
};

struct Stack {
    Value *values;
    int capacity;
    int sp;                 // stack pointer (-1 indicates empty stack)
};

/// stack functions
Stack *create_stack(int initial_capacity);
void destroy_stack(Stack *stack);
bool push_stack(Stack *stack, Value value);
bool pop_stack(Stack *stack, Value *value);
bool peek_stack(Stack *stack, Value *value);

// heap functions
Heap *create_heap();
void destroy_heap(Heap *heap);
uint8_t *heap_alloc(size_t size);
bool heap_free(uint8_t *ptr);

// ..
void *vm_malloc(size_t size);
void vm_free(void *ptr);

void* tige_alloc(Context* ctx, size_t size);
void tige_free(Context* ctx, void* ptr);

#endif //TIGE_MEMORY_H
