#include "interleave-checker.h"

int states[5] = {0}; // Array to track individual states

void transition_1(void *arg) {
    states[0] = 1;
}

void transition_2(void *arg) {
    states[1] = states[0] + 1;
}

void transition_3(void *arg) {
    states[2] = states[1] + 1;
}

void transition_4(void *arg) {
    states[3] = states[2] + 1;
}

void transition_5(void *arg) {
    states[4] = states[3] + 1;
}


void notmain(void) {
    interleave_opt_t opt = {
        .enable_stack = 1,
        .max_num_inst = 1000,
        .verbosity = 1,
        .non_interleave_type = RANDOM,
        .random_seed = 789,
        .enable_vm = 0,
        .enable_all_caches = 0
    };

    void (*functions[])(void *) = {transition_1, transition_2, transition_3, transition_4, transition_5};
    void *args[] = {0, 0, 0};

    interleave_multiple(functions, args, 5, opt);

    if (states[4] == 5) {
        trace("Success: Final state is correct! current_state=%d\n", states[4]);
    } else {
        trace("Error: Final state is incorrect! current_state=%d\n", states[4]);
    }
}
