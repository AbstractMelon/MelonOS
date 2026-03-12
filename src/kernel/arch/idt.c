/*
 * MelonOS - IDT (Interrupt Descriptor Table)
 * GDT setup, IDT setup, IRQ handling implementation
 */

#include "idt.h"
#include "io.h"
#include "vga.h"
#include "string.h"

/* GDT */
static gdt_entry_t gdt_entries[5];
static gdt_ptr_t gdt_ptr;

/* IDT */
static idt_entry_t idt_entries[256];
static idt_ptr_t idt_ptr;

/* IRQ handlers */
static irq_handler_t irq_handlers[16] = { 0 };

/* Exception messages */
static const char *exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved"
};

/* Set a GDT gate */
static void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;
    gdt_entries[num].limit_low   = (limit & 0xFFFF);
    gdt_entries[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt_entries[num].access      = access;
}

void gdt_init(void) {
    gdt_ptr.limit = (sizeof(gdt_entry_t) * 5) - 1;
    gdt_ptr.base  = (uint32_t)&gdt_entries;

    gdt_set_gate(0, 0, 0, 0, 0);                /* Null segment */
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); /* Kernel Code segment */
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); /* Kernel Data segment */
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); /* User Code segment */
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); /* User Data segment */

    gdt_flush((uint32_t)&gdt_ptr);
}

/* Set an IDT gate */
static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].base_low  = base & 0xFFFF;
    idt_entries[num].base_high = (base >> 16) & 0xFFFF;
    idt_entries[num].sel       = sel;
    idt_entries[num].always0   = 0;
    idt_entries[num].flags     = flags /* | 0x60 for user-mode */;
}

/* Remap the PIC (Programmable Interrupt Controller) */
static void pic_remap(void) {
    /* Save masks */
    uint8_t mask1 = inb(0x21);
    uint8_t mask2 = inb(0xA1);

    /* Start initialization sequence */
    outb(0x20, 0x11); io_wait();
    outb(0xA0, 0x11); io_wait();

    /* Set vector offsets */
    outb(0x21, 0x20); io_wait();  /* Master PIC: IRQs 0-7  -> interrupts 32-39 */
    outb(0xA1, 0x28); io_wait();  /* Slave PIC:  IRQs 8-15 -> interrupts 40-47 */

    /* Tell Master about Slave at IRQ2 */
    outb(0x21, 0x04); io_wait();
    outb(0xA1, 0x02); io_wait();

    /* 8086/88 mode */
    outb(0x21, 0x01); io_wait();
    outb(0xA1, 0x01); io_wait();

    /* Restore masks */
    outb(0x21, mask1);
    outb(0xA1, mask2);
}

