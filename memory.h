//
// Created by fathi on 10/21/2024.
//

#ifndef TIGE_MEMORY_H
#define TIGE_MEMORY_H

#include <stdlib.h>
#include <memory.h>

typedef struct Context Context;

void* tige_alloc(Context* ctx, size_t size);

void tige_free(Context* ctx, void* ptr);

#endif //TIGE_MEMORY_H
