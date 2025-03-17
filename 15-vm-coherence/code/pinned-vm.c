// put your code here.
//
#include "rpi.h"
#include "libc/bit-support.h"

// has useful enums and helpers.
#include "vector-base.h"
#include "pinned-vm.h"
#include "mmu.h"
#include "procmap.h"

// generate the _get and _set methods.
// (see asm-helpers.h for the cp_asm macro 
// definition)
// arm1176.pdf: 3-149

static void *null_pt = 0;

// should we have a pinned version?
cp_asm_set(cp15_domain, p15, 0, c3, c0, 0) 
void domain_access_ctrl_set(uint32_t d) {
    // Set cp15_domain (B4-42)
    cp15_domain_set(d); // automatically does a prefetch flush
    assert(domain_access_ctrl_get() == d);
}

// fill this in based on the <1-test-basic-tutorial.c>
// NOTE: 
//    you'll need to allocate an invalid page table
void pin_mmu_init(uint32_t domain_reg) {
    // staff_pin_mmu_init(domain_reg);
    // return;

    // 1. Initialize the hardware
    mmu_init(); 

    // 2. Create the invalid page table
    null_pt = kmalloc_aligned(4096*4, 1<<14);
    assert((uint32_t)null_pt % (1<<14) == 0);
    assert(!mmu_is_enabled());

    // 3. Set the domain register
    domain_access_ctrl_set(domain_reg); 
    return;
}

cp_asm_set(va_to_pa_write, p15, 0, c7, c8, 1) // p 3-82 (privileged write)
cp_asm_get(pa_reg, p15, 0, c7, c4, 0) // 3-82 (top)

// do a manual translation in tlb:
//   1. store result in <result>
//   2. return 1 if entry exists, 0 otherwise.
//
// NOTE: mmu must be on (confusing).
int tlb_contains_va(uint32_t *result, uint32_t va) {
    assert(mmu_is_enabled());

    // 3-79
    assert(bits_get(va, 0,2) == 0);

    va_to_pa_write_set(va);
    uint32_t res = pa_reg_get();
    uint32_t ret_val = !bit_get(res, 0);

    if (ret_val == 1) {
        res = bits_clr(res, 0, 9);
        res |= (va & 0xFFF); // The page offset is 12 bits for 4KB
        *result = res;
    } else {
        res >>= 1;
        *result = res;
    }
    return ret_val;
    // return staff_tlb_contains_va(result, va);
}

cp_asm(lockdown_index, p15, 5, c15, c4, 2)
cp_asm(lockdown_va, p15, 5, c15, c5, 2)
cp_asm(lockdown_pa, p15, 5, c15, c6, 2)
cp_asm(lockdown_attr, p15, 5, c15, c7, 2)

// map <va>-><pa> at TLB index <idx> with attributes <e>
void pin_mmu_sec(unsigned idx,  
                uint32_t va, 
                uint32_t pa,
                pin_t e) {


    demand(idx < 8, lockdown index too large);
    // lower 20 bits should be 0.
    demand(bits_get(va, 0, 19) == 0, only handling 1MB sections);
    demand(bits_get(pa, 0, 19) == 0, only handling 1MB sections);

    debug("about to map %x->%x\n", va,pa);

    // delete this and do add your code below.
    // staff_pin_mmu_sec(idx, va, pa, e);
    // return;

    // these will hold the values you assign for the tlb entries.
    uint32_t x, va_ent, pa_ent, attr;
    x = idx;

    // p 3-150
    va_ent = bits_clr(va, 0, 11);
    va_ent = bits_set(va_ent, 9, 9, e.G);
    if (!e.G) {
        va_ent = bits_set(va_ent, 0, 7, e.asid);
    }
    // output("[1]\n");
 
    // P 3-150
    pa_ent = bits_set(pa, 6, 7, e.pagesize);
    pa_ent = bits_set(pa_ent, 1, 3, e.AP_perm);
    pa_ent = bit_set(pa_ent, 0); // Valid
    // output("[2]\n");

    attr = 0x0;
    attr = bits_set(attr, 7, 10, e.dom);
    attr = bits_set(attr, 1, 5, e.mem_attr);
    // // output("[3]\n");


    // todo("assign these variables!\n");
    lockdown_index_set(x);
    lockdown_va_set(va_ent);
    lockdown_attr_set(attr);
    lockdown_pa_set(pa_ent);

    // unimplemented();

#if 0
    if((x = lockdown_va_get()) != va_ent)
        panic("lockdown va: expected %x, have %x\n", va_ent,x);
    if((x = lockdown_pa_get()) != pa_ent)
        panic("lockdown pa: expected %x, have %x\n", pa_ent,x);
    if((x = lockdown_attr_get()) != attr)
        panic("lockdown attr: expected %x, have %x\n", attr,x);
#endif
}

