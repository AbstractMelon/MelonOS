[bits 32]
[extern kernel_main] ; Declare that we will be referencing the external symbol 'kernel_main'

; Kernel entry point
global _start
_start:
    ; Call C function
    call kernel_main
    
    ; Halt the CPU if kernel_main returns
    cli     ; Disable interrupts
    hlt     ; Halt the CPU
