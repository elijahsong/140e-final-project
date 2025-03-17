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
    stm r0, {r0-r14}
    str lr, [r0, #60]
    
    // Save processor mode bits
    mrs r2, cpsr
    and r2, #0x1F
    str r2, [r0, #64]
    
    // Invoke system call to resume execution
    mov r0, #SYS_RESUME
    b syscall_invoke_asm