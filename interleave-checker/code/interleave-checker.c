// interleave-checker like lab 10 but using lab 12 code modified
#include "interleave-checker.h"

void reg_dump(uint32_t tid, uint32_t cnt, regs_t *r, interleave_opt_t opt, int x, uint32_t inst) {
    if (!opt.interleave_verbosity) return;

    uint32_t pc = r->regs[REGS_PC];
    uint32_t cpsr = r->regs[REGS_CPSR];
    output("tid=%d: pc=%x cpsr=%x: for X=%d, inst=%d", 
        tid, pc, cpsr, x, inst);
    if(!cnt) {
        output("  {first instruction}\n");
        return;
    }

    int changes = 0;
    output("\n");
    for(unsigned i = 0; i<15; i++) {
        if(r->regs[i]) {
            output("   r%d=%x, ", i, r->regs[i]);
            changes++;
        }
        if(changes && changes % 4 == 0)
            output("\n");
    }
    if(!changes)
        output("  {no changes}\n");
    else if(changes % 4 != 0)
        output("\n");
}

// basically: given the inputs from checker and num_copies_B
// 1. Runs A then B, checking the hash.
// 2. for (int i = 1; ; i++)
//      2.1. Runs A for i instructions, then switches to all num_copies_B which complete before returning to B
//      2.2. verify the hash
// 3. verify the final hash

int simple_interleave_check(checker_config_t c) {
    uint32_t N = c.n_copies;
    assert(N > 0);

    if (c.enable_vm) {
        eqx_init_w_vm(c.enable_all_caches);
    } else {
        eqx_init();
    }    

    // Run A then B in sequential mode (one after another)
    scheduler_config_t s;
    s.type = SEQUENTIAL;
    s.enable_interleave = 0;
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
    s.type = SEQUENTIAL;
    s.enable_interleave = 1;
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
    th->verbose_p = 0;
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

int interleave_x_with_others(int x, int sequential_hash, void (**funcArr)(void *), void **args, int N, interleave_opt_t opt, eqx_th_t **th) {
    assert(x < N);
    int n_errors = 0;
    // eqx_th_t *th[N];

    // The primary function, aka, the one where we will we run **i** instructions, switch, and then complete
    void (*X)(void *) = funcArr[x];
    void *X_args = args[x];

    // NOTE : Don't fork again (will cause $sp to differ due to stack and checksum to fail)
    // Fork the non-primary functions first
    // for (int i = 0; i < N; ++i) {
    //     if (i != x) {
    //         th[i] = opt.enable_stack ? eqx_fork(funcArr[i], args[i], 0) : eqx_fork_nostack(funcArr[i], args[i], 0);
    //     }
    // }
    // // Fork the primary function last (pushing it to the front of the runqueue)
    // th[x] = opt.enable_stack ? eqx_fork(funcArr[x], args[x], 0) : eqx_fork_nostack(funcArr[x], args[x], 0);
    // eqx_run_threads();

    scheduler_config_t s;
    s.type = opt.non_interleave_type; // scheduling strategy used after the primary function X completes i instructions
    s.random_seed = opt.random_seed;
    s.enable_interleave = 1;
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

        if (opt.interleave_verbosity)
            output("Switch X[idx=%d] on inst %d, hash: %x, expected: %x \n", x, i, res, sequential_hash);
        
        if (res != sequential_hash) {
            if (opt.interleave_verbosity) {
                output("hash=%x does not match expected=%x, on inst %d.\n", res, sequential_hash, i);
            }
            ++n_errors;
        }

        if (thread_x_completed_before_yielding()) {
            if (opt.enable_fnames)
                output("f_%d (%s) ran fully to inst: %d\n", x, opt.fnames[x], th[x]->cumulative_inst_cnt);
            else 
                output("f_%d ran fully to inst: %d\n", x, th[x]->cumulative_inst_cnt);

            if (opt.interleave_verbosity && res != sequential_hash) {
                reg_dump(th[x]->tid, th[x]->inst_cnt, &th[x]->regs, opt, x, th[x]->cumulative_inst_cnt);
            }
            break;
        } else if (i >= opt.max_num_inst) {
            if (opt.enable_fnames)
                output("f_%d (%s) reached max number of inst: %d\n", x, opt.fnames[x], opt.max_num_inst);
            else 
                output("f_%d reached max number of inst: %d\n", x, opt.max_num_inst);
            break;
        }
    }
    return n_errors;
}

int interleave_multiple(void (**funcArr)(void *), void **args, int n_fns, interleave_opt_t opt) {
    if (opt.enable_vm) {
        eqx_init_w_vm(opt.enable_all_caches);
    } else {
        eqx_init();
    }    

    eqx_th_t *ths[n_fns];
    scheduler_config_t s;
    eqx_verbose(opt.verbosity);

    // Ensure the hash (seems) determinstic
    for (int i = 0; i < n_fns; ++i) {
        ths[i] = run_single(3, funcArr[i], args[i], 0, opt.enable_stack);
    }

    // Run each function in order
    for (int i = 0; i < n_fns; ++i) {
        eqx_refork(ths[i]);
    }
    s.type = SEQUENTIAL;
    s.enable_interleave = 0;
    set_scheduler_config(s);
    uint32_t hash = eqx_run_threads();

    // if (opt.interleave_verbosity) {
    //     for (int i = 0; i < n_fns; ++i) {
    //         reg_dump(ths[0]->tid, ths[i]->inst_cnt, &ths[i]->regs, opt, i, ths[i]->cumulative_inst_cnt);
    //     }
    // }

    // We will interleave A with B, C, D
    // Then B with A, C, D
    // Then C with A, B, D, etc.    

    for (int i = 0; i < n_fns; ++i) {
        int n_errors = interleave_x_with_others(i, hash, funcArr, args, n_fns, opt, ths);
        output("Ran X[idx=%d] interleaved, found %d errors.\n\n", i, n_errors);
    }
    output("Sequential hash: %x\n\n", hash);

    return 1;
}