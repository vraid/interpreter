#include "util.h"
#include <stdlib.h>
#include <stdio.h>

long string_to_int(char* str) {
	long value = 0;
	int i = 0;
	while (str[i] != 0) {
		value *= 10;
		value += (str[i] - '0');
		i++;
	}
	return value;
}
