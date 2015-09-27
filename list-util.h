#pragma once

#include "data-structures.h"

object* cons(object* first, object* rest);
object* list_ref(int n, object* ls);
object* list_take(int n, object* obj);
object* list_drop(int n, object* obj);
object* list_append(object* as, object* rest);
object* list_reverse(object* obj);
