//
// Created by fathi on 10/29/2024.
//

#ifndef TIGE_OBJECT_H
#define TIGE_OBJECT_H

#include "functions.h"
#include "uthash.h"
#include "value.h"

typedef struct TObject TObject;
typedef struct TObjectProperty TObjectProperty;
typedef struct TObjectMetadata TObjectMetadata;
typedef struct TObjectMethod TObjectMethod;

struct TObjectMethod
{
    char *name;
    Function* function;
    UT_hash_handle hh;
};

struct TObjectMetadata
{
    bool gc_marked;

};

struct TObjectProperty {
    char* prop_name;
    Value value;
};

struct TObjectPropMap {
    TObjectProperty* properties;
};

struct TObject {
    TObjectMetadata* metadata;
    TObjectProperty* props;
};

TObject* object_new();
void object_free(const TObject* ptr);
void object_init(TObject* ptr);

// functions are just object without any property or access to this
TObject* make_function();

void object_add_property(TObject* ptr, TObjectProperty* property);
void object_add_method(TObject* ptr, TObjectMethod* method);
void object_set_method(TObject* ptr, TObjectMethod* method);
void object_set_property(TObject* ptr, TObjectProperty* property);
void object_remove_property(TObject* ptr, TObjectProperty* property);

#endif //TIGE_OBJECT_H
