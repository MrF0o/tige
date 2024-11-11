//
// Created by mrfoo on 11/3/24.
//

#include "tige_string.h"

#include "object.h"
#include <stdlib.h>
#include <string.h>

TString* new_string(void)
{
    const auto string = (TString*)vm_malloc(sizeof(TString));
    if (!string)
    {
        vm_free(string);
        return nullptr;
    }

    object_init((TObject*)string);
    return string;
}

void free_string(TString* str)
{
    free(str->chars);
    object_free((TObject*)str);
}

TString* string_copy(const TString* str)
{
    const auto string = new_string();
    string->chars = str->chars;
    return string;
}

bool string_equals(const TString* str1, const TString* str2)
{
    return strcmp(str1->chars, str2->chars) == 0;
}
