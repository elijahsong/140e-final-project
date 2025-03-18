// interleave-checker like lab 10 but using lab 12 code modified
#include "rpi.h"
#include "eqx-threads.h"
#include "breakpoint.h"
#include "full-except.h"
#include "fast-hash32.h"
#include "eqx-syscalls.h"
#include "cpsr-util.h"


typedef struct checker_config {
    void (*A)(void *);
    void (*B)(void *);
    int n_copies;
    int stack_nbytes;
    int max_num_inst;
    int verbosity;
} checker_config_t;

int interleave_check(checker_config_t c);