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
    void *argA;
    void *argB;
    int n_copies;
    int enable_stack;
    int max_num_inst;
    int verbosity;
} checker_config_t;

// Run the function A interleaved with `n_copies` of B 
int interleave_check(checker_config_t c);

// Run the primary function A interleaved with B, C, D, ...
// Then B interleaved with A, C, D
// Then C interleaved with A, B, D
// Then D interleaved with A, B, C
int interleave_multiple(void (**funcArr)(void *), void **args, int n_fns, int enable_stack, int max_num_inst, int verbosity);
