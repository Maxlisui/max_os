#pragma once

#include <arch.h>
#include <stivale2.h>

namespace PMM {

extern "C" u64 kernel_end_address;

#define ALIGN_UP(addr, size) \
    ((addr % size == 0) ? (addr) : (addr) + size - ((addr) % size))

#define ALIGN_DOWN(addr, size) ((addr) - ((addr) % size))

bool init_pmm(stivale2_struct_tag_memmap* mmap_tag);
void* allocate_zero(u64 length);
}