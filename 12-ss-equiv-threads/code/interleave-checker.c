// interleave-checker like lab 10 but using lab 12 code modified
#include "interleave-checker.h"

// basically: given the inputs from checker and num_copies_B
// 1. Runs A then B, checking the hash.
// 2. for (int i = 1; ; i++)
//      2.1. Runs A for i instructions, then switches to all num_copies_B which complete before returning to B
//      2.2. verify the hash
// 3. verify the final hash


int interleave_check_nostack(void (*A)(void*), void (*B)(void*), int N, int max_num_inst, int verbosity) {
    scheduler_config_t s;
    s.type = SEQUENTIAL;
    set_scheduler_config(s);
    eqx_verbose(verbosity);

    let th_A = eqx_fork_nostack(A, 0, 0);
    uint32_t h_A = eqx_run_threads();

    let th_B = eqx_fork_nostack(B, 0, 0);
    uint32_t h_B = eqx_run_threads();

    // Now, run A for i instructions, then switches to all num_copies_B
    output("== Run A 1 time, and B %d times ==\n", N);

    eqx_th_t *th[N + 1];

    // Fork n copies of B
    for(int i = 0; i < N; i++) {
        th[i] = eqx_fork_nostack(B, 0, h_B);
       // output("thread %d is B, tid: %d, th->verbose: %d\n", i, th[i]->tid, th[i]->verbose_p);
    }

    // Fork A (this gets pushed to the **front** of the queue)
    th[N] = eqx_fork_nostack(A, 0, h_A);
    // output("thread %d is A, tid: %d, h_A: %x\n\n", N, th[N]->tid, h_A);

    uint32_t expected_hash = h_A + N * h_B;
    s.type = INTERLEAVE_X;
    s.interleave_tid = th[N]->tid;

    // Now, run A for **i** instructions then n copies of B, then finish A.
    for(int i=1; i < max_num_inst; i++) {
        if (i != 1) {
            for (int i = 0; i < N + 1; i++) {
                eqx_refork(th[i]);
            }
        }

        s.switch_on_inst_n = i;
        set_scheduler_config(s);

        uint32_t res = eqx_run_threads();

        output("Switch A on inst:%d,\thash: %x, expected: %x\n", i, res, expected_hash);
        assert(res == expected_hash);
    }
    return 1;
}

int interleave_check(checker_config_t c) {
    // Run A then B in sequential mode (one after another)
    eqx_init();

    if (c.stack_nbytes == 0) {
        return interleave_check_nostack(c.A, c.B, c.n_copies, c.max_num_inst, c.verbosity);
    } else {
        output("=== NOT IMPLEMENTED ===\n");
        return 0;
    }
    // void *buf_A = kmalloc(nbytes);
    // void *buf_B = kmalloc(nbytes);
    // scheduler_config_t s;
    // s.type = SEQUENTIAL;

    // set_scheduler_config(s);

    // let th_A = eqx_fork(A, buf_A, 0);
    // h_A = eqx_run_threads();

    // let th_B = eqx_fork(B, buf_B, 0);
    // h_B = eqx_run_threads();

    // // Now, run A for i instructions, then switches to all num_copies_B
    // output("about to run A 1 time, and B %d times\n", N);

    // eqx_th_t *th[N + 1];

    // // Fork n copies of B
    // for(int i = 0; i < N; i++) {
    //     void *buf = kmalloc(nbytes);
    //     th[i] = eqx_fork(B, buf, 0);
    // }

    // // Fork A (this gets pushed to the **front** of the queue)
    // buf_A = kmalloc(nbytes);
    // th[N] = eqx_fork(A, buf_A, 0);

    // uint32_t expected_hash = h_A + N * h_B;
    // s.type = INTERLEAVE_X;
    // s.interleave_tid = th[N]->tid;

    // // Now, run A for **i** instructions then n copies of B, then finish A.
    // for(int i=1; i < max_num_inst; i++) {
    //     if (i != 1) {
    //         for (int i = 0; i < N + 1; i++) {
    //             eqx_refork(th[i]);
    //         }
    //     }

    //     s.switch_on_inst_n = i;
    //     set_scheduler_config(s);

    //     output("about to run A 1 time, and B %d times. Switch A on %d \n", N, i);

    //     uint32_t res = eqx_run_threads();

    //     output("\t hash: %d, expected: \n", res, expected_hash);
    //     assert(res == expected_hash);
    // }
    // return 1;
}