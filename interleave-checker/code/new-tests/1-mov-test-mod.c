// run N copies of the same thread together.
// #include "rpi.h"
// #include "eqx-threads.h"
#include "expected-hashes.h"
#include "interleave-checker.h"

void notmain(void) {
    checker_config_t c = {
        .A = mov_ident,
        .B = mov_ident,
        .argA = 0,
        .argB = 0,
        .n_copies = 19,
        .enable_stack = 0,
        .max_num_inst = 1000,
        .verbosity = 0,
        .enable_vm = 1
    };
    int res = simple_interleave_check(c);
}