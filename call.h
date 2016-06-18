#pragma once

#include "data-structures.h"

object* top_call(object* call);
object* perform_call(object* call);
object* throw_error(object* cont, char* message);
object* call_cont(object* cont, object* arg);
object* call_discarding_cont(object* cont);
