//
// Created by fathi on 10/29/2024.
//

#include <stdio.h>
#include <stdlib.h>
#include "object.h"

void object_free(Object *ptr) {
    printf("TODO: free the full object");
    free(ptr->metadata);
    free(ptr->props);
}
