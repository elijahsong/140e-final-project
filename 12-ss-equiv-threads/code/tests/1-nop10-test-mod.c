// run N copies of the same thread together.
#include "rpi.h"
#include "eqx-threads.h"
#include "expected-hashes.h"
#include "interleave-checker.h"

enum { N = 20 };

void notmain(void) {
    int res = interleave_check(nop_10, nop_10, N, 0, 10);
    output("success !!!\n");
}
