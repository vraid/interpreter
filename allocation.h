#pragma once

#include "data-structures.h"

object* allocate_object_type(object_type type);
object* allocate_list_type(bracket_type type);
object* allocate_list(void);
