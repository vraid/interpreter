#include "workspace.h"

#include <stdlib.h>
#include "data-structures.h"

char* _workspace;
int workspace_size;

char* workspace(void) {
	return _workspace;
}

void resize_workspace(int size) {
	free(_workspace);
	_workspace = malloc(size);
	workspace_size = size;
}

void set_workspace_min_size(int size) {
	if (size > workspace_size) {
		int double_size = 2 * workspace_size;
		resize_workspace(size > double_size ? size : double_size);
	}
}

void init_workspace(void) {
	_workspace = NULL;
	resize_workspace(sizeof(object) * 1024);
}
