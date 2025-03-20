// #include "rpi.h"
// #include "eqx-threads.h"
#include "expected-hashes.h"
#include "interleave-checker.h"

// Test 2: With VM but not all caches enabled
void test_vm_no_cache(void) {
    interleave_opt_t opt = {
        .enable_stack = 0,
        .max_num_inst = 2000,
        .verbosity = 0,
        .non_interleave_type = RANDOM,
        .random_seed = 12,
        .enable_fnames = 0,
        .enable_vm = 1,
        .enable_all_caches = 0
    };

    void (*fcns[])(void *) = {nop_1, nop_10, mov_ident};
    void *args[] = {0, 0, 0};
    interleave_multiple(fcns, args, 3, opt);
}

void notmain(void) {
    test_vm_no_cache();
    trace("all no-stack passed\n ========== \n");
}
