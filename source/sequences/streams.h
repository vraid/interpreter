#pragma once

#include "data-structures.h"

object make_stream_proc;
object eval_stream_rest_proc;
object stream_map_proc;
object stream_filter_proc;
object stream_fold_proc;

void init_stream_procedures(void);
