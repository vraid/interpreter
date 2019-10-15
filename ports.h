#pragma once

#include "data-structures.h"

int get_input(object* obj);
void unget_input(object* obj);
int peek(object* obj);

long port_position(object* obj);
