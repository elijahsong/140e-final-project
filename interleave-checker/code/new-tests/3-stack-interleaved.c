#include "expected-hashes.h"
#include "interleave-checker.h"

// Based on tests/3-stack-test.c
void sys_equiv_putc(uint8_t ch);

// print everyting out
void equiv_puts(char *msg) {
    for(; *msg; msg++)
        sys_equiv_putc(*msg);
}

void hello(void *msg) {
    equiv_puts("hello from 1\n");
}
void msg(void *msg) {
    equiv_puts(msg);
}

void notmain(void) {
    interleave_opt_t opt = {
        .enable_stack = 1,
        .max_num_inst = 2000,
        .verbosity = 0,
        .interleave_verbosity = 1,
        .non_interleave_type = SEQUENTIAL,
        .random_seed = 12,
        .enable_vm = 1,
        .enable_all_caches = 1
    };

    void (*fcns[])(void *) = {hello, msg, msg};
    void *args[] = {0, "hello from 2\n", "hello from 3\n"};
    interleave_multiple(fcns, args, 3, opt);

    trace("stack passed\n ========== \n");
}
