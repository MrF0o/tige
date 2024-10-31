//
// Created by fathi on 10/29/2024.
//

#ifndef TIGE_OBJECT_H
#define TIGE_OBJECT_H

#include "value.h"

typedef struct Object Object;
typedef struct ObjectProperty ObjectProperty;
typedef struct ObjectMetadata ObjectMetadata;

struct ObjectMetadata {};

struct ObjectProperty {
    char* prop_name;
    Value value;
};

struct ObjectPropMap {
    ObjectProperty* properties;
};

struct Object {
    ObjectMetadata* metadata;
    ObjectProperty* props;
};

Object* object_new();
void object_free(Object* ptr);

// functions are just object without any property or access to this
Object* make_function();

#endif //TIGE_OBJECT_H
