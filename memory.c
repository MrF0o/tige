//
// Created by fathi on 10/21/2024.
//

#include "context.h"
#include "memory.h"

void* tige_alloc(Context* ctx, size_t size) {
    ctx->total_mem += size;
    return malloc(size);
}

void tige_free(Context* ctx, void* ptr) {
    if (ctx->total_mem > 0) {
        free(ptr);
    }
}