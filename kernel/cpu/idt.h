#pragma once

#include <cpu/gdt.h>
#include <types.h>

namespace IDT {

typedef void (*irq_handler_func)(unsigned int irq);

struct idtr {
    u16 size;   // size of the IDT table
    u64 offset; // address of the IDT table
} __attribute__((packed));

struct idt_entry {
    u16 offset_low16;
    u16 cs;
    u8 ist;
    u8 attributes;
    u16 offset_mid16;
    u32 offset_high32;
    u32 zero;

public:
    idt_entry() {};

    idt_entry(void* idt_handler, u8 idt_ist, u8 idt_type)
    {
        zero = 0;

        cs = GDT::gdt_selector::KERNEL_CODE;
        ist = idt_ist;
        attributes = idt_type;

        u64 target_handler = (u64)idt_handler;
        offset_low16 = (u16)target_handler;
        offset_mid16 = (u16)(target_handler >> 16);
        offset_high32 = (u32)(target_handler >> 32);
    };
} __attribute__((packed));

struct interrupt_stack_frame {
    u64 r15;
    u64 r14;
    u64 r13;
    u64 r12;
    u64 r11;
    u64 r10;
    u64 r9;
    u64 r8;
    u64 rbp;
    u64 rdi;
    u64 rsi;
    u64 rdx;
    u64 rcx;
    u64 rbx;
    u64 rax;

    // Contains error code and interrupt number for exceptions
    // Contains syscall number for syscalls
    // Contains just the interrupt number otherwise
    u64 int_no;
    u64 error_code;
    // Interrupt stack frame
    u64 rip;
    u64 cs;
    u64 rflags;
    u64 rsp;
    u64 ss;
} __attribute__((packed));

void init_idt();

}