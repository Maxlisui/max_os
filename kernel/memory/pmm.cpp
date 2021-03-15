#include <cpu/serial.h>
#include <memory/pmm.h>
#include <mutex.h>
#include <string.h>
#include <types.h>

namespace PMM {

static mutex pmm_lock;

struct physical_memory {
    usize size;
    u8* buffer;
    usize last_free;
};

static physical_memory memory;
static u64 used_memory = 0;
static u64 available_memory = 0;

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
        if (!set(memory, idx + i, false)) {
            return false;
        }
    }
    memory->last_free = idx;
    return true;
}

bool set_used(physical_memory* memory, usize idx, usize length)
{
    for (usize i = 0; i < length; i++) {
        if (!set(memory, idx + i, true)) {
            return false;
        }
    }
    return true;
}

bool get(physical_memory* memory, usize idx)
{
    if (idx > memory->size) {
        Serial::serial_printf("Trying to set Physical Memory Index %x > %x", idx, memory->size);
        return false;
    }

    usize bit = idx % 8;
    usize byte = idx / 8;
    return (memory->buffer[byte] & (1 << (bit)));
}

usize find_free(physical_memory* memory, usize length)
{
    usize current_founded_length = 0;
    usize current_founded_idx = 0;

    for (usize i = memory->last_free; i < memory->size; i++) {
        if (i == 0) {
            continue;
        }

        if (!get(memory, i)) {
            if (current_founded_length == 0) {
                current_founded_idx = i;
            }
            current_founded_length++;
        } else {
            current_founded_length = 0;
            current_founded_idx = 0;
        }
        if (current_founded_length == length) {
            memory->last_free = current_founded_idx + current_founded_length;
            return current_founded_idx;
        }
    }

    if (memory->last_free == 0) {
        Serial::serial_printf("No Free Physical memory found!");
        return 0;
    } else {
        memory->last_free = 0;
        return find_free(memory, length);
    }
}

void* alloc(usize length)
{
    pmm_lock.lock();
    used_memory += length;
    if (used_memory >= available_memory) {
        Serial::serial_printf("Too much physical memory used %d/%d", used_memory, available_memory);
        return NULL;
    }

    usize v = find_free(&memory, length);

    if (v == 0) {
        Serial::serial_printf("Can't allocate block count %d", length);
        pmm_lock.unlock();
        return NULL;
    }
    if (!set_used(&memory, v, length)) {
        Serial::serial_printf("Can't set used block count %d", length);
        pmm_lock.unlock();
        return NULL;
    }

    pmm_lock.unlock();

    return (void*)(v * PAGE_SIZE);
}

void* allocate_zero(usize length)
{
    void* data = alloc(length);

    if (!data) {
        return NULL;
    }

    pmm_lock.lock();

    u64* pages = get_mem_addr<u64*>(data);

    for (u64 i = 0; i < (length * PAGE_SIZE) / sizeof(u64); i++) {
        pages[i] = 0;
    }
    pmm_lock.unlock();
    return data;
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
            if (set_free(&memory, it.base / PAGE_SIZE, it.length / PAGE_SIZE)) {
                available_memory = it.length / PAGE_SIZE;
            } else {
                return false;
            }
        }
    }

    Serial::serial_printf("PMM Initialized");

    return true;
}
}