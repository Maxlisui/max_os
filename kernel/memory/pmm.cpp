#include <cpu/serial.h>
#include <memory/pmm.h>
#include <string.h>
#include <types.h>

namespace PMM {

struct physical_memory {
    usize size;
    u8* buffer;
    usize last_free;
};

static physical_memory memory;

void init_physical_memory(physical_memory* memory, u64 base_address, u64 length)
{
    memset((void*)base_address, 0xff, length / 8);
    memory->buffer = (u8*)base_address;
    memory->last_free = 0;
    memory->size = length;
}

bool set(physical_memory* memory, usize idx, bool used)
{
    if (idx > memory->size) {
        Serial::serial_printf("Trying to set Physical Memory Index %x > %x", idx, memory->size);
        return false;
    }

    usize bit = idx % 8;
    usize byte = idx / 8;

    if (used) {
        memory->buffer[byte] |= (1 << bit);
    } else {
        memory->buffer[byte] &= ~(1 << bit);
    }
    return true;
}

bool set_free(physical_memory* memory, usize idx, usize length)
{
    for (usize i = 0; i < length; i++) {
        if (!set(memory, idx + i, true)) {
            return false;
        }
    }
    return true;
}

bool init_pmm(stivale2_struct_tag_memmap* mmap_tag)
{
    Serial::serial_printf("Initializing PMM");

    u64 total_memory_length = mmap_tag->memmap[mmap_tag->entries - 1].length + mmap_tag->memmap[mmap_tag->entries - 1].base;
    u64 base_address = 0;
    u64 length = 0;

    for (u64 i = 0; i < mmap_tag->entries; i++) {
        stivale2_mmap_entry it = mmap_tag->memmap[i];

        if (it.type == 1 && it.length > (total_memory_length / PAGE_SIZE / 8)) {
            base_address = it.base + PAGE_SIZE;
            length = total_memory_length / PAGE_SIZE;
            break;
        }
    }

    init_physical_memory(&memory, base_address, length);

    for (u64 i = 0; i < mmap_tag->entries; i++) {
        stivale2_mmap_entry it = mmap_tag->memmap[i];

        if (it.type == 1) {
            if (!set_free(&memory, it.base / PAGE_SIZE, it.length / PAGE_SIZE)) {
                return false;
            }
        }
    }

    Serial::serial_printf("PMM Initialized");

    return true;
}
}