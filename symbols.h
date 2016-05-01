#pragma once

#include "data-structures.h"

void init_symbols(void);
void init_symbol(object* obj, char* name);
void add_symbol(object* obj);
void make_static_symbol(object* obj, char* name);
object* symbol(char* name);