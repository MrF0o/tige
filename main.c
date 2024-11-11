#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "context.h"
#include "evaluator.h"
#include "ast_utils.h"
#include "bytecode_buffer.h"
#include "compiler.h"
#include "vm.h"

// Read entire file into memory
char *read_file(const char *filename, size_t *file_size) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error: Could not open file '%s'\n", filename);
        return NULL;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    *file_size = ftell(file);
    rewind(file);

    char *buffer = (char *) malloc(*file_size + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for file content\n");
        fclose(file);
        return nullptr;
    }

    size_t bytes_read = fread(buffer, 1, *file_size, file);
    if (bytes_read < *file_size) {
        fprintf(stderr, "Error: Could not read entire file\n");
        free(buffer);
        fclose(file);
        return nullptr;
    }

    buffer[*file_size] = '\0';

    fclose(file);
    return buffer;
}

const char* opcode_to_mnemonic(Opcode opcode) {
    switch (opcode) {
        case OP_LOAD_CONST_INT:      return "LDI";
        case OP_LOAD_CONST_FLOAT:    return "LDF";
        case OP_LOAD_BOOL:           return "LDZ";
        case OP_LOAD_VAR:            return "LD";
        case OP_STORE_VAR:           return "STORE";
        case OP_ADD:                 return "ADD";
        case OP_SUB:                 return "SUB";
        case OP_MUL:                 return "MUL";
        case OP_DIV:                 return "DIV";
        case OP_EQUAL:               return "EQ";
        case OP_NOT_EQUAL:           return "NEQ";
        case OP_LESS_THAN:           return "LT";
        case OP_GREATER_THAN:        return "GT";
        case OP_LESS_EQUAL:          return "LTE";
        case OP_GREATER_EQUAL:       return "GTE";
        case OP_JMP_IF_FALSE:        return "JZ";  // Jump if Zero (false)
        case OP_JMP_IF_TRUE:         return "JNZ"; // Jump if Not Zero (true)
        case OP_JMP:                 return "JMP";
        case OP_POP:                 return "POP";
        case OP_HALT:                return "HALT";
        case OP_NOT:                 return "NOT";
        case OP_RETURN:              return "RET";
        default:                     return "UNKNOWN";
    }
}

