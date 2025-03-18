// run N copies of the same thread together.
#include "rpi.h"
#include "eqx-threads.h"
#include "expected-hashes.h"
#include "interleave-checker.h"

enum { N = 10 };

void notmain(void) {
    checker_config_t c = {
        .A = nop_10,
        .B = nop_10,
        .n_copies = 20,
        .stack_nbytes = 0,
        .max_num_inst = 10,
        .verbosity = 0
    };
    int res = interleave_check(c);
    output("success !!!\n");
}