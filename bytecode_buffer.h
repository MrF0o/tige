#ifndef TIGE_BYTECODE_BUFFER_H
#define TIGE_BYTECODE_BUFFER_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "opcode.h"

#define INITIAL_CHUNK_CAPACITY 1024

// Structure to hold a single bytecode chunk (doubly linked list)
typedef struct BytecodeChunk {
    uint8_t *bytecode;              // Pointer to bytecode array for this chunk
    size_t size;                    // Current size of this chunk's bytecode
    size_t capacity;                // Allocated capacity for this chunk
    size_t chunk_id;                // Unique identifier for the chunk
    bool is_linked;

    struct BytecodeChunk *prev;     // Pointer to the previous chunk
    struct BytecodeChunk *next;     // Pointer to the next chunk
} BytecodeChunk;

// Structure to represent a jump placeholder
typedef struct {
    BytecodeChunk *chunk; // The chunk where the jump instruction is
    size_t offset;        // Offset within the chunk where the placeholder is
} JumpPlaceholder;

// Structure to hold the bytecode buffer with multiple chunks
typedef struct {
    BytecodeChunk *head;            // Pointer to the first chunk
    BytecodeChunk *tail;            // Pointer to the last chunk
    BytecodeChunk *current_chunk;   // Pointer to the current chunk
    size_t chunk_count;             // Number of chunks currently in use
    size_t next_chunk_id;           // Next chunk ID to assign
    uint16_t unused_push_count;
} BytecodeBuffer;

// Function prototypes
BytecodeBuffer *bc_buffer_create();

void bc_destroy_bytecode_buffer(BytecodeBuffer *buffer);

// Emit functions
void bc_emit_byte(BytecodeBuffer *buffer, uint8_t byte);

void bc_emit_int(BytecodeBuffer *buffer, int64_t value);

void bc_emit_uint(BytecodeBuffer *buffer, uint64_t value);

void bc_emit_float(BytecodeBuffer *buffer, double value);

// Emits a null-terminated string
void bc_emit_string(BytecodeBuffer *buffer, const char *string);

void bc_emit_opcode(BytecodeBuffer *buffer, Opcode opcode);

// Combined emit functions to ensure atomic emission
void bc_emit_opcode_with_string(BytecodeBuffer *buffer, Opcode opcode, const char *string);

void bc_emit_opcode_with_int(BytecodeBuffer *buffer, Opcode opcode, int64_t value);

void bc_emit_opcode_with_uint(BytecodeBuffer *buffer, Opcode opcode, uint64_t value);

void bc_emit_opcode_with_uint16(BytecodeBuffer *buffer, Opcode opcode, uint16_t value);

void bc_emit_opcode_with_float(BytecodeBuffer *buffer, Opcode opcode, double value);

void bc_emit_opcode_with_byte(BytecodeBuffer *buffer, Opcode opcode, uint8_t value);

// emit a jump with a placeholder
JumpPlaceholder bc_emit_jump_with_placeholder(BytecodeBuffer *buffer, Opcode opcode);

// backpatch a jump with the actual target address
void bc_backpatch_jump(JumpPlaceholder placeholder, size_t target_chunk_id, size_t target_offset);

void bc_emit_opcode_with_jump(BytecodeBuffer *buffer, Opcode opcode, size_t chunk_id, size_t offset);

bool bc_is_buffer_valid(BytecodeBuffer *buffer);

// Chunk management utility functions
BytecodeChunk *bc_create_bytecode_chunk(size_t initial_capacity);

void bc_destroy_bytecode_chunk(BytecodeChunk *chunk);

bool bc_chunk_has_free_space(BytecodeChunk *chunk, size_t size_needed);

BytecodeChunk *bc_add_new_chunk(BytecodeBuffer *buffer, size_t initial_capacity, bool link);

size_t bc_get_total_bytecode_size(BytecodeBuffer *buffer);

// Reading from the bytecode buffer
uint8_t bc_read_byte(BytecodeChunk **chunk, size_t *offset);

int64_t bc_read_int(BytecodeChunk **chunk, size_t *offset);

uint64_t bc_read_uint(BytecodeChunk **chunk, size_t *offset);

double bc_read_float(BytecodeChunk **chunk, size_t *offset);

char *bc_read_string(BytecodeChunk **chunk, size_t *offset);

// Unsafe write into bytecode buffer, internal use only!!
void bc_write_byte(BytecodeChunk **chunk, uint8_t byte);
void bc_write_ptr(BytecodeChunk **chunk, uintptr_t ptr);

// Jumping between chunks
void bc_set_chunk(BytecodeBuffer *buffer, size_t chunk_id, BytecodeChunk **chunk);
BytecodeChunk* bc_get_chunk_by_id(BytecodeBuffer* buffer, size_t chunk_id);

// Checking if a chunk has code/free slots
bool bc_chunk_has_code(BytecodeChunk *chunk);

bool bc_chunk_has_free_slots(BytecodeChunk *chunk, size_t size_needed);

#endif // TIGE_BYTECODE_BUFFER_H