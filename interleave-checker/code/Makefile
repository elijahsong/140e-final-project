# "make checkoff" should run a bunch of tests.
#


# ====== Run all new tests =====
# PROGS := $(wildcard new-tests/[1234567]*.c)
# PROGS += $(wildcard new-tests/[1]*.c)  # basic test
# PROGS += $(wildcard new-tests/[2]*.c)  # nop1 test
# PROGS += $(wildcard new-tests/[3]*.c)  # nop10 test
# PROGS += $(wildcard new-tests/[4]*.c)  # mov-ident-small1 test
# PROGS += $(wildcard new-tests/[5]*.c)  # no-stack-interleaved-five test
# PROGS += $(wildcard new-tests/[6]*.c)  # no-stack-interleaved-three test
PROGS += $(wildcard new-tests/[7]*.c)  # stack-interleaved test
# PROGS += $(wildcard new-tests/[8]*.c)  # memory reordering test
# PROGS += $(wildcard new-tests/[9]*.c)  # shared array should fail
# PROGS += new-tests/10-deterministic-arithmetic.c
# PROGS += new-tests/11-sequential-state-machine.c

# ==== Lab 15/17 Page Table VM Tests ====
# PROGS := $(wildcard pt-tests/[1234]-*c)

# ==== Lab 12 Tests, along with new versions ====
# note: 
#  - the 4,3,2 tests won't work with make check
#  - but they should pass their internal checks.
# PROGS = tests/4-cool-test.c

# PROGS = tests/3-stacåk-test.c
# PROGS += new-tests/3-stack-interleaved.c

# PROGS = tests/2-no-stack-test.c
# PROGS += new-tests/2-no-stack-interleaved.c

# PROGS += tests/1-mov-test.c
# PROGS += new-tests/1-mov-test-mod.c

# PROGS += tests/1-nop1-test.c
# PROGS += new-tests/1-nop1-test-mod.c

# PROGS += tests/1-nop10-test.c
# PROGS += new-tests/1-nop10-test-mod.c

# # very simple tests. start here.
# PROGS += tests/0-nop1-test.c
# PROGS += tests/0-small1-test.c
# PROGS += tests/0-nop10-test.c
# PROGS += tests/0-mov-test.c

# we start with this b/c it's cute.
# PROGS = tests/4-cool-test.c

# we give you these files code.  you can start tweaking them
# when pass the tests.
COMMON_SRC += eqx-threads.c
# COMMON_SRC += breakpoint.c
STAFF_OBJS += staff-breakpoint.o
COMMON_SRC += staff-full-except.c

COMMON_SRC += interleave-checker.c

# this calls staff: replace routines 1 by 1.
COMMON_SRC += switchto-asm.S

# this calls staff: replace routines 1 by 1.
COMMON_SRC += full-except-asm.S

# More helpers (Lab 15 and 17)
COMMON_SRC += mmu-helpers.c
COMMON_SRC += pt-vm.c
COMMON_SRC += your-mmu-asm.S
COMMON_SRC += mmu.c

# we give you a kmalloc
STAFF_OBJS += $(CS140E_2025_PATH)/libpi/staff-objs/kmalloc.o

# as you replace routines comment these out.
# STAFF_OBJS += staff-full-except-asm.o
# STAFF_OBJS += staff-switchto-asm.o

BOOTLOADER=my-install
RUN = 1
CAN_EMIT=1

# look in here for the stackless tests.
OUR_START = staff-start.S

EXCLUDE ?= grep -v simple_boot
# GREP_STR := 'TRACE:\|THREAD:\|ERROR:\|PANIC:'

GREP_STR := 'TRACE:\|SUCCESS:\|ERROR:\|PANIC:'

include $(CS140E_2025_PATH)/libpi/mk/Makefile.robust

# START_ASM = staff-start.S
# include $(CS140E_2025_PATH)/libpi/mk/Makefile.template-fixed

checkoff:
	make -f ./checkoff.mk

# Added this for randomm support
LIB_POST += $(CS140E_2025_PATH)/lib/libgcc.a