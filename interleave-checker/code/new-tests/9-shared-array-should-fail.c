#include "expected-hashes.h"
#include "interleave-checker.h"

int* shared_array;
int* result1;
int* result2;

void my_nop_1(void *arg) {
    int sum = 0;
    for (int i = 0; i < 10; i += 2) {
        sum += shared_array[i];
        // potential race condition
        shared_array[i]++;
    }
    *result1 = sum;
}

void my_nop_10(void *arg) {
    int sum = 0;
    for (int i = 1; i < 10; i += 2) {
        sum += shared_array[i];
        // potential race condition
        shared_array[i]++;
    }
    *result2 = sum;
}

void notmain(void) {
    interleave_opt_t opt = {
        .enable_stack = 1,
        .max_num_inst = 5000,
        .verbosity = 1,
        .non_interleave_type = RANDOM,
        .random_seed = 123,
        .enable_vm = 1,
        .enable_all_caches = 1
    };

    void (*fcns[])(void *) = {my_nop_1, my_nop_10};
    void *args[] = {0, 0};
    interleave_multiple(fcns, args, 2, opt);
}