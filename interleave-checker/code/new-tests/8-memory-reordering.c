#include "expected-hashes.h"
#include "interleave-checker.h"

#define NUM_FUNCS 2
#define NUM_CTX 3

int* flag1;
int* flag2;
int* data1;
int* data2;

void my_mov_ident(void *arg) {
    *data1 = 42;           // Store data
    *flag1 = 1;            // Set flag
}

void my_small1(void *arg) {
    *data2 = 100;          // Store data
    *flag2 = 1;            // Set flag
}

// reordering
void my_nop_10(void *arg) {
    if (*flag1 == 1 && *flag2 == 1) {
        // Both flags set, check if we can see both data values
        if (*data1 != 42 || *data2 != 100) {
            // This would indicate memory reordering!
        }
    }
}

void notmain(void) {
    interleave_opt_t opt = {
        .enable_stack = 1,
        .max_num_inst = 2000,
        .verbosity = 1,
        .non_interleave_type = RANDOM,
        .random_seed = 42,
        .enable_vm = 1,
        .enable_all_caches = 1
    };

    void (*fcns[])(void *) = {my_mov_ident, my_small1, my_nop_10};
    void *args[] = {0, 0, 0};
    interleave_multiple(fcns, args, 3, opt);
     trace("stack passed\n ========== \n");
}
