#pragma once

#include "data-structures.h"

void init_object(object_location loc, object_type type, object* obj);
object* allocate_object(object_type type);

object* new_list(void);
