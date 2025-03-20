#include "rpi-asm.h"

#define SYS_RESUME  1
#define SYS_TRYLOCK 2
#define SYS_TEST 3

@ user-level trampoline to call system call.
@
@ save lr b/c hardware will trash it (since we are at SUPER initially)
MK_FN(syscall_invoke_asm)
    push {lr}   @ we won't need these at user level.
    swi 0
    pop {lr}    @ we won't need these at user level.
    bx lr

@ implement a context switch for yield
@ Note i think there is an easier way to do this using cpsid
@ but this doesn't make use of SYS RESUME which is already written
@ for us.
MK_FN(context_switch)
    // Save current register state to memory pointed by r0
    str r0, [r0, #0]
    str r1, [r0, #4]
    str r2, [r0, #8]
    str r3, [r0, #12]
    str r4, [r0, #16]
    str r5, [r0, #20]
    str r6, [r0, #24]
    str r7, [r0, #28]
    str r8, [r0, #32]
    str r9, [r0, #36]
    str r10, [r0, #40]
    str r11, [r0, #44]
    str r12, [r0, #48]
    str sp, [r0, #52]   // Save stack pointer
    str lr, [r0, #60]   // Save link register
    
    // Save processor mode bits
    mrs r2, cpsr
    str r2, [r0, #64]
    
    // Invoke system call to resume execution
    mov r0, #SYS_RESUME
    b syscall_invoke_asm