#pragma once

#include <types.h>

#define CPU_SLEEP(expr) \
    while (expr) { }

namespace CPU {

static inline void write_to_port_u8(u16 port, u8 value)
{
    asm volatile("out dx, al" ::"a"(value), "d"(port));
}

static inline void write_to_por_u16(u16 port, u16 value)
{
    asm volatile("out dx, ax" ::"a"(value), "d"(port));
}

static inline void write_to_port_u32(u16 port, u32 value)
{
    asm volatile("out dx, eax" ::"a"(value), "d"(port));
}

static inline u8 read_from_port_u8(u16 port)
{
    u8 result = 0;
    asm volatile("in al, dx" ::"a"(result), "d"(port));
    return result;
}

static inline u16 read_from_port_u16(u16 port)
{
    u16 result = 0;
    asm volatile("in ax, dx" ::"a"(result), "d"(port));
    return result;
}

static inline u32 read_from_port_u32(u16 port)
{
    u32 result = 0;
    asm volatile("in eax, dx" ::"a"(result), "d"(port));
    return result;
}
}
