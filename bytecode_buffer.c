#include "bytecode_buffer.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

// Create a new bytecode chunk (doubly linked list node)
BytecodeChunk *bc_create_bytecode_chunk(size_t initial_capacity) {
    static size_t global_chunk_id = 0;

    const auto chunk = (BytecodeChunk *) malloc(sizeof(BytecodeChunk));
    chunk->size = 0;
    chunk->capacity = initial_capacity;
    chunk->bytecode = (uint8_t *) malloc(chunk->capacity);
    memset(chunk->bytecode, 0, chunk->capacity);
    chunk->chunk_id = global_chunk_id++;
    chunk->is_linked = true;

    chunk->prev = nullptr;
    chunk->next = nullptr;

    return chunk;
}

// Destroy a bytecode chunk
void bc_destroy_bytecode_chunk(BytecodeChunk *chunk) {
    if (chunk) {
        free(chunk->bytecode);
        free(chunk);
    }
}

// Initialize a new bytecode buffer
BytecodeBuffer *bc_buffer_create() {
    BytecodeBuffer *buffer = (BytecodeBuffer *) malloc(sizeof(BytecodeBuffer));
    buffer->chunk_count = 0;
    buffer->next_chunk_id = 0;
    buffer->head = nullptr;
    buffer->tail = nullptr;
    buffer->current_chunk = nullptr;

    // Create the first chunk
    BytecodeChunk *first_chunk = bc_create_bytecode_chunk(INITIAL_CHUNK_CAPACITY);
    buffer->head = buffer->tail = buffer->current_chunk = first_chunk;
    buffer->chunk_count++;

    return buffer;
}

// Destroy the bytecode buffer and all its chunks
void bc_destroy_bytecode_buffer(BytecodeBuffer *buffer) {
    if (buffer) {
        BytecodeChunk *chunk = buffer->head;
        while (chunk) {
            BytecodeChunk *next_chunk = chunk->next;
            bc_destroy_bytecode_chunk(chunk);
            chunk = next_chunk;
        }
        free(buffer);
    }
}

// Check if a chunk has free space for the required size
bool bc_chunk_has_free_space(BytecodeChunk *chunk, size_t size_needed) {
    return (chunk->size + size_needed) <= chunk->capacity;
}

// Check if a chunk has any code
bool bc_chunk_has_code(BytecodeChunk *chunk) {
    return chunk->size > 0;
}

// Check if a chunk has free slots
bool bc_chunk_has_free_slots(BytecodeChunk *chunk, size_t size_needed) {
    return bc_chunk_has_free_space(chunk, size_needed);
}

// Add a new chunk to the buffer (automatically linked)
BytecodeChunk *bc_add_new_chunk(BytecodeBuffer *buffer, size_t initial_capacity, bool link) {
    // TODO: change chunk is_linked accordingly
    BytecodeChunk *new_chunk = bc_create_bytecode_chunk(initial_capacity);
    buffer->next_chunk_id = new_chunk->chunk_id + 1;
    new_chunk->is_linked = link;

    new_chunk->prev = buffer->tail;
    if (buffer->tail) {
        buffer->tail->next = new_chunk;
    }
    buffer->tail = new_chunk;
    if (!buffer->head) {
        buffer->head = new_chunk;
    }

    if (new_chunk->is_linked && buffer->current_chunk->linked_with == new_chunk->chunk_id) {
        bc_write_byte(&buffer->current_chunk, (uint8_t) OP_JMP_ADR);
        bc_write_ptr(&buffer->current_chunk, (uintptr_t) new_chunk);
        new_chunk->linked_with = buffer->next_chunk_id;
    }

    buffer->current_chunk = new_chunk;
    buffer->chunk_count++;

    return new_chunk;
}

// Ensure there's enough capacity in the current chunk for the entire operation
void bc_ensure_chunk_capacity(BytecodeBuffer *buffer, size_t size_needed) {
    // ensure we have enough space to linkage jump at the end for linked chunks
    size_t link_size = buffer->current_chunk->is_linked ? sizeof(OP_JMP_ADR) + sizeof(uintptr_t) : 0;
    if (!bc_chunk_has_free_space(buffer->current_chunk, size_needed + link_size)) {
        // Create a new chunk
        size_t new_chunk_capacity = buffer->current_chunk->capacity;
        bc_add_new_chunk(buffer, new_chunk_capacity, true);
    }
}

