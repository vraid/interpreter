#pragma once

#include <stdio.h>

int current_read_state(void);

char get_input(FILE* in);
void unget_input(FILE* in);

void reset_read_state(void);
