// interleave-checker like lab 10 but using lab 12 code modified
#include "interleave-checker.h"

// basically: given the inputs from checker and num_copies_B
// 1. Runs A then B, checking the hash.
// 2. for (int i = 1; ; i++)
//      2.1. Runs A for i instructions, then switches to all num_copies_B which complete before returning to B
//      2.2. verify the hash
// 3. verify the final hash

int interleave_check_stack(void (*A)(void*), void (*B)(void*), void *argA, void *argB, int N, int max_num_inst, int verbosity) {
    scheduler_config_t s;
    s.type = SEQUENTIAL;
    set_scheduler_config(s);
    eqx_verbose(verbosity);

    let th_A = eqx_fork(A, argA, 0);
    uint32_t h_A = eqx_run_threads();

    let th_B = eqx_fork(B, argB, 0);
    uint32_t h_B = eqx_run_threads();

    // Now, run A for i instructions, then switches to all num_copies_B
    output("== Run A 1 time, and B %d times ==\n", N);

    eqx_th_t *th[N + 1];

    // Fork n copies of B
    for(int i = 0; i < N; i++) {
        th[i] = eqx_fork(B, 0, h_B);
       // output("thread %d is B, tid: %d, th->verbose: %d\n", i, th[i]->tid, th[i]->verbose_p);
    }

    // Fork A (this gets pushed to the **front** of the queue)
    th[N] = eqx_fork(A, 0, h_A);
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

        output("Switch A on inst:%d, hash: %x, expected: %x :D\n", i, res, expected_hash);
        assert(res == expected_hash);
    }
    return 1;
}


int interleave_check_nostack(void (*A)(void*), void (*B)(void*), void *argA, void *argB, int N, int max_num_inst, int verbosity) {
    scheduler_config_t s;
    s.type = SEQUENTIAL;
    set_scheduler_config(s);
    eqx_verbose(verbosity);

    let th_A = eqx_fork_nostack(A, argA, 0);
    uint32_t h_A = eqx_run_threads();

    let th_B = eqx_fork_nostack(B, argB, 0);
    uint32_t h_B = eqx_run_threads();

    // Now, run A for i instructions, then switches to all num_copies_B
    output("== Run A 1 time, and B %d times ==\n", N);

    eqx_th_t *th[N + 1];

    // Fork n copies of B
    eqx_refork(th_B);
    th[0] = th_B;
    for(int i = 1; i < N; i++) {
        th[i] = eqx_fork_nostack(B, argB, h_B);
       // output("thread %d is B, tid: %d, th->verbose: %d\n", i, th[i]->tid, th[i]->verbose_p);
    }

    // Fork A (this gets pushed to the **front** of the queue)
    eqx_refork(th_A);
    th[N] = th_A;
    // output("thread %d is A, tid: %d, h_A: %x\n\n", N, th[N]->tid, h_A);

    uint32_t expected_hash = h_A + N * h_B;
    s.type = INTERLEAVE_X;
    s.interleave_tid = th[N]->tid;

    // Now, run A for **i** instructions then n copies of B, then finish A.
    for(int i=1; ; i++) {
        if (i != 1) {
            for (int i = 0; i < N + 1; i++) {
                eqx_refork(th[i]);
            }
        }

        s.switch_on_inst_n = i;
        set_scheduler_config(s);

        uint32_t res = eqx_run_threads();

        output("Switch A on inst:%d, hash: %x, expected: %x :D\n", i, res, expected_hash);
        assert(res == expected_hash);

        if (thread_x_completed_before_yielding()) {
            output("A complete with inst:%d\n\n", i);
            break;
        }
    }
    return 1;
}

