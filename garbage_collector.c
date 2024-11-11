//
// Created by mrfoo on 11/3/24.
//

#include "garbage_collector.h"
#include "object.h"
#include <stdlib.h>
#define MEMORY_THRESHOLD (1024 * 4)

void mark_object(TObject* obj);
void mark_value(Value value);
void mark_roots(VM* vm);
void sweep(Heap* heap);

void collect_garbage(VM* vm) {
    mark_roots(vm);
    sweep(vm->heap);
}


void gc(VM* vm)
{
    mark_roots(vm);
    sweep(vm->heap);
}

void maybe_gc(VM* vm)
{

}

void mark_object(TObject* obj)
{
}

void mark_value(Value value)
{
}

void mark_roots(VM* vm)
{
}

void sweep(Heap* heap) {
    // TODO
}
