//
// Created by fathi on 10/29/2024.
//

#ifndef TIGE_OBJECT_H
#define TIGE_OBJECT_H

#include "value.h"

typedef struct Object Object;
typedef struct ObjectProperty ObjectProperty;

struct ObjectProperty {
    char* prop_name;
    Value value;
};

struct ObjectPropMap {
    struct ObjectProperty** properties;
};

struct Object {

};

Object* object_new();
Object* object_free();

// functions are just object without any property or access to this
Object* make_function();

#endif //TIGE_OBJECT_H
