#pragma once

#include "data-structures.h"

object* symbol_list;

void init_symbols(void);
object* make_static_symbol(char* name);
object* symbol(char* name, object* cont);
