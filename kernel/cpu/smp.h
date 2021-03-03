#pragma once

#include <arch.h>
#include <backtrace.h>
#include <cpu/gdt.h>
#include <cpu/idt.h>
#include <types.h>

namespace SMP {

static const u8 MAX_CPU_COUNT = 64;

struct cpu {
    u64 stack_base;
    u64 current_processor_id;
    IDT::idtr cidt;
    GDT::gdtr cgdt;
    GDT::tss ctss;
    static const u32 stack_size = 8192;
    u8 stack_data[stack_size] PAGE_ALIGN;
    u8 stack_data_interrupt[stack_size] PAGE_ALIGN;
    u8 syscall_stack[stack_size] PAGE_ALIGN;
    u64 lapic_id;
    u64* page_table;
    u64 fpu_data[128] __attribute__((aligned(16)));
    Backtrace::backtrace local_backtrace;
};

extern cpu procData[MAX_CPU_COUNT];

void load_sse(cpu* cpu, u64* data);
void save_sse(cpu* cpu, u64* data);

inline cpu* get_current_cpu()
{
    u64 cc = 0;
    asm volatile("mov %0, fs \n"
                 : "=r"(cc));

    return &procData[cc];
}

inline cpu* get_current_cpu(int id)
{

    return &procData[id];
}

}