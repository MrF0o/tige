//
// Created by mrfoo on 11/3/24.
//

#include "garbage_collector.h"
#include "object.h"
#include <stdlib.h>

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
