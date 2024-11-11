//
// Created by mrfoo on 11/3/24.
//

#ifndef TIGE_STRING
#define TIGE_STRING

#include <uchar.h>

typedef struct TObjectMetadata TObjectMetadata;
typedef struct TObjectProperty TObjectProperty;
typedef struct TString TString;

struct TString
{
    TObjectMetadata* metadata;
    TObjectProperty* props;
    char* chars;
};

TString* new_string(void);
void free_string(TString* str);

size_t string_length(TString* str);
bool string_equals(const TString* str1, const TString* str2);
TString* string_copy(const TString* str);
TString* string_concat(TString* str1, TString* str2);
TString* string_at(TString* str, size_t index);
TString* string_slice(TString* str, size_t start, size_t end);

#endif //TIGE_STRING
