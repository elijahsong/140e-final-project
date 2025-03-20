#include "breakpoint.h"
#include "rpi.h"
#include "armv6-debug-impl.h"
#include "full-except.h"

static int single_step_on_p;

void brkpt_mismatch_stop(void) {
    if(!single_step_on_p)
        panic("mismatch not turned on\n");
    single_step_on_p = 0;

    // RMW bcr0 to disable breakpoint, 
    // make sure you do a prefetch_flush!
    uint32_t bcr0_v = cp14_bcr0_get();
    bcr0_v = bits_set(bcr0_v, 21, 22, 0b00);
    cp14_bcr0_set(bcr0_v);
}

void brkpt_mismatch_start(void) {
    if(single_step_on_p)
    panic("mismatch_on: already turned on\n");
    single_step_on_p = 1;

    // is ok if we call more than once.
    cp14_enable();

    // we assume they won't jump to 0.
    brkpt_mismatch_set(0);
    
}
void brkpt_mismatch_set(unsigned addr) {
    assert(single_step_on_p);
    uint32_t old_pc = cp14_bvr0_get();

    // set a mismatch (vs match) using bvr0 and bcr0 on
    // <pc>
    uint32_t b = 0b111100111; // 13-18 and 13-19
    b = bits_set(b, 21, 22, 0b10); // Set mismatch [22:21], 13-18
    cp14_bcr0_set(b);
    cp14_bvr0_set(addr);

    assert( cp14_bvr0_get() == addr );
    // return old_pc;
}

//   1. was a debug instruction fault (use ifsr)
//   2. breakpoint occured (13-11: use dscr)
int brkpt_fault_p(void) {
    return was_brkpt_fault();
}