// Internal function to write data to the current chunk
void bc_write_to_chunk(BytecodeBuffer *buffer, const uint8_t *data, size_t data_size) {
    memcpy(&buffer->current_chunk->bytecode[buffer->current_chunk->size], data, data_size);
    buffer->current_chunk->size += data_size;
}

// Emit functions

// Emit a single byte
void bc_emit_byte(BytecodeBuffer *buffer, uint8_t byte) {
    bc_ensure_chunk_capacity(buffer, 1);
    bc_write_to_chunk(buffer, &byte, 1);
}

void bc_emit_int(BytecodeBuffer *buffer, int64_t value) {
    bc_ensure_chunk_capacity(buffer, sizeof(int64_t));
    bc_write_to_chunk(buffer, (uint8_t *) &value, sizeof(int64_t));
}

void bc_emit_uint(BytecodeBuffer *buffer, uint64_t value) {
    bc_ensure_chunk_capacity(buffer, sizeof(uint64_t));
    bc_write_to_chunk(buffer, (uint8_t *) &value, sizeof(uint64_t));
}

void bc_emit_float(BytecodeBuffer *buffer, double value) {
    assert(sizeof(double) * CHAR_BIT == 64);
    bc_ensure_chunk_capacity(buffer, sizeof(double));
    bc_write_to_chunk(buffer, (uint8_t *) &value, sizeof(double));
}

// Emit an opcode
void bc_emit_opcode(BytecodeBuffer *buffer, Opcode opcode) {
    bc_ensure_chunk_capacity(buffer, 1);
    bc_write_to_chunk(buffer, (uint8_t *) &opcode, 1);
}

bool bc_is_buffer_valid(BytecodeBuffer *buffer) {
    if (buffer == NULL) {
        return false;
    }
    BytecodeChunk *chunk = buffer->head;
    while (chunk) {
        if (chunk->bytecode == NULL) {
            return false;
        }
        chunk = chunk->next;
    }
    return true;
}

// Emit a null-terminated string
void bc_emit_string(BytecodeBuffer *buffer, const char *string) {
    if (!buffer || !string) {
        fprintf(stderr, "Error: bc_emit_string received NULL buffer or string.\n");
        return;
    }

    size_t len = strlen(string) + 1; // Include null terminator
    bc_ensure_chunk_capacity(buffer, len);
    bc_write_to_chunk(buffer, (const uint8_t *) string, len);
}

// Combined emit functions to ensure atomic emission

void bc_emit_opcode_with_string(BytecodeBuffer *buffer, Opcode opcode, const char *string) {
    const size_t len = strlen(string) + 1; // Include null terminator
    const size_t total_size = 1 + len;     // Opcode size + string size

    bc_ensure_chunk_capacity(buffer, total_size);

    // Begin atomic emission
    bc_write_to_chunk(buffer, (uint8_t *) &opcode, 1);
    bc_write_to_chunk(buffer, (const uint8_t *) string, len);
}

void bc_emit_opcode_with_string_obj(BytecodeBuffer *buffer, Opcode opcode, TString *string) {
    constexpr size_t size = sizeof(TString*);
    bc_ensure_chunk_capacity(buffer, size + 1);

    bc_write_to_chunk(buffer, (uint8_t *) &opcode, 1);
    bc_write_ptr(&buffer->current_chunk, (uintptr_t) string);
}

void bc_emit_opcode_with_int(BytecodeBuffer *buffer, Opcode opcode, int64_t value) {
    constexpr size_t total_size = 1 + sizeof(int64_t); // Opcode size + int64_t size

    bc_ensure_chunk_capacity(buffer, total_size);

    // Begin atomic emission
    bc_write_to_chunk(buffer, (uint8_t *) &opcode, 1);
    bc_write_to_chunk(buffer, (uint8_t *) &value, sizeof(int64_t));
}

void bc_emit_opcode_with_uint(BytecodeBuffer *buffer, Opcode opcode, uint64_t value) {
    size_t total_size = 1 + sizeof(uint64_t); // Opcode size + uint64_t size

    bc_ensure_chunk_capacity(buffer, total_size);

    // Begin atomic emission
    bc_write_to_chunk(buffer, (uint8_t *) &opcode, 1);
    bc_write_to_chunk(buffer, (uint8_t *) &value, sizeof(uint64_t));
}

// Get the total size of all bytecode chunks
size_t bc_get_total_bytecode_size(BytecodeBuffer *buffer) {
    size_t total_size = 0;
    BytecodeChunk *chunk = buffer->head;
    while (chunk) {
        total_size += chunk->size;
        chunk = chunk->next;
    }
    return total_size;
}

