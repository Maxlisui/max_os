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

void setup_gdt_descriptors(gdt_descriptor_array<GDT_DESCRIPTORS>* target, u64 tss_base, u64 tss_limit)
{
    target->zero();

    target->set(gdt_selector::KERNEL_CODE, gdt_descriptor(gdt_flags::CS | gdt_flags::WRITABLE, gdt_granularity::LONG_MODE_GRANULARITY));
    target->set(gdt_selector::KERNEL_DATA, gdt_descriptor(gdt_flags::DS | gdt_flags::WRITABLE, 0));
    target->set(gdt_selector::USER_CODE, gdt_descriptor(gdt_flags::CS | gdt_flags::WRITABLE | gdt_flags::USER, gdt_granularity::LONG_MODE_GRANULARITY));
    target->set(gdt_selector::USER_DATA, gdt_descriptor(gdt_flags::DS | gdt_flags::WRITABLE | gdt_flags::USER, 0));

    target->xset(gdt_selector::TSS_SELECTOR, gdt_xdescriptor(gdt_flags::TSS, tss_base, tss_limit));

    // TODO
    gdt_descriptors[0].fill_gdt_register(&SMP::get_current_cpu()->cgdt);
}

void gdt_init()
{
    Serial::serial_printf("Initializing GDT");

    SMP::cpu* current_cpu = SMP::get_current_cpu();

    u64 tss_base = (u64)&current_cpu->ctss;
    u64 tss_limit = tss_base + sizeof(current_cpu->ctss);

    setup_gdt_descriptors(&gdt_descriptors[0], tss_base, tss_limit);

    gdtr_install(&SMP::get_current_cpu()->cgdt, gdt_selector::KERNEL_CODE, gdt_selector::KERNEL_DATA);

    Serial::serial_printf("GDT Initialized");
}

}