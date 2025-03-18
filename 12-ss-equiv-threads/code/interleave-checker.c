// interleave-checker like lab 10 but using lab 12 code modified
#include "interleave-checker.h"

// basically: given the inputs from checker and num_copies_B
// 1. Runs A then B, checking the hash.
// 2. for (int i = 1; ; i++)
//      2.1. Runs A for i instructions, then switches to all num_copies_B which complete before returning to B
//      2.2. verify the hash
// 3. verify the final hash

int interleave_check(checker_config_t c) {
    uint32_t N = c.n_copies;
    assert(N > 0);
    eqx_init();

    // Run A then B in sequential mode (one after another)
    scheduler_config_t s;
    s.type = SEQUENTIAL;
    set_scheduler_config(s);
    eqx_verbose(c.verbosity);

    output("== Run A and B sequentially, using stack?=%d ==\n", c.enable_stack);
    let th_A = c.enable_stack ? eqx_fork(c.A, c.argA, 0) : eqx_fork_nostack(c.A, c.argA, 0);
    uint32_t h_A = eqx_run_threads();

    let th_B = c.enable_stack ? eqx_fork_nostack(c.B, c.argB, 0) : eqx_fork_nostack(c.B, c.argB, 0);
    uint32_t h_B = eqx_run_threads();

    // Now, run A for [i] instructions, then switches to num_copies_B of B
    output("== Run A 1 time, and B %d times ==\n", N);

    eqx_th_t *th[N + 1];

    // Fork n copies of B
    eqx_refork(th_B);
    th[0] = th_B;
    for(int i = 1; i < N; i++) {
        th[i] = c.enable_stack ? eqx_fork(c.B, c.argB, h_B) : eqx_fork_nostack(c.B, c.argB, h_B);
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
    output("success !!! ran A %d time and B %d times, A==B=%d\n\n", 1, c.n_copies, c.A == c.B);
    return 1;
}

// Taken from 12-ss-equiv-threads/code/tests/2-no-stack.test
// runs routines <N> times, comparing the result against <hash>.
// they are stateless, so each time we run them we should get
// the same result.  note: if you set hash to 0, it will run,
// and then write the result into the thread block.
static eqx_th_t * 
run_single(int N, void (*fn)(void*), void *arg, uint32_t hash, int enable_stack) {
    let th = enable_stack ? eqx_fork(fn, arg, hash) : eqx_fork_nostack(fn, arg, hash);
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

int interleave_x_with_others(int x, int sequential_hash, void (**funcArr)(void *), void **args, int N, int max_num_inst, int enable_stack) {
    assert(x < N);
    
    eqx_th_t *th[N];

    // The primary function, aka, the one where we will we run **i** instructions, switch, and then complete
    void (*X)(void *) = funcArr[x];
    void *X_args = args[x];

    // Fork the non-primary functions first
    for (int i = 0; i < N; ++i) {
        if (i != x) {
            th[i] = enable_stack ? eqx_fork(funcArr[i], args[i], 0) : eqx_fork_nostack(funcArr[i], args[i], 0);
        }
    }
    // Fork the primary function last (pushing it to the front of the runqueue)
    th[x] = enable_stack ? eqx_fork(funcArr[x], args[x], 0) : eqx_fork_nostack(funcArr[x], args[x], 0);
    eqx_run_threads();

    scheduler_config_t s;
    s.type = INTERLEAVE_X;
    s.interleave_tid = th[x]->tid;
    set_scheduler_config(s);

    // Now, run X for **i** instructions then the other threads
    for (int i = 1; ; i++) {
        // Refork all of the threads
        for (int j = 0; j < N; j++) {
            if (j != x) {
                eqx_refork(th[j]);
            }
        }
        eqx_refork(th[x]);

        s.switch_on_inst_n = i;
        set_scheduler_config(s);

        uint32_t res = eqx_run_threads();

        output("Switch X[idx=%d] on inst %d, hash: %x, expected %x \n", x, i, res, sequential_hash);
        assert(res == sequential_hash);

        if (thread_x_completed_before_yielding()) {
            output("X[idx=%d] complete with inst:%d\n", x, i);
            break;
        } else if (i >= max_num_inst) {
            output("X[idx=%d] reached max number of inst: %d\n", x, max_num_inst);
            break;
        }
    }
    output("success !!! ran X[idx=%d] interleaved\n\n", x);
    return 0;
}

int interleave_multiple(void (**funcArr)(void *), void **args, int n_fns, int max_num_inst, int verbosity, int enable_stack) {
    eqx_init();

    eqx_th_t *ths[n_fns];
    scheduler_config_t s;
    eqx_verbose(verbosity);

    // Ensure the hash (seems) determinstic
    for (int i = 0; i < n_fns; ++i) {
        ths[i] = run_single(3, funcArr[i], args[i], 0, enable_stack);
    }

    // Run each function in order
    for (int i = 0; i < n_fns; ++i) {
        eqx_refork(ths[i]);
    }
    s.type = SEQUENTIAL;
    set_scheduler_config(s);
    uint32_t hash = eqx_run_threads();
    output("Sequential hash: %x\n", hash);

    // We will interleave A with B, C, D
    // Then B with A, C, D
    // Then C with A, B, D, etc.    

    for (int i = 0; i < n_fns; ++i) {
        interleave_x_with_others(i, hash, funcArr, args, n_fns, max_num_inst, enable_stack);
    }
    return 1;
}