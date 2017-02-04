#pragma once

#include <stdio.h>

typedef struct {
	int x;
	int y;
} read_state;

read_state current_read_state(void);

char get_input(FILE* in);
void unget_input(FILE* in);

void reset_read_state(void);
