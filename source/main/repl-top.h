#pragma once

#include "data-structures.h"

object repl_read_entry_proc;
object repl_init_proc;

void init_repl_procedures(void);

object* repl_read_entry(object* args, object* cont);