// Reading from the bytecode buffer
// We use a chunk pointer and offset to keep track of the reading position

uint8_t bc_read_byte(BytecodeChunk **chunk, size_t *offset) {
    if (*offset >= (*chunk)->size) {
        // Move to the next chunk
        if ((*chunk)->next == NULL) {
            fprintf(stderr, "Error: Attempt to read bytecode out of bounds.\n");
            exit(EXIT_FAILURE);
        }
        *chunk = (*chunk)->next;
        *offset = 0;
    }
    uint8_t byte = (*chunk)->bytecode[*offset];
    (*offset)++;
    return byte;
}

int64_t bc_read_int(BytecodeChunk **chunk, size_t *offset) {
    if (*offset + sizeof(int64_t) > (*chunk)->size) {
        fprintf(stderr, "Error: int64 value spans multiple chunks (unsupported in this implementation).\n");
        exit(EXIT_FAILURE);
    }
    int64_t value;
    memcpy(&value, &(*chunk)->bytecode[*offset], sizeof(int64_t));
    (*offset) += sizeof(int64_t);
    return value;
}

uint64_t bc_read_uint(BytecodeChunk **chunk, size_t *offset) {
    if (*offset + sizeof(uint64_t) > (*chunk)->size) {
        fprintf(stderr, "Error: uint64 value spans multiple chunks (unsupported in this implementation).\n");
        exit(EXIT_FAILURE);
    }
    uint64_t value;
    memcpy(&value, &(*chunk)->bytecode[*offset], sizeof(uint64_t));
    (*offset) += sizeof(uint64_t);
    return value;
}

double bc_read_float(BytecodeChunk **chunk, size_t *offset) {
    if (*offset + sizeof(double) > (*chunk)->size) {
        fprintf(stderr, "Error: double value spans multiple chunks (unsupported in this implementation).\n");
        exit(EXIT_FAILURE);
    }
    double value;
    memcpy(&value, &(*chunk)->bytecode[*offset], sizeof(double));
    (*offset) += sizeof(double);
    return value;
}

char *bc_read_string(BytecodeChunk **chunk, size_t *offset) {
    size_t start_offset = *offset;
    BytecodeChunk *start_chunk = *chunk;
    size_t str_length = 0;

    // Find the null terminator, within the same chunk
    while (true) {
        if (*offset >= (*chunk)->size) {
            fprintf(stderr, "Error: String not null-terminated within the same chunk.\n");
            exit(EXIT_FAILURE);
        }

        if ((*chunk)->bytecode[*offset] == '\0') {
            // Found null terminator
            str_length = *offset - start_offset + 1; // Include null terminator
            (*offset)++;
            break;
        }

        (*offset)++;
    }

    // Allocate and copy the string
    char *string = (char *) malloc(str_length);
    if (!string) {
        fprintf(stderr, "Error: Failed to allocate memory for string.\n");
        exit(EXIT_FAILURE);
    }

    memcpy(string, &start_chunk->bytecode[start_offset], str_length);
    return string;
}

// Jumping between chunks
void bc_set_chunk(BytecodeBuffer *buffer, size_t chunk_id) {
    BytecodeChunk *current = buffer->head;
    while (current) {
        if (current->chunk_id == chunk_id) {
            buffer->current_chunk = current;
            return;
        }
        current = current->next;
    }
    fprintf(stderr, "Error: Chunk with ID %zu not found.\n", chunk_id);
    exit(EXIT_FAILURE);
}

void bc_emit_opcode_with_float(BytecodeBuffer *buffer, Opcode opcode, double value) {
    size_t total_size = 1 + sizeof(double); // Opcode size + double size

    bc_ensure_chunk_capacity(buffer, total_size);

    bc_write_to_chunk(buffer, (uint8_t *) &opcode, 1);
    bc_write_to_chunk(buffer, (uint8_t *) &value, sizeof(double));
}

void bc_emit_opcode_with_byte(BytecodeBuffer *buffer, Opcode opcode, uint8_t value) {
    size_t total_size = 1 + sizeof(uint8_t); // Opcode size + uint8_t size

    bc_ensure_chunk_capacity(buffer, total_size);

    bc_write_to_chunk(buffer, (uint8_t *) &opcode, 1);
    bc_write_to_chunk(buffer, (uint8_t *) &value, 1);
}

