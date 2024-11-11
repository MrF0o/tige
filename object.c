//
// Created by fathi on 10/29/2024.
//

#include <stdio.h>
#include <stdlib.h>
#include "object.h"

#include "memory.h"

void object_free(const TObject *ptr) {
    printf("TODO: free the full object");
    free(ptr->metadata);
    free(ptr->props);
}

void object_init(TObject* ptr)
{
    ptr->metadata = (TObjectMetadata *)vm_malloc(sizeof(TObjectMetadata));
    // TODO: initialize object metadata

    printf("Is marked %s ", ptr->metadata->gc_marked ? "true" : "false");
}