void idt_init(void) {
    idt_ptr.limit = (sizeof(idt_entry_t) * 256) - 1;
    idt_ptr.base  = (uint32_t)&idt_entries;

    memset(&idt_entries, 0, sizeof(idt_entry_t) * 256);

    /* Remap the PIC */
    pic_remap();

    /* CPU exception ISRs (0-31) */
    idt_set_gate(0,  (uint32_t)isr0,  0x08, 0x8E);
    idt_set_gate(1,  (uint32_t)isr1,  0x08, 0x8E);
    idt_set_gate(2,  (uint32_t)isr2,  0x08, 0x8E);
    idt_set_gate(3,  (uint32_t)isr3,  0x08, 0x8E);
    idt_set_gate(4,  (uint32_t)isr4,  0x08, 0x8E);
    idt_set_gate(5,  (uint32_t)isr5,  0x08, 0x8E);
    idt_set_gate(6,  (uint32_t)isr6,  0x08, 0x8E);
    idt_set_gate(7,  (uint32_t)isr7,  0x08, 0x8E);
    idt_set_gate(8,  (uint32_t)isr8,  0x08, 0x8E);
    idt_set_gate(9,  (uint32_t)isr9,  0x08, 0x8E);
    idt_set_gate(10, (uint32_t)isr10, 0x08, 0x8E);
    idt_set_gate(11, (uint32_t)isr11, 0x08, 0x8E);
    idt_set_gate(12, (uint32_t)isr12, 0x08, 0x8E);
    idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E);
    idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E);
    idt_set_gate(15, (uint32_t)isr15, 0x08, 0x8E);
    idt_set_gate(16, (uint32_t)isr16, 0x08, 0x8E);
    idt_set_gate(17, (uint32_t)isr17, 0x08, 0x8E);
    idt_set_gate(18, (uint32_t)isr18, 0x08, 0x8E);
    idt_set_gate(19, (uint32_t)isr19, 0x08, 0x8E);
    idt_set_gate(20, (uint32_t)isr20, 0x08, 0x8E);
    idt_set_gate(21, (uint32_t)isr21, 0x08, 0x8E);
    idt_set_gate(22, (uint32_t)isr22, 0x08, 0x8E);
    idt_set_gate(23, (uint32_t)isr23, 0x08, 0x8E);
    idt_set_gate(24, (uint32_t)isr24, 0x08, 0x8E);
    idt_set_gate(25, (uint32_t)isr25, 0x08, 0x8E);
    idt_set_gate(26, (uint32_t)isr26, 0x08, 0x8E);
    idt_set_gate(27, (uint32_t)isr27, 0x08, 0x8E);
    idt_set_gate(28, (uint32_t)isr28, 0x08, 0x8E);
    idt_set_gate(29, (uint32_t)isr29, 0x08, 0x8E);
    idt_set_gate(30, (uint32_t)isr30, 0x08, 0x8E);
    idt_set_gate(31, (uint32_t)isr31, 0x08, 0x8E);

    /* Hardware IRQs (32-47) */
    idt_set_gate(32, (uint32_t)irq0,  0x08, 0x8E);
    idt_set_gate(33, (uint32_t)irq1,  0x08, 0x8E);
    idt_set_gate(34, (uint32_t)irq2,  0x08, 0x8E);
    idt_set_gate(35, (uint32_t)irq3,  0x08, 0x8E);
    idt_set_gate(36, (uint32_t)irq4,  0x08, 0x8E);
    idt_set_gate(37, (uint32_t)irq5,  0x08, 0x8E);
    idt_set_gate(38, (uint32_t)irq6,  0x08, 0x8E);
    idt_set_gate(39, (uint32_t)irq7,  0x08, 0x8E);
    idt_set_gate(40, (uint32_t)irq8,  0x08, 0x8E);
    idt_set_gate(41, (uint32_t)irq9,  0x08, 0x8E);
    idt_set_gate(42, (uint32_t)irq10, 0x08, 0x8E);
    idt_set_gate(43, (uint32_t)irq11, 0x08, 0x8E);
    idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E);
    idt_set_gate(45, (uint32_t)irq13, 0x08, 0x8E);
    idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E);
    idt_set_gate(47, (uint32_t)irq15, 0x08, 0x8E);

    idt_load((uint32_t)&idt_ptr);
}

/* ISR handler - called from assembly */
void isr_handler(registers_t *regs) {
    if (regs->int_no < 32) {
        vga_print_colored("\n!!! KERNEL PANIC: ", VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_print_colored(exception_messages[regs->int_no], VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_print_colored(" !!!\n", VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_print("System halted.\n");
        __asm__ volatile ("cli; hlt");
    }
}

/* IRQ handler - called from assembly */
void irq_handler(registers_t *regs) {
    /* Send EOI (End of Interrupt) */
    if (regs->int_no >= 40) {
        outb(0xA0, 0x20);  /* Slave PIC EOI */
    }
    outb(0x20, 0x20);      /* Master PIC EOI */

    /* Call registered handler */
    int irq_num = regs->int_no - 32;
    if (irq_num >= 0 && irq_num < 16 && irq_handlers[irq_num]) {
        irq_handlers[irq_num](regs);
    }
}

void irq_install_handler(int irq, irq_handler_t handler) {
    if (irq >= 0 && irq < 16) {
        irq_handlers[irq] = handler;
    }
}

void irq_uninstall_handler(int irq) {
    if (irq >= 0 && irq < 16) {
        irq_handlers[irq] = 0;
    }
}
