//
// Created by fathi on 10/21/2024.
//

#ifndef TIGE_OPCODE_H
#define TIGE_OPCODE_H
typedef enum {
    OP_NOPE = 0x00,
    // Stack Manipulation
    OP_LOAD_CONST_INT = 0x01,
    OP_LOAD_CONST_FLOAT = 0x2D,
    OP_LOAD_VAR = 0x02,
    OP_STORE_VAR = 0x03,

    // Arithmetic Operations
    OP_ADD = 0x04,
    OP_SUB = 0x05,
    OP_MUL = 0x06,
    OP_DIV = 0x07,

    // Logical Operations
    OP_AND = 0x08,
    OP_OR = 0x09,
    OP_NOT = 0x0A,

    // Comparison Operations
    OP_EQUAL = 0x0B,
    OP_NOT_EQUAL = 0x0C,
    OP_LESS_THAN = 0x0D,
    OP_GREATER_THAN = 0x0E,
    OP_LESS_EQUAL = 0x0F,
    OP_GREATER_EQUAL = 0x10,

    // Control Flow
    OP_JMP = 0x11,
    OP_JMP_IF_TRUE = 0x12,
    OP_JMP_IF_FALSE = 0x13,

    // Function Calls
    OP_CALL = 0x14,
    OP_RETURN = 0x15,

    // Object Handling
    OP_NEW_OBJECT = 0x16,
    OP_GET_PROPERTY = 0x17,
    OP_SET_PROPERTY = 0x18,

    // Heap Management
    OP_ALLOC_HEAP = 0x19,
    OP_FREE_HEAP = 0x1A,

    // Data Types
    OP_LOAD_STRING = 0x1B,
    OP_LOAD_BOOL = 0x1C,

    // Ternary Operator
    OP_TERNARY = 0x1D,

    OP_JMP_ADR = 0x1E,

    OP_ENTER_SCOPE = 0x1F,
    OP_EXIT_SCOPE = 0xF1,

    OP_PUSH = 0xF2,
    OP_POP = 0xF3,

    OP_SAVE_SP = 0xF4,
    OP_RESET_SP = 0xF5,

    // Halt Execution
    OP_HALT = 0xFF,

} Opcode;

#endif //TIGE_OPCODE_H
