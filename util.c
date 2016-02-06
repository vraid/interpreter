#include "util.h"
#include <stdlib.h>
#include <stdio.h>

char* int_to_string(int i) {
	int k = 10;
	int count = 1;
	while (k >= i) {
		k *= 10;
		count++;
	}
	
	char* c = malloc(sizeof(char) * (count + 1));
	sprintf(c, "%d", i);
	return c;
}