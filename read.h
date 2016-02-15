#pragma once

#include <stdio.h>
#include "data-structures.h"

typedef enum {
	read_error_none,
	read_error_some } read_error_type;

typedef struct {
	read_error_type type;
	int row;
	int column;
	char* message;
} read_error;

typedef struct {
	int char_index;
	char is_line_comment;
} read_state;

typedef struct {
	FILE* in;
	read_state state;
} reader_object;

char reader_next(reader_object* reader);
object* read(FILE* in);