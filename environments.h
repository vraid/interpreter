#pragma once

#include "data-structures.h"

object* extend_environment(object* env, object* bindings);
object* find_in_environment(object* env, object* symbol);
