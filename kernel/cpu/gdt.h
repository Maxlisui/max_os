#pragma once

#include <types.h>

namespace GDT {

enum gdt_selector : u16 {
    NULL_SELECTOR = 0,
    KERNEL_CODE = 0x8,
    KERNEL_DATA = 0x10,
    USER_DATA = 0x1b,
    USER_CODE = 0x23,
    TSS_SELECTOR = 0x28,
};

enum gdt_flags : u8 {
    WRITABLE = 0b10,
    USER = 0b1100000,
    PRESENT = 0b10000000,
    TSS = 0b1001,
    DS = 0b10000,
    CS = 0b11000,
};

enum gdt_granularity : u8 {
    LONG_MODE_GRANULARITY = 0x2,
};

struct gdtr {
    u16 len;
    u64 addr;
} __attribute__((packed));

struct gdt_descriptor {
    u16 limit_low;
    u16 base_low;
    u8 base_mid;
    u8 flags;
    u8 granularity; /* and high limit */
    u8 base_high;

    gdt_descriptor() {};
    gdt_descriptor(u8 flag, u8 gran)
    {
        base_high = 0;
        base_mid = 0;
        base_low = 0;
        flags = flag | gdt_flags::PRESENT;
        granularity = (gran << 4) | 0x0F;
        limit_low = 0;
    }
} __attribute__((packed));

struct gdt_xdescriptor {
    gdt_descriptor low;

    struct
    {
        u32 base_xhigh;
        u32 reserved;
    } high;

    gdt_xdescriptor(u8 flag, u64 base, u64 limit)
    {
        low.flags = flag | gdt_flags::PRESENT;
        low.granularity = (0 << 4) | ((limit >> 16) & 0x0F);
        low.limit_low = limit & 0xFFFF;
        low.base_low = base & 0xFFFF;
        low.base_mid = ((base >> 16) & 0xFF);
        low.base_high = ((base >> 24) & 0xFF);
        high.base_xhigh = ((base >> 32) & 0xFFFFFFFF);
        high.reserved = 0;
    }
} __attribute__((packed));

struct tss {
    u32 reserved0;
    u64 rsp0;
    u64 rsp1;
    u64 rsp2;
    u64 reserved1;
    u64 ist1;
    u64 ist2;
    u64 ist3;
    u64 ist4;
    u64 ist5;
    u64 ist6;
    u64 ist7;
    u64 reserved2;
    u16 reserved3;
    u16 iomap_base;
} __attribute__((packed));

#define GDT_ARRAY_SEL(a) a / sizeof(gdt_descriptor)

template<int T>
struct gdt_descriptor_array {
    gdt_descriptor entrys[T];

    constexpr size_t size() const
    {
        return sizeof(gdt_descriptor) * T;
    }
    void zero()
    {
        memset(entrys, 0, sizeof(entrys[0]) * T);
    }
    gdt_descriptor* get_entry(gdt_selector entry)
    {
        return &entrys[GDT_ARRAY_SEL(entry)];
    }
    gdt_xdescriptor* get_entry_x(gdt_selector entry)
    {
        return (gdt_xdescriptor*)&entrys[GDT_ARRAY_SEL(entry)];
    }
    void set(gdt_selector entry, gdt_descriptor desc)
    {
        *get_entry(entry) = desc;
    }
    void xset(gdt_selector entry, gdt_xdescriptor desc)
    {
        *get_entry_x(entry) = desc;
    }
    gdt_descriptor* raw()
    {
        return entrys;
    }
    void fill_gdt_register(gdtr* target)
    {
        target->len = size();
        target->addr = (u64)raw();
    }
};

void tss_init(u64 i);
void tss_set_rsp0(u64 rsp0);

void gdt_init();
void gdt_ap_init();

}
