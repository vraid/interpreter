#include "read-state.h"

#include <stdio.h>

int curr_read_state;
int last_read_state;

int current_read_state(void) {
	return curr_read_state;
}

void reset_read_state(void) {
	curr_read_state = 0;
}

void read_state_inc(char c) {
	curr_read_state += 1;
}

char last_read_char;

char get_input(FILE* in) {
	last_read_state = curr_read_state;
	last_read_char = getc(in);
	read_state_inc(last_read_char);
	return last_read_char;
}

void unget_input(FILE* in) {
	curr_read_state = last_read_state;
	ungetc(last_read_char, in);
}
