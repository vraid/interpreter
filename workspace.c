#include "workspace.h"

#include <stdlib.h>
#include "data-structures.h"

char* _workspace;
char* workspace_next;
int workspace_size;

char* workspace(void) {
	return _workspace;
}

void resize_workspace(int size) {
	free(_workspace);
	_workspace = malloc(size);
	printf("workspace at %p\n", (void*)(_workspace));
	workspace_size = size;
}

void reset_workspace(int size) {
	if (size > workspace_size) {
		int double_size = 2 * workspace_size;
		resize_workspace(size > double_size ? size : double_size);
	}
	workspace_next = _workspace;
}

char* workspace_allocate(int size) {
	char* result = workspace_next;
	workspace_next += size;
	if (workspace_next > (_workspace + workspace_size)) {
		printf("workspace memory exceeded\n");
		exit(1);
	}
	return result;
}

void init_workspace(void) {
	_workspace = NULL;
	resize_workspace(sizeof(object) * 1024);
	reset_workspace(workspace_size);
}
