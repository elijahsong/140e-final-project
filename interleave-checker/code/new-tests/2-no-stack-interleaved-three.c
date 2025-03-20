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

    // Run `nop1`, `nop10`, `mov_ident`
    void (*fcns[])(void *) = {nop_1, nop_10, mov_ident};
    void *args[] = {0, 0, 0};
    // opt.fnames = {"nop_1", "nop_10", "mov_ident"};
    // opt.enable_fnames = 1;
    interleave_multiple(fcns, args, 3, opt);
}
