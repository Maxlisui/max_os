#include <cpu/gdt.h>
#include <cpu/serial.h>
#include <cpu/smp.h>

#define GDT_DESCRIPTORS 7

namespace GDT {

gdt_descriptor_array<GDT_DESCRIPTORS> gdt_descriptors[SMP::MAX_CPU_COUNT];

char tss_ist1[8192] __attribute__((aligned(16)));
char tss_ist2[8192] __attribute__((aligned(16)));
char tss_ist3[8192] __attribute__((aligned(16)));

extern "C" void gdtr_install(gdtr*, u16, u16);

void gdt_init()
{
    Serial::serial_printf("Initializing GDT");

    SMP::cpu* current_cpu = SMP::get_current_cpu();

    u64 tss_base = (u64)&current_cpu->ctss;
    u64 tss_limit = tss_base + sizeof(current_cpu->ctss);
}

}