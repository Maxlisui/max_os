#include <arch.h>
#include <cpu/serial.h>
#include <cpu/smp.h>
#include <memory/pmm.h>
#include <memory/vmm.h>
#include <utils.h>

#define PML4_GET_INDEX(addr) (addr & ((u64)0x1ff << 39)) >> 39
#define PDPT_GET_INDEX(addr) (addr & ((u64)0x1ff << 30)) >> 30
#define PAGE_DIR_GET_INDEX(addr) (addr & ((u64)0x1ff << 21)) >> 21
#define PAGE_TABLE_GET_INDEX(addr) (addr & ((u64)0x1ff << 12)) >> 12
#define FRAME_ADDR 0xfffffffffffff000
#define PAGE_TABLE_FLAGS 0x07
#define BASIC_PAGE_FLAGS 0x03

namespace VMM {

inline void set_paging_dir(u64 pd)
{
    asm volatile("mov cr3, %0" ::"r"(pd & FRAME_ADDR));
}

bool map_page(u64* table, u64 phys_addr, u64 virt_addr, u64 flags)
{
    u64 pml4_entry = PML4_GET_INDEX(virt_addr);
    u64 pdpt_entry = PDPT_GET_INDEX(virt_addr);
    u64 pd_entry = PAGE_DIR_GET_INDEX(virt_addr);
    u64 pt_entry = PAGE_TABLE_GET_INDEX(virt_addr);

    u64* pdpt = NULL;
    u64* pd = NULL;
    u64* pt = NULL;

    if (table[pml4_entry] & 0x1) {
        pdpt = get_mem_addr<u64*>(table[pml4_entry] & FRAME_ADDR);
    } else {
        void* pdpt_data = PMM::allocate_zero(1);

        if (!pdpt_data) {
            return false;
        }

        pdpt = get_mem_addr<u64*>(pdpt_data);
        table[pml4_entry] = (get_rmem_addr((u64)pdpt)) | PAGE_TABLE_FLAGS;
    }

    if (pdpt[pdpt_entry] & 0x1) {
        pd = get_mem_addr<u64*>(pdpt[pdpt_entry] & FRAME_ADDR);
    } else {
        void* pd_data = PMM::allocate_zero(1);

        if (!pd_data) {
            return false;
        }

        pd = get_mem_addr<u64*>(pd_data);
        pdpt[pdpt_entry] = get_rmem_addr(pd) | PAGE_TABLE_FLAGS;
    }

    if (pd[pd_entry] & 0x1) {
        pt = get_mem_addr<u64*>(pd[pd_entry] & FRAME_ADDR);
    } else {
        void* pt_data = PMM::allocate_zero(1);

        if (!pt_data) {
            return false;
        }

        pt = get_mem_addr<u64*>(pt_data);
        pd[pd_entry] = get_rmem_addr(pt) | PAGE_TABLE_FLAGS;
    }

    pt[pt_entry] = (u64)(phys_addr | flags);

    return true;
}

bool init_vmm(stivale2_struct_tag_memmap* mmap_tag)
{
    __break__;
    Serial::serial_printf("Initializing VMM");

    void* table_data = PMM::allocate_zero(1);

    if (!table_data) {
        return false;
    }

    u64* table = get_mem_addr<u64*>(table_data);

    SMP::get_current_cpu()->page_table = table;

    for (u64 i = 0; i < (MB(2) / PAGE_SIZE); i++) {
        u64 addr = i * PAGE_SIZE;
        if (!map_page(table, addr, addr, BASIC_PAGE_FLAGS)) {
            return false;
        }
        if (!map_page(table, addr, get_mem_addr(addr), BASIC_PAGE_FLAGS)) {
            return false;
        }
        if (!map_page(table, addr, get_kern_addr(addr), BASIC_PAGE_FLAGS)) {
            return false;
        }
    }

    set_paging_dir(get_rmem_addr(table));

    for (u64 i = 0; i < (GB(2) / PAGE_SIZE); i++) {
        u64 addr = i * PAGE_SIZE;
        if (!map_page(table, addr, get_mem_addr(addr), BASIC_PAGE_FLAGS)) {
            return false;
        }
    }

    for (u64 i = 0; i < mmap_tag->entries; i++) {
        stivale2_mmap_entry it = mmap_tag->memmap[i];

        u64 aligned_base = it.base - (it.base % PAGE_SIZE);
        u64 aligned_length = ((it.length / PAGE_SIZE) + 1) * PAGE_SIZE;

        for (u64 j = 0; j * PAGE_SIZE < aligned_length; j++) {
            u64 addr = aligned_base + j * PAGE_SIZE;

            if (addr < GB(4)) {
                continue;
            }

            if (!map_page(table, addr, get_mem_addr(addr), BASIC_PAGE_FLAGS)) {
                return false;
            }
        }
    }

    set_paging_dir(get_rmem_addr(table));

    Serial::serial_printf("VMM Initialized");
    return true;
}

}
