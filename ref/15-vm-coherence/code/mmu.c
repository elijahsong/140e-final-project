// hardware mmu code: mostly they do error checking and call into
// assembly (your-mmu-asm.S).
#include "rpi.h"
#include "rpi-constants.h"
#include "rpi-interrupts.h"
#include "libc/helper-macros.h"
#include "mmu-internal.h"

// given.
uint32_t get_cache_type();
void mmu_reset(void);
void ctrl_reg_1_wr(cp15_ctrl_reg1_t v);

void mmu_enable_set_asm(cp15_ctrl_reg1_t c);
void mmu_disable_set_asm(cp15_ctrl_reg1_t c);

uint32_t get_domain_ctrl_wr();
cp15_ctrl_reg1_t get_ctrl_reg_1();

cp15_ctrl_reg1_t cp15_ctrl_reg1_rd(void) {
    return get_ctrl_reg_1();
}

int mmu_is_enabled(void) {
    return cp15_ctrl_reg1_rd().MMU_enabled != 0;
}

// disable the mmu by setting control register 1
// to <c:32>.
// 
// we use a C veneer over the assembly (mmu_disable_set_asm)
// so we can easily do assertions: the real work is 
// done by the asm code (you'll write this next time).
void mmu_disable_set(cp15_ctrl_reg1_t c) {
    assert(!c.MMU_enabled);
    
    // record if dcache on.
    uint32_t cache_on_p = c.C_unified_enable;

    mmu_disable_set_asm(c);

    // re-enable if it was on.
    if(cache_on_p) {
        c.C_unified_enable = 1;
        cp15_ctrl_reg1_wr(c);
    }
}

// disable the MMU by flipping the enable bit.   we 
// use a C vener so we can do assertions and then call
// out to assembly to do the real work (you'll write this
// next time).
void mmu_disable(void) {
    cp15_ctrl_reg1_t c = cp15_ctrl_reg1_rd();
    assert(c.MMU_enabled);
    c.MMU_enabled=0;
    mmu_disable_set(c);
}

// enable the mmu by setting control reg 1 to
// <c>.   we start in C so we can do assertions
// and then call out to the assembly for the 
// real work (you'll write this code next time).
void mmu_enable_set(cp15_ctrl_reg1_t c) {
    assert(c.MMU_enabled);
    // staff_mmu_enable_set_asm(c);
    mmu_enable_set_asm(c);
}

// enable mmu by flipping enable bit.
void mmu_enable(void) {
    cp15_ctrl_reg1_t c = cp15_ctrl_reg1_rd();
    assert(!c.MMU_enabled);
    c.MMU_enabled = 1;
    mmu_enable_set(c);
}

// probably should merge with <set_procid_ttbr0>
void mmu_set_ctx(uint32_t pid, uint32_t asid, void *pt) {
    assert(asid!=0);
    assert(asid<64);
    cp15_set_procid_ttbr0(pid << 8 | asid, pt); // B4.9.10: [31:8] is PID, [7:0] is ASID
    // staff_cp15_set_procid_ttbr0(pid << 8 | asid, pt);
}

// set so that we use armv6 memory.
// this should be wrapped up neater.  broken down so can replace 
// one by one.
//  1. the fields are defined in vm.h.
//  2. specify armv6 (no subpages).
//  3. check that the coprocessor write succeeded.
void mmu_init(void) { 
    // staff_mmu_init(); // return;
    // initialize the MMU hardware state
    mmu_reset(); // TODO: both labs??

    // trivial: RMW the xp bit in control reg 1.
    // leave mmu disabled.
    cp15_ctrl_reg1_t c = cp15_ctrl_reg1_rd();
    c.XP_pt = 1; // Arm1776 3-45: XP bit set: 1 = vmsav6, subpages disabled.
    //    must be enabled for XN. (armv6-cp15.h)
    cp15_ctrl_reg1_wr(c);

    // make sure write succeeded.
    struct control_reg1 c1 = cp15_ctrl_reg1_rd();
    assert(c1.XP_pt);
    assert(!c1.MMU_enabled);
}

// read and return the domain access control register
uint32_t domain_access_ctrl_get(void) {
    // return staff_domain_access_ctrl_get();
    return get_domain_ctrl_wr();
}

// please move your <domain_access_ctrl_set> routines
// from pinned-vm.c to here.

#if 0
// b4-42
// set domain access control register to <r>
void domain_access_ctrl_set(uint32_t r) {
    // staff_domain_access_ctrl_set(r);
    // Set cp15_domain (B4-42)
    cp15_domain_set(d); // automatically does a prefetch flush
    assert(domain_access_ctrl_get() == d);

    assert(domain_access_ctrl_get() == r);
}
#endif
