#pragma once

#include "data-structures.h"

void init_object(object_location loc, object_type type, object* obj);
object* allocate_object_type(object_type type);
object* allocate_list_type(bracket_type type);
object* allocate_list(void);
object* allocate_object_boolean(char value);
