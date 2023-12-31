#pragma once

#include "data-structures.h"

object* top_call(object* call);
object* perform_call(object* call);
object* throw_error(object* cont, object* message);
object* throw_error_string(object* cont, char* str);
object* throw_trace_error(object* cont, object* trace, object* message);
object* throw_trace_error_string(object* cont, object* trace, char* str);
object* call_cont(object* cont, object* arg);
object* call_discarding_cont(object* cont);
