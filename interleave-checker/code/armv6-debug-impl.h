#ifndef __ARMV6_DEBUG_IMPL_H__
#define __ARMV6_DEBUG_IMPL_H__
// header file w/ all the cp14 (debug) assembly routines.

// bit_* and bits_* routines.
#include "libc/bit-support.h"           

// <prefetch_flush()> is defined here as well as useful macros.
#include "asm-helpers.h"

// the macros below:
//   - <coproc_mk>
//   - <coproc_mk_set>
//   - <coproc_mk_get>
// are useful for generating get and set routines. 
//
// Example use: 
//    coproc_mk(bvr0, p14, 0, c0, c0, 4)
// produces the complete routines for 
//   cp14_brv0_get
//   cp14_brv0_set
// 
// ******NOTE******
// ******NOTE******
// ******NOTE******
// ******NOTE******
//  - we've provided definitions for most instructions
//    so that the code will compile and exit with an "unimplemented"
//    error
//  - thus: each time you define a routine if you get a "redefinition
//    error" delete the prototype below. 

// turn <x> into a string
#define MK_STR(x) #x

// define a general co-processor inline assembly routine to set the value.
// from manual: must prefetch-flush after each set.
#define coproc_mk_set(fn_name, coproc, opcode_1, Crn, Crm, opcode_2)       \
    static inline void c ## coproc ## _ ## fn_name ## _set(uint32_t v) {                    \
        asm volatile ("mcr " MK_STR(coproc) ", "                        \
                             MK_STR(opcode_1) ", "                      \
                             "%0, "                                     \
                            MK_STR(Crn) ", "                            \
                            MK_STR(Crm) ", "                            \
                            MK_STR(opcode_2) :: "r" (v));               \
        prefetch_flush();                                               \
    }

#define coproc_mk_get(fn_name, coproc, opcode_1, Crn, Crm, opcode_2)       \
    static inline uint32_t c ## coproc ## _ ## fn_name ## _get(void) {                      \
        uint32_t ret=0;                                                   \
        asm volatile ("mrc " MK_STR(coproc) ", "                        \
                             MK_STR(opcode_1) ", "                      \
                             "%0, "                                     \
                            MK_STR(Crn) ", "                            \
                            MK_STR(Crm) ", "                            \
                            MK_STR(opcode_2) : "=r" (ret));             \
        return ret;                                                     \
    }


// make both get and set methods.
#define coproc_mk(fn, coproc, opcode_1, Crn, Crm, opcode_2)     \
    coproc_mk_set(fn, coproc, opcode_1, Crn, Crm, opcode_2)        \
    coproc_mk_get(fn, coproc, opcode_1, Crn, Crm, opcode_2) 


//**********************************************************************
// all your code to implement the cp14 debug helpers should
// go below.

// 13-5: example of defining a struct to specify debug
// register layout.
struct debug_id {
    // uses bitfields.
    // <lower bit pos> ':' <upper bit pos> [inclusive]
    // see 0-example-debug.c for how to use macros
    // to check bitposition and size.  very very easy
    // to mess up: you should always do.
    uint32_t    revision:4,     // 0:3  revision number
                variant:4,      // 4:7  major revision number
                :4,             // 8:11
                debug_rev:4,   // 12:15
                debug_ver:4,    // 16:19
                context:4,      // 20:23
                brp:4,          // 24:27 --- number of breakpoint register
                                //           pairs+1
                wrp:4          // 28:31 --- number of watchpoint pairs.
        ;
};

// two different ways to implement <cp14_debug_id_get>
// one writing by hand, the other using the macro above.
#if 0
// write the routine to get the debug id register by hand.
static inline uint32_t cp14_debug_id_get(void) {
    // the documents seem to imply the general purpose register 
    // SBZ ("should be zero") so we clear it first.
    uint32_t ret = 0;

    asm volatile ("mrc p14, 0, %0, c0, c0, 0" : "=r"(ret));
    return ret;
}
#else
// This macro invocation creates a routine called cp14_debug_id_get
// identical to the above.
//
// you can see this by adding "-E" to the gcc compile line and inspecting
// the output.
coproc_mk_get(debug_id, p14, 0, c0, c0, 0)

#endif

// example of how to define get and set for the cp14
// status registers.  will generate:
//
//  static inline uint32_t cp14_status_get(void) { ... }
//  static inline void cp14_status_set(uint32_t status) {...}

// REFERENCE

// coproc_mk_get(fn_name, coproc, opcode_1, Crn, Crm, opcode_2) 
// coproc_mk_set(fn_name, coproc, opcode_1, Crn, Crm, opcode_2)  
// coproc_mk(fn, coproc, opcode_1, Crn, Crm, opcode_2) 

// Creates c{p14}_{status}_{get, set}

coproc_mk(status, p14, 0, c0, c1, 0) // p. 13-9

coproc_mk(dfsr, p15, 0, c5, c0, 0) // p. 3-67
coproc_mk(ifar, p15, 0, c6, c0, 2) // p. 3-69
coproc_mk(ifsr, p15, 0, c5, c0, 1) // p. 3-68

coproc_mk(dscr, p14, 0, c0, c1, 0) // p. 13-5

coproc_mk(wcr0, p14, 0, c0, c0, 7) // p. 13-21 to 13-22 0b111 TODO: c112
coproc_mk(wvr0, p14, 0, c0, c0, 6) // p. 13-20 TODO: c96
coproc_mk(wfar, p14, 0, c0, c6, 0) // p. 13-12

