#include "interleave-checker.h"

int add_result = 0;
int multiply_result = 0;

void add(void *arg) {
    // Perform addition independently
    add_result = 5 + 3; // Example: 5 + 3
}

void multiply(void *arg) {
    // Perform multiplication independently
    multiply_result = 4 * 2; // Example: 4 * 2
}

void notmain(void) {
    interleave_opt_t opt = {
        .enable_stack = 1,
        .max_num_inst = 1000,
        .verbosity = 1,
        .non_interleave_type = RANDOM,
        .random_seed = 456,
        .enable_vm = 1,
        .enable_all_caches = 1
    };

    void (*functions[])(void *) = {add, multiply};
    void *args[] = {0, 0, 0};

    interleave_multiple(functions, args, 2, opt);

    if (add_result == 8 && multiply_result == 8) {
        trace("Success: Results are correct! add_result=%d, multiply_result=%d\n", add_result, multiply_result);
    } else {
        trace("Error: Unexpected results! add_result=%d, multiply_result=%d\n", add_result, multiply_result);
    }
}
