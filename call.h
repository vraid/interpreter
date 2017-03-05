#pragma once

#include "data-structures.h"

object* top_call(object* call);
object* perform_call(object* call);
object* throw_error(object* cont, object* message);
object* throw_error_string(object* cont, char* str);
object* throw_origin_error(object* cont, object* origin, object* message);
object* throw_origin_error_string(object* cont, object* origin, char* str);
object* call_cont(object* cont, object* arg);
object* call_discarding_cont(object* cont);