coproc_mk(bcr0, p14, 0, c0, c0, 5) // p. 13-17 0b101 TODO: is this c80 % 64
coproc_mk(bvr0, p14, 0, c0, c0, 4) // p. 13-16 0b100


// you'll need to define these and a bunch of other routines.
// static inline uint32_t cp15_dfsr_get(void) { todo("implement"); }
// static inline uint32_t cp15_ifar_get(void) { todo("implement"); }
// static inline uint32_t cp15_ifsr_get(void) { todo("implement"); }
// static inline uint32_t cp14_dscr_get(void) { todo("implement"); }

// static inline uint32_t cp14_wcr0_set(uint32_t r) { todo("implement"); }
// static inline void cp14_wvr0_set(uint32_t r) { todo("implement"); }

// static inline void cp14_bcr0_set(uint32_t r) { todo("implement"); }
// static inline void cp14_bvr0_set(uint32_t r) { todo("implement"); }
// static inline uint32_t cp14_bvr0_get(void) { todo("implement"); }

// return 1 if enabled, 0 otherwise.  
//    - we wind up reading the status register a bunch:
//      could return its value instead of 1 (since is 
//      non-zero).
static inline int cp14_is_enabled(void) {
    // Check bits 14 and 15 of DSCR
    // coproc=C14, Opcode_1 = 0, Crn = 0,
    // Crm = 0b1 (c1), Opcode_2 = 0
    uint32_t v = cp14_dscr_get();
    
    if (bit_is_off(v, 14) && bit_is_on(v, 15))
        return 1;
    return 0;
}

// enable debug coprocessor 
static inline void cp14_enable(void) {
    // if it's already enabled, just return
    if(cp14_is_enabled())
        return;

    // for the core to take a debug exception, monitor debug 
    // mode has to be both selected and enabled --- bit 14 
    // clear and bit 15 set.
    uint32_t s = cp14_status_get();

    s = bit_clr(s, 14);
    s = bit_set(s, 15);

    cp14_dscr_set(s);
    assert(cp14_is_enabled());
}

// disable debug coprocessor
static inline void cp14_disable(void) {
    if(!cp14_is_enabled())
        return;

    uint32_t s = cp14_status_get();
    s = bit_clr(s, 15);
    cp14_status_set(s);

    assert(!cp14_is_enabled());
}

static inline int cp14_bcr0_is_enabled(void) {
    uint32_t v = cp14_bcr0_get(); // p. 13-17
    return bit_is_on(v, 0);
}

static inline void cp14_bcr0_enable(void) {
    uint32_t v = cp14_bcr0_get(); // p. 13-17
    v = bit_set(v, 0);
    cp14_bcr0_set(v);
}

static inline void cp14_bcr0_disable(void) {
    uint32_t v = cp14_bcr0_get(); // p. 13-17
    v = bit_clr(v, 0);
    cp14_bcr0_set(v);
}

// was this a brkpt fault?
static inline int was_brkpt_fault(void) {
    // use IFSR and then DSCR
    uint32_t ifsr_v = cp15_ifsr_get();
    // Check bit 10 is 0
    if (bit_get(ifsr_v, 10) != 0)
        return 0;

    // TODO: Do we need to check if instruction debug event
    // p. 6-34 (arm 351)
    // e.g. b0010
    // if (bits_get(ifsr_v, 0, 3) != 0b0010) {
    //     return 0;
    // }

    uint32_t dscr_v = cp14_dscr_get();
    if (bits_get(dscr_v, 2, 5) == 0b1) {
        return 1;
    }
    return 0;
}

// was watchpoint debug fault caused by a load?
static inline int datafault_from_ld(void) {
    return bit_isset(cp15_dfsr_get(), 11) == 0;
}
// ...  by a store?
static inline int datafault_from_st(void) {
    return !datafault_from_ld();
}

// 13-33: tabl 13-23
static inline int was_watchpt_fault(void) {
    // use DFSR then DSCR
    uint32_t dfsr_v = cp15_dfsr_get();
    if (bit_get(dfsr_v, 10) != 0) {
        printk("here");
        return 0;
    }

    // TODO: Do we need to check if instruction debug event
    // TODO: And is it an instruction debug event for watchpt?
    // p. 6-34 (arm 351)
    // e.g. b0010
    // if (bits_get(ifsr_v, 0, 3) != 0b0010) {
    //     return 0;
    // }

    uint32_t dscr_v = cp14_dscr_get();
    if (bits_get(dscr_v, 2, 5) == 0b10) {
        return 1;
    }
    return 0;
}

static inline int cp14_wcr0_is_enabled(void) { // p. 13-22
    uint32_t v = cp14_wcr0_get();
    return bit_is_on(v, 0);
}

static inline void cp14_wcr0_enable(void) {
    uint32_t v = cp14_wcr0_get();
    v = bit_set(v, 0);
    cp14_wcr0_set(v);
}

static inline void cp14_wcr0_disable(void) {
    uint32_t v = cp14_wcr0_get();
    v = bit_clr(v, 0);
    cp14_wcr0_set(v);
}

// Get watchpoint fault using WFAR
static inline uint32_t watchpt_fault_pc(void) {
    uint32_t v = cp14_wfar_get();
    return v - 0x8; // p. 13-12: ARM state, the WFAR contains the address of the instruction causing it plus 0x8
}
    
#endif