// check that <va> is pinned.  
int pin_exists(uint32_t va, int verbose_p) {
    if(!mmu_is_enabled())
        panic("XXX: i think we can only check existence w/ mmu enabled\n");

    uint32_t r;
    if(tlb_contains_va(&r, va)) {
        assert(va == r);
        return 1;
    } else {
        if(verbose_p) 
            output("TLB should have %x: returned %x [reason=%b]\n", 
                va, r, bits_get(r,1,6));
        return 0;
    }
}

// look in test <1-test-basic.c> to see what to do.
// need to set the <asid> before turning VM on and 
// to switch processes.
void pin_set_context(uint32_t asid) {
    // put these back
    demand(asid > 0 && asid < 64, invalid asid);
    demand(null_pt, must setup null_pt --- look at tests);

    mmu_set_ctx(128, asid, null_pt);
    // staff_pin_set_context(asid);
    // return;
}

void pin_clear(unsigned idx)  {
    // uint32_t v = lockdown_index_get();
    // v = bit_clr(v, idx);
    lockdown_index_set(idx);
    // tlb_va_set(0x0);
    // tlb_pa_set(0x0);
    // staff_pin_clear(idx);
}

void staff_lockdown_print_entry(unsigned idx);

void lockdown_print_entry(unsigned idx) {
    // staff_lockdown_print_entry(idx);

    trace("   idx=%d\n", idx);
    lockdown_index_set(idx);
    uint32_t va_ent = lockdown_va_get();
    uint32_t pa_ent = lockdown_pa_get();
    unsigned v = bit_get(pa_ent, 0);

    if(!v) {
        trace("     [invalid entry %d]\n", idx);
        return;
    }

    // 3-149
    uint32_t va = bits_get(va_ent, 12, 31);
    uint32_t G = bits_get(va_ent, 9, 9);
    uint32_t asid = bits_get(va_ent, 0, 7);
    trace("     va_ent=%x: va=%x|G=%d|ASID=%d\n",
        va_ent, va, G, asid);

    // 3-150
    uint32_t pa = bits_get(pa_ent, 12, 31);
    uint32_t nsa = bit_get(pa_ent, 9);
    uint32_t nstid = bit_get(pa_ent, 8);
    uint32_t size = bits_get(pa_ent, 6, 7);
    uint32_t apx = bits_get(pa_ent, 1, 3);
    trace("     pa_ent=%x: pa=%x|nsa=%d|nstid=%d|size=%b|apx=%b|v=%d\n",
                pa_ent, pa, nsa,nstid,size, apx,v);

    // 3-151
    uint32_t attr = lockdown_attr_get();
    uint32_t dom = bits_get(attr, 7, 10);
    uint32_t xn = bit_get(attr, 6);
    uint32_t tex = bits_get(attr, 3, 5);
    uint32_t C = bit_get(attr, 2);
    uint32_t B = bit_get(attr, 1);
    trace("     attr=%x: dom=%d|xn=%d|tex=%b|C=%d|B=%d\n",
            attr, dom,xn,tex,C,B);
}

void lockdown_print_entries(const char *msg) {
    trace("-----  <%s> ----- \n", msg);
    trace("  pinned TLB lockdown entries:\n");
    for(int i = 0; i < 8; i++)
        lockdown_print_entry(i);
    trace("----- ---------------------------------- \n");
}
