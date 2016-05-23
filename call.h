#pragma once

#include "data-structures.h"

object* top_call(object* call);
object* perform_call(object* call);
object* call_cont(object* cont, object* arg);
object* call_discarding_cont(object* cont);