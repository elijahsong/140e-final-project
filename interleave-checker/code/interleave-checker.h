// interleave-checker like lab 10 but using lab 12 code modified
#include "rpi.h"
#include "eqx-threads.h"
#include "breakpoint.h"
#include "full-except.h"
#include "fast-hash32.h"
#include "eqx-syscalls.h"
#include "cpsr-util.h"


typedef struct interleave_opt {
    int enable_stack; // Whether to enable the stack
    int max_num_inst; // Limit on the number of instructions to interleave
    int verbosity;  
    int interleave_verbosity; // Limit the verbosity of interleave_x_with_others and related functions
    // The scheduling strategy used after the primary function X completes i instructions (see interleave_multiple)
    Scheduler non_interleave_type; // SEQUENTIAL, ROUND_ROBIN, EVERY_X, ALL_PATHS, RANDOM
    uint32_t random_seed; // Required if RANDOM enabled
    char **fnames;
    int enable_fnames;
    int enable_vm;
} interleave_opt_t;

typedef struct checker_config {
    void (*A)(void *);
    void (*B)(void *);
    void *argA;
    void *argB;
    int n_copies;
    int enable_stack;
    int max_num_inst;
    int verbosity;
    int enable_vm;
} checker_config_t;

// Run the function A interleaved with `n_copies` of B 
int simple_interleave_check(checker_config_t c);

// Run the primary function A interleaved with B, C, D, ...
// Then B interleaved with A, C, D
// Then C interleaved with A, B, D
// Then D interleaved with A, B, C
int interleave_multiple(void (**funcArr)(void *), void **args, int n_fns, interleave_opt_t opt);
