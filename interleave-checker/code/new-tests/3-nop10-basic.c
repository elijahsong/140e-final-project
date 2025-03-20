// run N copies of the same thread together.
// #include "rpi.h"
// #include "eqx-threads.h"
#include "expected-hashes.h"
#include "interleave-checker.h"

enum { N = 10 };

void test_no_vm_no_cache(void) {
    checker_config_t c = {
        .A = nop_10,
        .B = nop_10,
        .argA = 0,
        .argB = 0,
        .n_copies = 19,
        .enable_stack = 0,
        .max_num_inst = 10,
        .verbosity = 0,
        .enable_vm = 0,
        .enable_all_caches = 0
    };
    int res = simple_interleave_check(c);
}

void notmain(void) {
    test_no_vm_no_cache();
}
