// #include "rpi.h"
// #include "eqx-threads.h"
#include "expected-hashes.h"
#include "interleave-checker.h"


void notmain(void) {
    interleave_opt_t opt = {
        .enable_stack = 0,
        .max_num_inst = 2000,
        .verbosity = 0,
        .non_interleave_type = RANDOM,
        .random_seed = 12,
        .enable_fnames = 0,
        .enable_vm = 0
    };

    // run all together: interleaving all 5 threads.
    // void (*fcns_b[])(void *) = {small1, small2, nop_1, nop_10, mov_ident};
    void (*fcns_b[])(void *) = {mov_ident, small1, small2, nop_1, nop_10};
    void *args_b[] = {0, 0, 0, 0, 0};
    // opt.fnames = {"mov_ident", "small1", "small2", "nop_1", "nop_10"};
    // opt.enable_fnames = 1;
    interleave_multiple(fcns_b, args_b, 5, opt);
    
    trace("all no-stack passed\n ========== \n");
}
