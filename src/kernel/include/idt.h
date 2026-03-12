/*
 * MelonOS - IDT (Interrupt Descriptor Table)
 * Handles CPU exceptions and hardware interrupts
 */

#ifndef IDT_H
#define IDT_H

#include <stdint.h>

/* Registers struct passed to handlers */
typedef struct {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} registers_t;

/* IDT entry */
typedef struct {
    uint16_t base_low;
    uint16_t sel;
    uint8_t  always0;
    uint8_t  flags;
    uint16_t base_high;
} __attribute__((packed)) idt_entry_t;

/* IDT pointer */
typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_ptr_t;

/* GDT entry */
typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed)) gdt_entry_t;

/* GDT pointer */
typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) gdt_ptr_t;

/* IRQ handler function type */
typedef void (*irq_handler_t)(registers_t *);

/* Initialize GDT */
void gdt_init(void);

/* Initialize IDT */
void idt_init(void);

/* Register an IRQ handler */
void irq_install_handler(int irq, irq_handler_t handler);

/* Unregister an IRQ handler */
void irq_uninstall_handler(int irq);

/* Assembly-defined ISR stubs */
extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);

/* Assembly-defined IRQ stubs */
extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

/* Assembly routines */
extern void gdt_flush(uint32_t);
extern void idt_load(uint32_t);

#endif /* IDT_H */
