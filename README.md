# 140e-final-project
This project created a robust and versatile interleave checker that enables us to
verify the correctness of multi-threaded programs. The interleave checker uses
an additive hash of the registers of a program at each instruction to
verify correctness and builds upon 12-single-step `eqx-threads.c`.

Our interleave checker also provides the ability to run multiple copies of a
piece of code at the same time. For example, users can run a function across
many threads (e.g. 20 or more) and verify that the program functions correctly.
We call this our simple interleave checker, and it works as follows:

Given the inputs from `checker` and `num_copies_B`:
   1. Runs A then B for `num_copies_B` times, checking the hash.
   2. for (int i = 1; ; i++)
        2.1. Runs A for i instructions, then switches to all num_copies_B which complete before returning to B  
        2.2. Verify the hash

We then decided to expand upon our functionality, by supporting interleaving multiple functions 
at once in `interleave_multiple`. Say we have multiple functions A, B, C, D:
- We first interleave A w.r.t. B, C, and D 
    (namely, we run A for `i` instructions then B, C, D, then the rest of A)
    The strategy by which B, C, D, and the rest of A is run is customizable (see more below)
- Then, we interleave B w.r.t. A, C, D
- etc.

This is a fairly robust way to check different orderings of programs. Furthermore, with regard to
the way the rest of the program runs (after the function runs for `i` instructions), users have
several options, such as:
1. Sequential - run threads in order
2. Round-robin - run one instruction from each thread at a time (the original implementation)
3. Every-x - run x instructions from each thread at a time
4. Random - switch threads based on a `random` function.

Thus, users have a multitude of ways to verify the correctness of their programs.

Furthermore, we built upon the work in labs 15 (VM coherence) and lab 17 (Page Table VM) to
verify the correctness of programs that:
1. Do not use VM
2. Use VM but not cache
3. Use VM and cache.

For user's convenience, we also provide options to enable various levels of verbosity.

We hope you enjoyed our project, and have a great Spring Break!!!