#pragma once

#include "data-structures.h"

object* extend_environment(object* args, object* cont);
object* find_in_environment(object* env, object* symbol);