JumpPlaceholder bc_emit_jump_with_placeholder(BytecodeBuffer *buffer, Opcode opcode) {
    // The jump instruction consists of:
    // - opcode (1 byte)
    // - target chunk_id (size_t)
    // - target offset (size_t)

    size_t total_size = 1 + sizeof(size_t) + sizeof(size_t);

    // Ensure the entire jump instruction fits in the same chunk
    bc_ensure_chunk_capacity(buffer, total_size);

    // Record the current chunk and offset for the placeholder
    JumpPlaceholder placeholder;
    placeholder.chunk = buffer->current_chunk;
    placeholder.offset = buffer->current_chunk->size;

    // Begin atomic emission
    // Emit opcode
    bc_write_to_chunk(buffer, (uint8_t *) &opcode, 1);

    // Emit placeholder for chunk_id
    size_t placeholder_chunk_id = 0; // Placeholder value
    bc_write_to_chunk(buffer, (uint8_t *) &placeholder_chunk_id, sizeof(size_t));

    // Emit placeholder for offset
    size_t placeholder_offset = 0; // Placeholder value
    bc_write_to_chunk(buffer, (uint8_t *) &placeholder_offset, sizeof(size_t));

    return placeholder;
}

void bc_backpatch_jump(JumpPlaceholder placeholder, size_t target_chunk_id, size_t target_offset) {
    // The placeholders are at positions:
    // - placeholder.offset + 1 (for chunk_id)
    // - placeholder.offset + 1 + sizeof(size_t) (for offset)

    size_t chunk_id_pos = placeholder.offset + 1;
    size_t offset_pos = placeholder.offset + 1 + sizeof(size_t);

    // Write the target_chunk_id into the chunk
    memcpy(&placeholder.chunk->bytecode[chunk_id_pos], &target_chunk_id, sizeof(size_t));

    // Write the target_offset into the chunk
    memcpy(&placeholder.chunk->bytecode[offset_pos], &target_offset, sizeof(size_t));
}

void bc_emit_opcode_with_jump(BytecodeBuffer *buffer, Opcode opcode, size_t chunk_id, size_t offset) {
    size_t total_size = 1 + sizeof(size_t) * 2; // Opcode + chunk_id + offset

    bc_ensure_chunk_capacity(buffer, total_size);

    // Write the opcode
    bc_write_to_chunk(buffer, (uint8_t *)&opcode, 1);

    // Write the chunk_id
    bc_write_to_chunk(buffer, (uint8_t *)&chunk_id, sizeof(size_t));

    // Write the offset
    bc_write_to_chunk(buffer, (uint8_t *)&offset, sizeof(size_t));
}

void bc_write_byte(BytecodeChunk **chunk, uint8_t byte) {
    (*chunk)->bytecode[(*chunk)->size++] = byte;
}

void bc_write_ptr(BytecodeChunk **chunk, uintptr_t ptr) {
    size_t ptr_size = sizeof(uintptr_t);
    memcpy(&(*chunk)->bytecode[(*chunk)->size], &ptr, ptr_size);
    (*chunk)->size += ptr_size;
}

BytecodeChunk *bc_get_chunk_by_id(BytecodeBuffer *buffer, size_t chunk_id) {
    BytecodeChunk *chunk = buffer->head;
    while (chunk != NULL) {
        if (chunk->chunk_id == chunk_id) {
            return chunk;
        }
        chunk = chunk->next;
    }
    return nullptr;
}

void bc_emit_opcode_with_uint16(BytecodeBuffer *buffer, Opcode opcode, uint16_t value) {
    size_t total_size = 1 + sizeof(uint16_t);
    bc_ensure_chunk_capacity(buffer, total_size);
    bc_write_to_chunk(buffer, (uint8_t *)&opcode, 1);
    bc_write_to_chunk(buffer, (uint8_t *)&value, sizeof(uint16_t));
}

void bc_start_non_linked_chunk(BytecodeBuffer *buffer) {
    const auto linked_with = buffer->next_chunk_id + 1;
    buffer->return_to = buffer->current_chunk;

    // find to the first linked chunk
    const auto current = buffer->current_chunk;
    while (current->prev) {
        if (current->is_linked) {
            buffer->current_chunk = current;
            break;
        }
    }

    // skip non linked chunk
    buffer->current_chunk->linked_with = linked_with;

    bc_add_new_chunk(buffer, INITIAL_CHUNK_CAPACITY, false);
}

BytecodeChunk * bc_end_non_linked_chunk(BytecodeBuffer *buffer) {
    auto no_linked_chunk = buffer->current_chunk;
    bc_return_to(buffer);

    return no_linked_chunk;
}

void bc_return_to(BytecodeBuffer *buffer) {
    buffer->current_chunk = buffer->return_to;
}

