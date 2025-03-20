// first test do low level setup of everything.
#include "rpi.h"
#include "pt-vm.h"
#include "armv6-pmu.h"
#include "cache-support.h"

enum { OneMB = 1024*1024, cache_addr = OneMB * 10 };
enum { DIM = 128 };

void matmul(int A[DIM][DIM], int B[DIM][DIM], int C[DIM][DIM]) {
    // Calculate A@B and store it in C
    for (int i = 0; i < DIM; ++i) {
        for (int j = 0; j < DIM; ++j) {
            int sum = 0;
            for (int k = 0; k < DIM; ++k) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}

void init(int A[DIM][DIM], int B[DIM][DIM]) {
    for (int i = 0; i < DIM; ++i) {
        for (int j = 0; j < DIM; ++j) {
            A[i][j] = timer_get_usec() % 14;
            B[i][j] = -1 * timer_get_usec() % 12;
        }
    }
}

void notmain(void) { 
    // sleazy hack: create a bx lr nop routine on the page.
    volatile uint32_t *code_ptr = (void*)(cache_addr+128);
    unsigned i;
    for(i = 0; i < 256; i++)
        code_ptr[i] = 0xe320f000; // nop
    code_ptr[i] = 0xe12fff1e;   // bx lr
    dmb();

    // should probably make a bunch of adds.
    uint32_t (*fp)(uint32_t x) = (void *)code_ptr;
    assert(fp(5) == 5);
    assert(fp(2) == 2);

    // map the heap: for lab cksums must be at 0x100000.
    kmalloc_init_set_start((void*)OneMB, OneMB);
    enum { dom_kern = 1 };

    procmap_t p = procmap_default_mk(dom_kern);
    vm_pt_t *pt = vm_map_kernel(&p,0);
    assert(!mmu_is_enabled());

    // should also try it after.
    pin_t attr = pin_mk_global(dom_kern, perm_rw_priv, MEM_wb_alloc);

    vm_map_sec(pt, cache_addr, cache_addr, attr);
    vm_mmu_enable();
    assert(mmu_is_enabled());

    volatile uint32_t *ptr = (void*)cache_addr;

    uint32_t begin = pmu_cycle_get();
    // matrix multiply
    static int A[DIM][DIM], B[DIM][DIM], C[DIM][DIM];
    init(A, B);
    matmul(A, B, C);
    // end cycle count
    // print out the difference
    uint32_t end = pmu_cycle_get();
    trace("difference (no cache): %d cycles\n", end - begin);


    // turn all caches on.

    caches_all_on();
    assert(caches_all_on_p());

    uint32_t begin_cache = pmu_cycle_get();
    static int X[DIM][DIM], Y[DIM][DIM], Z[DIM][DIM];
    init(X, Y);
    matmul(X, Y, Z);
    uint32_t end_cache = pmu_cycle_get();
    trace("difference (with cache): %d cycles\n", end_cache - begin_cache);
}