int interleave_check(checker_config_t c) {
    // Run A then B in sequential mode (one after another)
    eqx_init();
    uint32_t N = c.n_copies;

    // if (!c.enable_stack) {
    //     return interleave_check_nostack(c.A, c.B, c.argA, c.argB, c.n_copies, c.max_num_inst, c.verbosity);
    // } else {
    //     return interleave_check_stack(c.A, c.B, c.argA, c.argB, c.n_copies, c.max_num_inst, c.verbosity);
    // }

    scheduler_config_t s;
    s.type = SEQUENTIAL;
    set_scheduler_config(s);
    eqx_verbose(c.verbosity);

    output("== Run A and B sequentially, using stack?=%d ==\n", c.enable_stack);
    let th_A = c.enable_stack ? eqx_fork(c.A, c.argA, 0) : eqx_fork_nostack(c.A, c.argA, 0);
    uint32_t h_A = eqx_run_threads();

    let th_B = c.enable_stack ? eqx_fork_nostack(c.B, c.argB, 0) : eqx_fork_nostack(c.B, c.argB, 0);
    uint32_t h_B = eqx_run_threads();

    // Now, run A for i instructions, then switches to all num_copies_B
    output("== Run A 1 time, and B %d times ==\n", N);

    eqx_th_t *th[N + 1];

    // Fork n copies of B
    eqx_refork(th_B);
    th[0] = th_B;
    for(int i = 1; i < N; i++) {
        th[i] = eqx_fork_nostack(c.B, c.argB, h_B);
       // output("thread %d is B, tid: %d, th->verbose: %d\n", i, th[i]->tid, th[i]->verbose_p);
    }

    // Fork A (this gets pushed to the **front** of the queue)
    eqx_refork(th_A);
    th[N] = th_A;
    // output("thread %d is A, tid: %d, h_A: %x\n\n", N, th[N]->tid, h_A);

    uint32_t expected_hash = h_A + N * h_B;
    s.type = INTERLEAVE_X;
    s.interleave_tid = th[N]->tid;

    // Now, run A for **i** instructions then n copies of B, then finish A.
    for(int i=1; ; i++) {
        if (i != 1) {
            for (int i = 0; i < N + 1; i++) {
                eqx_refork(th[i]);
            }
        }

        s.switch_on_inst_n = i;
        set_scheduler_config(s);

        uint32_t res = eqx_run_threads();

        output("Switch A on inst:%d, hash: %x, expected: %x :D\n", i, res, expected_hash);
        assert(res == expected_hash);

        if (thread_x_completed_before_yielding()) {
            output("A complete with inst:%d\n\n", i);
            break;
        }
    }
    return 1;
}

// Taken from 12-ss-equiv-threads/code/tests/2-no-stack.test
// runs routines <N> times, comparing the result against <hash>.
// they are stateless, so each time we run them we should get
// the same result.  note: if you set hash to 0, it will run,
// and then write the result into the thread block.
static eqx_th_t * 
run_single_nostack(int N, void (*fn)(void*), void *arg, uint32_t hash) {
    let th = eqx_fork_nostack(fn, arg, hash);
    th->verbose_p = 1;
    eqx_run_threads();

    if(hash && th->reg_hash != hash)
        panic("impossible: eqx did not catch mismatch hash\n");
    hash = th->reg_hash;
    trace("--------------done first run!-----------------\n");

    // turn off extra output
    th->verbose_p = 0;
    for(int i = 0; i < N; i++) {
        eqx_refork(th);
        eqx_run_threads();
        if(th->reg_hash != hash)
            panic("impossible: eqx did not catch mismatch hash\n");
        trace("--------------done run=%d!-----------------\n", i);
    }

    return th;
}

int interleave_multiple_nostack(void (**funcArr)(void *), void **args, int n_fns, int max_num_inst, int verbosity) {
    eqx_th_t *th[n_fns];
    scheduler_config_t s;

    for (int i = 0; i < n_fns; ++i) {
        th[i] = run_single_nostack(3, funcArr[i], args[i], 0);
    }

    // Run each function in order
    for (int i = 0; i < n_fns; ++i) {
        eqx_refork(th[i]);
    }
    s.type = SEQUENTIAL;
    set_scheduler_config(s);
    uint32_t hash = eqx_run_threads();
    output("Sequential hash: %d\n", hash);
    
    return 1;
}

int interleave_multiple(void (**funcArr)(void *), void **args, int n_fns, int enable_stack, int max_num_inst, int verbosity) {
    // Initialize interleaving
    eqx_init();

    // Ensure that each function (seems) deterministic. If not, we return.
    if (!enable_stack) {
        return interleave_multiple_nostack(funcArr, args, n_fns, max_num_inst, verbosity);
    } else {
        output("UNIMPLEMENTED!\n");
    }
    return 1;
}