[bits 16]
[org 0x7c00]

KERNEL_OFFSET equ 0x1000 ; Memory offset where we will load our kernel

    ; Initialize segment registers
    mov ax, 0
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x9000 ; Set up stack

    ; Save boot drive number
    mov [BOOT_DRIVE], dl

    ; Display boot message
    mov si, MSG_BOOT
    call print_string

    ; Load kernel from disk
    call load_kernel

    ; Switch to protected mode
    call switch_to_pm

    ; We should never reach here
    jmp $

; Load kernel from disk
load_kernel:
    mov si, MSG_LOAD_KERNEL
    call print_string

    mov bx, KERNEL_OFFSET ; Set destination address
    mov dh, 15            ; Load 15 sectors (7.5KB)
    mov dl, [BOOT_DRIVE]  ; Drive number
    
    mov ah, 0x02          ; BIOS read sector function
    mov al, dh            ; Number of sectors to read
    mov ch, 0             ; Cylinder number
    mov dh, 0             ; Head number
    mov cl, 2             ; Start from sector 2 (sector 1 is our boot sector)
    
    int 0x13              ; BIOS interrupt
    
    jc disk_error         ; Jump if error (carry flag set)
    
    cmp al, 15            ; Check if we read all 15 sectors
    jne disk_error        ; Jump if error
    
    ret

; Error handler for disk operations
disk_error:
    mov si, MSG_DISK_ERROR
    call print_string
    jmp $

; Print a null-terminated string
print_string:
    pusha
    mov ah, 0x0e ; BIOS teletype function

.loop:
    lodsb        ; Load byte at SI into AL and increment SI
    or al, al    ; Check if AL is 0 (end of string)
    jz .done     ; If zero, we're done
    int 0x10     ; Print the character
    jmp .loop    ; Repeat for next character

.done:
    popa
    ret

; GDT for protected mode
gdt_start:

gdt_null:       ; Null descriptor (required)
    dd 0x0
    dd 0x0

gdt_code:       ; Code segment descriptor
    dw 0xffff   ; Limit (bits 0-15)
    dw 0x0      ; Base (bits 0-15)
    db 0x0      ; Base (bits 16-23)
    db 10011010b ; Flags (present, privilege, type) + Type flags
    db 11001111b ; Flags (granularity, size) + Limit (bits 16-19)
    db 0x0      ; Base (bits 24-31)

gdt_data:       ; Data segment descriptor
    dw 0xffff   ; Limit (bits 0-15)
    dw 0x0      ; Base (bits 0-15)
    db 0x0      ; Base (bits 16-23)
    db 10010010b ; Flags (present, privilege, type) + Type flags
    db 11001111b ; Flags (granularity, size) + Limit (bits 16-19)
    db 0x0      ; Base (bits 24-31)

gdt_end:

; GDT descriptor
gdt_descriptor:
    dw gdt_end - gdt_start - 1 ; Size of GDT
    dd gdt_start               ; Address of GDT

; Define segment selectors
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; Switch to protected mode
switch_to_pm:
    cli                     ; Disable interrupts
    lgdt [gdt_descriptor]   ; Load GDT

    ; Set PE (Protection Enable) bit in CR0
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    ; Far jump to 32-bit code
    jmp CODE_SEG:init_pm

[bits 32]
; Initialize protected mode
init_pm:
    ; Update segment registers
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Update stack pointer
    mov ebp, 0x90000
    mov esp, ebp

    ; Jump to kernel
    call KERNEL_OFFSET
    
    ; We should never reach here
    jmp $

; Data
BOOT_DRIVE db 0
MSG_BOOT db 'Booting MelonOS...', 0x0D, 0x0A, 0
MSG_LOAD_KERNEL db 'Loading kernel into memory...', 0x0D, 0x0A, 0
MSG_DISK_ERROR db 'Error loading kernel from disk!', 0x0D, 0x0A, 0

; Boot sector padding and signature
times 510-($-$$) db 0
dw 0xaa55
