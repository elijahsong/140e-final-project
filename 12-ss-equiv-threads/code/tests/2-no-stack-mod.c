#include "rpi.h"
#include "eqx-threads.h"
#include "expected-hashes.h"
#include "interleave-checker.h"


void notmain(void) {
    // // Run `nop1`, `nop10`, `mov_ident`
    // void (*fcns[])(void *) = {nop_1, nop_10, mov_ident};
    // void *args[] = {0, 0, 0};
    // interleave_multiple(fcns, args, 3, 2500, 0, 0);

    // trace("second no-stack passed\n ========== \n");

    // run all together: interleaving all 5 threads.
    // void (*fcns_b[])(void *) = {small1, small2, nop_1, nop_10, mov_ident};
    void (*fcns_b[])(void *) = {mov_ident, small1, small2, nop_1, nop_10};
    void *args_b[] = {0, 0, 0, 0, 0};
    interleave_multiple(fcns_b, args_b, 5, 2500, 0, 0);
    
    trace("all no-stack passed\n ========== \n");

    // void (*fcns_c[])(void *) = {small2, small1, nop_1, nop_10, mov_ident};
    // void *args_c[] = {0, 0, 0, 0, 0};
    // interleave_multiple(fcns_c, args_c, 5, 2500, 0, 0);
    
    // trace("all no-stack passed\n ========== \n");
}
