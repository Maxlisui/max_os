#include <backtrace.h>
#include <cpu/idt.h>
#include <cpu/serial.h>
#include <cpu/smp.h>

#define IDT_ENTRY_COUNT 256
#define INTGATE 0x8e
#define TRAPGATE 0xeF
#define INT_USER 0x60

namespace IDT {

const char* interrupt_exception_name[] = {
    "Division By 0",
    "Debug interrupt",
    "NMI (Non Maskable Interrupt)",
    "Breakpoint interrupt",
    "invalid (4)", // int 4 is not valid in 64 bit
    "table overflow",
    "Invalid opcode",
    "No FPU",
    "Double fault",
    "invalid (9)", // int 9 is not used
    "invalid TSS",
    "Segment not present",
    "invalid stack",
    "General protection fault",
    "Page fault",
    "invalid (15)",
    "x87 FPU fault",
    "Alignment fault",
    "Machine check fault",
    "SIMD floating point exception",
    "vitualisation excpetion",
    "control protection exception",
    "invalid (22)",
    "invalid (23)",

    "invalid (24)",
    "invalid (25)",
    "invalid (26)",
    "invalid (27)",
    "invalid (28)",
    "invalid (29)",
    "invalid (30)",
    "invalid (31)"
};

bool error = false;

idt_entry idt[IDT_ENTRY_COUNT];

idtr idt_descriptor = {
    .size = sizeof(idt_entry) * IDT_ENTRY_COUNT,
    .offset = (u64)&idt[0],
};

extern "C" void idt_flush(u64);

struct interrupt_handler_specific_array {
    irq_handler_func function_list[8]; // max 8 handler for an irq;
    int current_array_length = 0;
};

extern "C" u64 __interrupt_vector[128];
interrupt_handler_specific_array irq_array_handler[32];

void init_irq_handlers()
{
    for (int i = 0; i < 32; i++) {
        irq_array_handler[i].current_array_length = 0;
    }
}

void init_idt()
{
    Serial::serial_printf("Initializing IDT");

    for (u32 i = 0; i < 32 + 48; i++) {
        idt[i] = idt_entry((void*)__interrupt_vector[i], 0, INTGATE);
    }

    idt[127] = idt_entry((void*)__interrupt_vector[48], 0, INTGATE | INT_USER);
    idt[100] = idt_entry((void*)__interrupt_vector[49], 0, INTGATE | INT_USER);
    init_irq_handlers();
    idt_flush((u64)&idt_descriptor);

    Serial::serial_printf("IDT Initialized");
}

void update_backtrace(interrupt_stack_frame* frame)
{
    Backtrace::add_entry(&SMP::get_current_cpu()->local_backtrace, frame->rip);
}

bool is_interrupt_error(u8 intno)
{
    if (intno > 31) {
        return false;
    }
    if (intno == 1 || intno == 15 || (intno >= 21 && intno <= 29) || intno == 31) {
        return false;
    }
    return true;
}

void interrupt_error_handler(interrupt_stack_frame* frame)
{
    error = true;

    Serial::serial_printf("FATAL Interrupt Error %x", frame->rip);
    Serial::serial_printf("ID: %x", frame->int_no);
    Serial::serial_printf("Type: %s", interrupt_exception_name[frame->int_no]);

    Backtrace::dump_backtrace(&SMP::get_current_cpu()->local_backtrace);

    while (true) {
        asm volatile("hlt");
    }
}

void call_irq_handlers(unsigned int irq, interrupt_stack_frame* frame)
{
    interrupt_handler_specific_array& target = irq_array_handler[irq];
    if (target.current_array_length == 0) {
        return;
    }
    for (int i = 0; i < target.current_array_length; i++) {
        target.function_list[i](irq);
    }
}

extern "C" u64 interrupts_handler(interrupt_stack_frame* frame)
{
    u64 result = (u64)frame;
    update_backtrace(frame);

    if (error) {
        while (error)
            ;
    }

    if (is_interrupt_error(frame->int_no)) {
        interrupt_error_handler(frame);
    }

    if (frame->int_no == 0x7f) {
        // Syscall
    } else if (frame->int_no == 32) {
        // Process switch
    } else if (frame->int_no > 32 && frame->int_no < 64) {
        call_irq_handlers(frame->int_no - 32, frame);
    } else if (frame->int_no == 100) {
        // Process switch
    }

    // apic::the()->EOI();

    return result;
}

}