@ Must carefully read B2 of the armv6 manual.  You cannot rely on "well it
@ worked on my test case": mistakes cause stale data, which may or may not
@ show up in your test case (likely not) despite being wildly broken.
@
@ Note: Rd is allegedly a read-only parameter to these instructions, but
@ the manual says SBZ (should be zero).  I think (and the linux source seems
@ to think) that this means we have to explicitly set it to 0.  Not setting
@ Rd=0 is an easy mistake to make.  Perhaps is worth building in (Linux does
@ not, but) --- the cost of an extra mov is negligible given our other 
@ overheads.
@
@ Alot of people put these instructions in inline assembly, wrapped up by 
@ function calls (e.g., cs107e's MMU code).  This practice is dangerous. 
@ For example, if you are flushing caches, you cannot be sure that the 
@ function return, etc does not then reload them, esp w.r.t. the BTB.  I 
@ think the only safe approach is to put directly in assembly so you are 
@ guaranteed no branch, indirect jump, load/store happens in between.
@
@ A lot of MMU code out there appears pretty broken b/c they don't do enough
@ flushing operations and/or rely on gcc code generation to not do the above.

#include "rpi-asm.h"
#include "armv6-coprocessor-asm.h"


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@ many page numbers are in 
@    armv6-coprocessor-asm.h
@ and the other armv6-*.h files

// just read the cache type: use C code to figure out if it is unified
// b6-13, b6-14
MK_FN(get_cache_type)
    mov r0, #0
    CACHE_TYPE_RD(r0) @ provided (B6-13), seems like there's no need to PrefetchFlush
    bx lr

MK_FN(get_domain_ctrl_wr) @ Arm1776 3-63, no flush needed since it's just a read
    mov r0, #0
    DOMAIN_CTRL_RD(r0)
    bx lr

MK_FN(get_ctrl_reg_1) @ Arm1776, 3-45
    mov r0, #0
    MRC p15, 0, r0, c1, c0, 0 @ 3-47, no prefetch flush needed for a read
    bx lr

MK_FN(get_control_reg1_uint32_t) @ Arm1776, 3-45
    mov r0, #0
    CONTROL_REG1_RD(r0) @ 3-47, no prefetch flush needed for a read
    bx lr

// void ctrl_reg_1_wr(cp15_ctrl_reg1_t v);
@ r0 : v
MK_FN(cp15_ctrl_reg1_wr) @ Arm1776, 3-45
    MCR p15, 0, r0, c1, c0, 0 @ 3-47
    @ For a write (change to cp15), we need a prefetch flush
    mov r1, #0
    PREFETCH_FLUSH(r1)
    bx lr

// void reset(void)
// clear the dcache, icache, tlbs
MK_FN(mmu_reset)
    @ b staff_mmu_reset
    @ NOTE: MMU is OFF

    @ Load 0 into Rd (r0) as per the comment in the header
    mov r0, #0

    @ Invalidate all caches (ITLB, DTLB, data cache, instruction cache). Do not clean the data cache since it will potentially write garbage back to memory)
    @ 1. DMB: all prior cache maintenance ops are visible to **explicit** LOAD/STORE after the DMB
    @ 2. DSB: forces prior cache maintenance ops to complete
    @       --> TLB maintenance can only be guaranteed in this way
    @           --> ** must be followed by a PrefetchFlush
    @ 3. The following must also be followed by a PrefectchFlush
    @   a. BTB / branch predictor maintenance ops
    @   b. Changes to CP14 and CP15

    @ Invalidate data cache -- DO NOT CLEAN
    INV_DCACHE(r0)

    @ Invalidate instruction cache
    INV_ICACHE(r0)

    @ Invalidate unified or I/DTLB last
    INV_TLB(r0)
    DSB(r0)
    
    @ Must: DSB (to force TLB maintenance to complete), then prefetch flush according to B2-22, B2.7.3
    @ Must: flush_BTB (because of INV_TLB) ~ this seems not to be true
    FLUSH_BTB(r0) @ Need to flush_BTB (B2.7.5, B2-24)
    DSB(r0)

    PREFETCH_FLUSH(r0) @ B2-22, B2.7.3: PrefetchFlush enables the completed ops to be visible to all subsequent instructions
    @ need to invalidate BTB -> Prefetch Flush again
    @       when you write new data to instruction memory
    @ Note: This should take care of a unified TLB (data/instruction)
    @ These instructions are sequentially consistent w.r.t. each other

    bx lr

// void cp15_domain_ctrl_wr(uint32_t dom_reg);
//  need to do cp15 sync: flush_btb, dsb, prefetch flush
MK_FN(cp15_domain_ctrl_wr)
    DOMAIN_CTRL_WR(r0)
    mov r0, #0
    PREFETCH_FLUSH(r0) @ since we CP15 this
    bx lr

