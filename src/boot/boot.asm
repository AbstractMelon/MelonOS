; MelonOS Bootloader - Multiboot compliant entry point
; This sets up the stack and jumps to the C kernel

MBALIGN  equ 1 << 0            ; Align loaded modules on page boundaries
MEMINFO  equ 1 << 1            ; Provide memory map
FLAGS    equ MBALIGN | MEMINFO ; Multiboot flag field
MAGIC    equ 0x1BADB002        ; Magic number for multiboot
CHECKSUM equ -(MAGIC + FLAGS)  ; Checksum

; Multiboot header
section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

; Stack
section .bss
align 16
stack_bottom:
    resb 16384                 ; 16 KiB stack
stack_top:

; Entry point
section .text
global _start
extern kernel_main

_start:
    mov esp, stack_top         ; Set up the stack pointer
    push ebx                   ; Push multiboot info pointer
    push eax                   ; Push multiboot magic number
    call kernel_main           ; Call the C kernel
    cli                        ; Disable interrupts
.hang:
    hlt                        ; Halt the CPU
    jmp .hang                  ; Loop forever if we somehow continue

; GDT flush routine
global gdt_flush
gdt_flush:
    mov eax, [esp + 4]
    lgdt [eax]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush
.flush:
    ret

; IDT load routine
global idt_load
idt_load:
    mov eax, [esp + 4]
    lidt [eax]
    ret

; ISR stubs - CPU exceptions
%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    cli
    push dword 0               ; Dummy error code
    push dword %1              ; Interrupt number
    jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
global isr%1
isr%1:
    cli
    push dword %1              ; Interrupt number (error code already pushed)
    jmp isr_common_stub
%endmacro

; IRQ stubs
%macro IRQ 2
global irq%1
irq%1:
    cli
    push dword 0
    push dword %2
    jmp irq_common_stub
%endmacro

; CPU exceptions
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_NOERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

; Hardware IRQs (mapped to interrupts 32-47)
IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

; Common ISR stub
extern isr_handler
isr_common_stub:
    pusha                      ; Push all general registers
    mov ax, ds
    push eax                   ; Save data segment

    mov ax, 0x10               ; Load kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp                   ; Push pointer to registers struct
    call isr_handler
    add esp, 4

    pop eax                    ; Restore data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa
    add esp, 8                 ; Clean up error code and ISR number
    iret

; Common IRQ stub
extern irq_handler
irq_common_stub:
    pusha
    mov ax, ds
    push eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp
    call irq_handler
    add esp, 4

    pop ebx
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx

    popa
    add esp, 8
    iret
