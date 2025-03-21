#ifndef __EQX_H__
#define __EQX_H__
/*******************************************************************
 * simple process support.
 */

#include "switchto.h"   // needed for <regs_t>

typedef enum {
    SEQUENTIAL = 1,
    ROUND_ROBIN = 2,
    EVERY_X = 3,
    ALL_PATHS = 4,
    RANDOM = 5
} Scheduler;

typedef struct config {
  Scheduler type;
  // EVERY_X
  uint32_t switch_on_inst_n;
  uint32_t random_seed;
  int enable_interleave;
  uint32_t interleave_tid;
} scheduler_config_t;

typedef struct eqx_th {
    // thread's registers.
    regs_t regs;

    struct eqx_th *next;

    // if non-zero: the hash we expect to get when
    // the thread exits
    uint32_t expected_hash;

    // the current cumulative hash
    uint32_t reg_hash;          

    uint32_t tid;           // thread id.

    uint32_t fn;
    uint32_t arg;
    uint32_t stack_start;
    uint32_t stack_end;
    uint32_t refork_cnt;

    // how many instructions we executed.
    uint32_t inst_cnt;
    uint32_t cumulative_inst_cnt;
    unsigned verbose_p;  // if you want alot of information.
} eqx_th_t;

// a very heavy handed initialization just for today's lab.
// assumes it has total control of system calls etc.
void eqx_init(void);
void eqx_init_w_vm(int enable_all_caches);

int thread_x_completed_before_yielding(void);

// run all the threads until the runqueue is empty.
//  - returns xor-hash of all hashes.
uint32_t eqx_run_threads(void);

void set_scheduler_config(scheduler_config_t s);
void eqx_refork(eqx_th_t *th);

eqx_th_t * eqx_fork(void (*fn)(void*), void *arg, uint32_t hash);
eqx_th_t * eqx_fork_nostack(void (*fn)(void*), void *arg, uint32_t hash) ;
eqx_th_t * eqx_fork_stack(void (*fn)(void*), void *arg,
    uint32_t expected_hash, void *stack, uint32_t nbytes);

// 1 = chatty, 0 = quiet
void eqx_verbose(int verbose_p);

// default stack size.
enum { eqx_stack_size = 8192 * 8 };
_Static_assert(eqx_stack_size > 1024, "too small");
_Static_assert(eqx_stack_size % 8 == 0, "not aligned");


// called by client code.
void sys_equiv_exit(uint32_t ret);

#endif