// void cp15_set_procid_ttbr0(uint32_t proc_and_asid, fld_t *pt);
// sequence from b2-25
// B4-41: Hardware has to find page table when TLB miss.
//  [ ] Write address of page table to TTBR0
//  [ ] Set both TTBR1 and TTBC to 0
// B4-52: Set current address space identifier (between 1..63)
// Coherence requirements:
//  1. Page table register: B2-21 to b2-24
//
// Changing ASID Cookbook: B2-25
@ Change ASID to 0
@ PrefetchFlush
@ Change Translation Table Base Register
@ PrefetchFlush
@ Change ASID to new value
@ cp15_set_procid_ttbr0(uint32_t proc_and_asid, void *pt)
MK_FN(cp15_set_procid_ttbr0)
    @ b staff_cp15_set_procid_ttbr0

    mov r3, #0
    DSB(r3) @ Must clear the contents of all thread/PID registers before switching
    @ 3-129/3-130 ARM 1776

    @ Change ASID to 0
    ASID_SET(r3)
    @ PrefetchFlush
    PREFETCH_FLUSH(r3)

    @ Change TTBR0 (r1), TTBR1 (0), and TTBRC (0)
    TTBR0_SET(r1)
    TTBR1_SET(r3)
    TTBR_BASE_CTRL_WR(r3)
    @ B2.7.5 (B2-24): Must flush BTB since we changed TTBR0/TTBR1/TTBCR
    FLUSH_BTB(r3)
    @ PrefetchFlush required after BTB flush
    PREFETCH_FLUSH(r3)

    @ Now, set the ASID/Context Register to proc_and_asid
    ASID_SET(r0)
    FLUSH_BTB(r3) @ B2.7.5 (B2-24) required since we changed ProcID
    PREFETCH_FLUSH(r3) @ PrefetchFlush required after BTB flush

    bx lr

// void mmu_disable_set_asm(cp15_ctrl_reg1_t c);
MK_FN(mmu_disable_set_asm)
    @ b staff_mmu_disable_set_asm

    @ note: this piece of code is mapped into the same place in both
    @ MMU-enabled and MMU-disabled so BTB and Prefetch entries work identically
    @ to both MMU-enable and disable.   Without this requirement I believe is
    @ very hard to get right (perhaps impossible?) since we cannot flush
    @ the BTB and prefetch and switch MMU on/off atomically.  thus, would
    @ flush, but instructions could get fetched before we could disable,
    @ defeating the purpose.

    @ NOTE: [Writeup] "Before disabling the MMU you'll need to clean the data cache or you'll lose dirty entries."
    @ 0a. It seems like we need to clear the data cache before disabling it in ctrl_1_reg??
    mov r1, #0
    CLEAN_INV_DCACHE(r1)
    DSB(r1) @ Required for data cache maintenance ops

    @ 0b. Invalidate ICache - the test assumes you do
    @ --> requires a Prefetch Flush even with DSB
    
    @ Arm1776 6.4 (6-9)
    @ 1. Clear bit 2 to 0 in ctrl_1_reg to disable the Data Cache.
    @ ** You must disable the Data Cache in the world before or at
    @       the same time of disabling the MMU (3-46)
    @ 2. Clear bit 0 to 0 in the CP15 control register c1. (r0 already contains this bit being cleared)
    @ bic r0, r0, #0x4 @ ctrl_1_reg[2]:= 0 ~ seems to be causing issues
    CONTROL_REG1_WR(r0)
    PREFETCH_FLUSH(r1)

    @ Clean INV cache (writeup)
    INV_ICACHE(r1)
    FLUSH_BTB(r1) @ B2.7.5 (B2-24) requires flush BTB then PrefetchFlush for MMU
    DSB(r1) @ Required for data cache maintenance ops
    PREFETCH_FLUSH(r1)

    bx lr

// void mmu_enable_set_asm(cp15_ctrl_reg1_t c);
MK_FN(mmu_enable_set_asm)
    @ b staff_mmu_enable_set_asm

    @ Arm1776 6.4 (6-9)
    @ 1. Program all **relevant** cp15 registers
    @ --> set MMU enabled in CP15_ctrl_reg1 [DONE IN THE C code]
    @ --> r0 is the correct register to write

    @ 2. Program first-level and second-level descriptor page tables
    @ We skip this based on the lab writeup (see below)

    @ 3. Disable AND invalidate Instruction Cache
    @ 3a. Disable Instruction Cache
    @ Clear bit 12 (bit12 = 0) (ARM 1776 p. 178 / 3-46)
    mov r2, #0
    CONTROL_REG1_RD(r1)
    bic r1, r1, #4096 @ Clear bit 12
    CONTROL_REG1_WR(r1)
    PREFETCH_FLUSH(r2) @ Must prefetch_flush after write to CP15

    @ 3b. Invalidate ICache
    INV_ICACHE(r2)
    DSB(r2)
    @ PREFETCH_FLUSH(r2)

    @ 4. Enable the MMU by setting bit 0 in the CP15 control register in the corresponding world.
    @ 4b. B4.2.3 (B4-6) says that you can re-enable ICache simultaneously
    @ orr r0, r0, #0x1000 @ Set r0[12] = 1 (ARM 1776 p. 178 / 3-46) ~ seems to be causing issues
    CONTROL_REG1_WR(r0)
    PREFETCH_FLUSH(r2)
    FLUSH_BTB(r2) @ B2.7.5 (B2-24) requires flush BTB then PrefetchFlush for MMU disable
    PREFETCH_FLUSH(r2)

    bx lr

@ B 2.7.3 (B2-23)
@ STR rx, [Page table entry] ;
@ Clean line [Page table entry]
@ DSB ; ensures visibility of the data cleaned from the D Cache
@ Invalidate TLB entry by MVA [page address]
@ Invalidate BTB
@ DSB ; ensure completion of the Invalidate TLB
@ PrefetchFlush
MK_FN(mmu_sync_pte_mods)
    @ @ @ b staff_mmu_sync_pte_mods

    mov r1, #0
    CLEAN_INV_DCACHE(r1) @ clean the line (both data and instruction)
    INV_ICACHE(r1)
    DSB(r1)

    INV_TLB(r1) @ invalidate TLB entry

    FLUSH_BTB(r1)
    DSB(r1)
    PREFETCH_FLUSH(r1)

    bx lr

