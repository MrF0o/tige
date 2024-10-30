//
// Created by fathi on 10/21/2024.
//

#include "context.h"
#include "memory.h"
#include "vm.h"         // To access the global VM
#include "value.h"      // Assuming Value is defined here
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ------------------------
// Stack Management
// ------------------------

// Create a new stack with a specified initial capacity
Stack *create_stack(int initial_capacity) {
    Stack *stack = malloc(sizeof(Stack));
    if (!stack) {
        fprintf(stderr, "Failed to allocate memory for stack.\n");
        exit(EXIT_FAILURE);
    }
    stack->values = malloc(sizeof(Value) * initial_capacity);
    if (!stack->values) {
        fprintf(stderr, "Failed to allocate memory for stack values.\n");
        free(stack);
        exit(EXIT_FAILURE);
    }
    stack->capacity = initial_capacity;
    stack->sp = -1; // Empty stack
    return stack;
}

// Destroy the stack and free its memory
void destroy_stack(Stack *stack) {
    if (stack) {
        free(stack->values);
        free(stack);
    }
}

// Push a value onto the stack
inline bool push_stack(Stack *stack, Value value) {
    if (stack->sp + 1 >= stack->capacity) {
        // Resize the stack (double the capacity)
        int new_capacity = stack->capacity * 2;
        Value *new_values = realloc(stack->values, sizeof(Value) * new_capacity);
        if (!new_values) {
            fprintf(stderr, "Stack overflow! Cannot vm_push value.\n");
            return false;
        }
        stack->values = new_values;
        stack->capacity = new_capacity;
    }
    stack->values[++stack->sp] = value;
    return true;
}

// Pop a value from the stack
inline bool pop_stack(Stack *stack, Value *value) {
    if (stack->sp < 0) {
        fprintf(stderr, "Stack underflow! Cannot pop value.\n");
        return false;
    }
    *value = stack->values[stack->sp--];
    return true;
}

// Peek at the top value of the stack without popping
bool peek_stack(Stack *stack, Value *value) {
    if (stack->sp < 0) {
        fprintf(stderr, "Stack underflow! Cannot peek value.\n");
        return false;
    }
    *value = stack->values[stack->sp];
    return true;
}

// ------------------------
// Heap Management
// ------------------------

// Create a new heap
Heap *create_heap() {
    Heap *heap = malloc(sizeof(Heap));
    if (!heap) {
        fprintf(stderr, "Failed to allocate memory for heap.\n");
        exit(EXIT_FAILURE);
    }
    heap->blocks = NULL;
    heap->total_allocated = 0;
    heap->total_freed = 0;
    return heap;
}

// Destroy the heap and free all allocated blocks
void destroy_heap(Heap *heap) {
    if (heap) {
        HeapBlock *current = heap->blocks;
        while (current) {
            HeapBlock *next = current->next;
            // Assuming that objects are managed separately,
            // you might need to handle object-specific cleanup here.
            // For example, if objects have destructors, call them here.
            free(current->object); // Free the Object itself
            free(current);
            current = next;
        }
        printf("Heap destroyed. Total Allocated: %zu bytes, Total Freed: %zu bytes.\n",
               heap->total_allocated, heap->total_freed);
        free(heap);
    }
}

// Allocate a block of memory on the heap (allocates an Object)
uint8_t *heap_alloc(size_t size) {
    if (!get_vm()) {
        fprintf(stderr, "VM not initialized. Cannot allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    Heap *heap = get_vm()->heap;
    HeapBlock *block = malloc(sizeof(HeapBlock));
    if (!block) {
        fprintf(stderr, "Failed to allocate memory for heap block.\n");
        exit(EXIT_FAILURE);
    }
    block->object = malloc(size);
    if (!block->object) {
        fprintf(stderr, "Failed to allocate %zu bytes on heap.\n", size);
        free(block);
        exit(EXIT_FAILURE);
    }
    block->size = size;
    block->next = heap->blocks;
    heap->blocks = block;
    heap->total_allocated += size;
    memset(block->object, 0, size); // Initialize memory to zero
    return (uint8_t *)block->object;
}

// Free a block of memory on the heap (frees an Object)
bool heap_free(uint8_t *ptr) {
    if (!get_vm()) {
        fprintf(stderr, "VM not initialized. Cannot free memory.\n");
        return false;
    }

    Heap *heap = get_vm()->heap;
    HeapBlock **current = &heap->blocks;
    while (*current) {
        if ((*current)->object == ptr) {
            HeapBlock *to_free = *current;
            *current = to_free->next;
            heap->total_freed += to_free->size;
            free(to_free->object);
            free(to_free);
            return true;
        }
        current = &(*current)->next;
    }
    fprintf(stderr, "Attempted to free unallocated memory at %p.\n", (void *)ptr);
    return false;
}

// ------------------------
// Memory Management Functions
// ------------------------

// Allocate memory using the heap (wrapper for heap_alloc)
void *vm_malloc(size_t size) {
    return (void *)heap_alloc(size);
}

// Free memory using the heap (wrapper for heap_free)
void vm_free(void *ptr) {
    if (ptr == NULL) return; // Nothing to free
    if (!heap_free((uint8_t *)ptr)) {
        fprintf(stderr, "vm_free failed to deallocate memory at %p.\n", ptr);
        // TODO: error handling
    }
}

// ------------------------
// Utility Functions
// ------------------------

// Print memory statistics for debugging
void print_memory_stats(void) {
    if (!get_vm()) {
        fprintf(stderr, "VM not initialized. Cannot print memory stats.\n");
        return;
    }

    Heap *heap = get_vm()->heap;
    printf("Heap Memory Stats:\n");
    printf("Total Allocated: %zu bytes\n", heap->total_allocated);
    printf("Total Freed: %zu bytes\n", heap->total_freed);
    size_t current_allocated = heap->total_allocated - heap->total_freed;
    printf("Current Allocated: %zu bytes\n", current_allocated);
}
void* tige_alloc(Context* ctx, size_t size) {
    ctx->total_mem += size;
    return malloc(size);
}

void tige_free(Context* ctx, void* ptr) {
    if (ctx->total_mem > 0) {
        free(ptr);
    }
}