void disassemble_chunk(BytecodeChunk* chunk) {
    if (!chunk || !chunk->bytecode) {
        fprintf(stderr, "Error: Invalid BytecodeChunk.\n");
        return;
    }

    size_t offset = 0;

    while (offset < chunk->size) {
        Opcode opcode = chunk->bytecode[offset];
        const char* mnemonic = opcode_to_mnemonic(opcode);
        size_t instruction_offset = offset; // Current instruction offset within chunk

        switch (opcode) {
            case OP_LOAD_CONST_INT: {
                if (offset + 4 >= chunk->size) {
                    fprintf(stderr, "Error: Unexpected end of bytecode at chunk %zu, offset 0x%02zx\n",
                            chunk->chunk_id, offset);
                    return;
                }
                int64_t value;
                memcpy(&value, chunk->bytecode + offset + 1, sizeof(int64_t));
                printf("0x%02zx %-10s %lld\n", instruction_offset, mnemonic, value);
                offset += 1 + sizeof(int64_t);
                break;
            }
            case OP_LOAD_CONST_FLOAT: {
                if (offset + 4 >= chunk->size) {
                    fprintf(stderr, "Error: Unexpected end of bytecode at chunk %zu, offset 0x%02zx\n",
                            chunk->chunk_id, offset);
                    return;
                }
                double value;
                memcpy(&value, chunk->bytecode + offset + 1, sizeof(double));
                printf("0x%02zx %-10s %f\n", instruction_offset, mnemonic, value);
                offset += 1 + sizeof(double); // opcode + operand
                break;
            }
            case OP_LOAD_BOOL: {
                if (offset + 1 >= chunk->size) {
                    fprintf(stderr, "Error: Unexpected end of bytecode at chunk %zu, offset 0x%02zx\n",
                            chunk->chunk_id, offset);
                    return;
                }
                unsigned char value = chunk->bytecode[offset + 1];
                printf("0x%02zx %-10s %s\n", instruction_offset, mnemonic, value ? "true" : "false");
                offset += 1 + 1;
                break;
            }
            case OP_LOAD_VAR:
            case OP_STORE_VAR: {
                if (offset + 2 >= chunk->size) {
                    fprintf(stderr, "Error: Unexpected end of bytecode at chunk %zu, offset 0x%02zx\n",
                            chunk->chunk_id, offset);
                    return;
                }
                uint16_t reg_index;
                memcpy(&reg_index, chunk->bytecode + offset + 1, sizeof(uint16_t));
                printf("0x%02zx %-10s r%u\n", instruction_offset, mnemonic, reg_index);
                offset += 1 + sizeof(uint16_t);
                break;
            }
            case OP_JMP_IF_FALSE:
            case OP_JMP: {
                if (offset + 2 >= chunk->size) {
                    fprintf(stderr, "Error: Unexpected end of bytecode at chunk %zu, offset 0x%02zx\n",
                            chunk->chunk_id, offset);
                    return;
                }
                size_t target_offset;
                memcpy(&target_offset, chunk->bytecode + offset + 1 + sizeof(size_t), sizeof(size_t));
                printf("0x%02zx %-10s 0x%02llx\n", instruction_offset, mnemonic, target_offset);
                offset += 1 + sizeof(size_t) * 2;
                break;
            }
            case OP_ADD:
            case OP_SUB:
            case OP_MUL:
            case OP_DIV:
            case OP_EQUAL:
            case OP_NOT_EQUAL:
            case OP_LESS_THAN:
            case OP_GREATER_THAN:
            case OP_LESS_EQUAL:
            case OP_GREATER_EQUAL:
            case OP_NOT:
            case OP_POP:
            case OP_HALT:
            case OP_RETURN: {
                printf("0x%02zx %-10s\n", instruction_offset, mnemonic);
                offset += 1; // opcode
                break;
            }
            default: {
                printf("0x%02zx %-10s (Unknown Opcode: %u)\n", instruction_offset, mnemonic, opcode);
                offset += 1; // opcode
                break;
            }
        }
    }
}

void disassemble_bytecode(BytecodeBuffer* buffer) {
    if (!buffer) {
        fprintf(stderr, "Error: Invalid BytecodeBuffer.\n");
        return;
    }

    printf("[CODE]\n");

    // Iterate through all chunks in order
    BytecodeChunk* current = buffer->head;
    while (current) {
        disassemble_chunk(current);
        current = current->next;
    }
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <source_file>\n", argv[0]);
        return 1;
    }

    // Read the source file
    size_t file_size;
    char *source = read_file(argv[1], &file_size);
    if (source == nullptr) {
        return 1;
    }

    Context context;
    ctx_init(&context, source);


    // configs:
    // ctx_set_strict_mode(&context, false);
    // ctx_set_exit_on_parse_errors(&context, false);
    // ctx_set_declare_vars_in_parser(&context, false);
    // ctx_set_use_token_stream(&context, false);
    // ctx_set_compile_after_parse(&context, true);
    // ctx_set_execute_after_compile(&context, false);
    // ctx_set_vm_max_memory(&context, 1024);
    // ctx_set_vm_debug(&context, true);

    if (ctx_is_initialized(&context)) {
        // add the print function
        auto print_fn = create_function();
        print_fn->arity = 1;
        print_fn->name = "print";
        register_function(&context, "print", print_fn);

        ctx_start_parsing(&context);

        if (ctx_check_errors(&context)) {
            // TODO: print errors and exit
            ctx_clean_parse_info(&context);
            return -1;
        }

        if (!ctx_is_vm_initialized(&context)) {
            ctx_create_vm(&context);
        }

        VM* vm = ctx_get_active_vm(&context);
        BytecodeBuffer* buffer;
        ctx_get_compiled_code(&context, &buffer);

        if (vm && bc_is_buffer_valid(buffer)) {
            vm_swap_code_buffer(vm, buffer);
            auto value = vm_execute(vm);
        }
        // printf("\n\n##disassembly##\n\n");
        // disassemble_bytecode(buffer);
    }

    // Cleanup
    ctx_destroy(&context);
    return 0;
}
