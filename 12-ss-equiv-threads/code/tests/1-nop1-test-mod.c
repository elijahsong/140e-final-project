// run N copies of the same thread together.
#include "rpi.h"
#include "eqx-threads.h"
#include "expected-hashes.h"
#include "interleave-checker.h"

enum { N = 10 };

void notmain(void) {
    checker_config_t c = {
        .A = nop_1,
        .B = nop_1,
        .argA = 0,
        .argB = 0,
        .n_copies = 19,
        .enable_stack = 0,
        .max_num_inst = 10,
        .verbosity = 0
    };
    int res = interleave_check(c);
}