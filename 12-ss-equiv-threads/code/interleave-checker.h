// interleave-checker like lab 10 but using lab 12 code modified
#include "rpi.h"
#include "eqx-threads.h"
#include "breakpoint.h"
#include "full-except.h"
#include "fast-hash32.h"
#include "eqx-syscalls.h"
#include "cpsr-util.h"


int interleave_check(void (*A)(void*), void (*B)(void*), int N, int nbytes, int max_num_inst);