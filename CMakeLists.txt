cmake_minimum_required(VERSION 3.27)
project(tige C)

set(CMAKE_C_STANDARD 23)

set(DEBUG_OPTIONS
        -std=c2x
        -g
        -Wall
        -Wextra
        -DDEBUG
        -O0
)

set(RELEASE_OPTIONS
        -std=c2x
        -O03
        -DNDEBUG
        -Wall
        -Wextra
        -Werror
        -march=native
        -flto
        -ffast-math
        -fno-unroll-loops
        -fprofile-use
)

add_compile_definitions(LEXER_DEBUG)

add_executable(tige main.c
        lexer.c
        parser.c
        ast.c
        evaluator.c
        context.c
        error.c
        symbol_table.c
        memory.h
        memory.c
        compiler.c
        bytecode_buffer.c
        vm.c
        op_handlers.c
        value.c
        object.c
        functions.c)

target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:${DEBUG_OPTIONS}>")
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:${RELEASE_OPTIONS}>")