#pragma once

#include "data-structures.h"

void init_repl_procedures(void);

object* repl_read_entry(object* args, object* cont);