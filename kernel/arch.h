#pragma once

#include <types.h>

#define PAGE_SIZE 4096
#define PAGE_ALIGN __attribute__((aligned(PAGE_SIZE)));
#define MEM_ADDR 0xffff800000000000
#define KER_ADDR 0xffffffff80000000
typedef u64 backtrace_entry_type;

template<class T = u64, class F>
inline constexpr T get_mem_addr(F addr) { return reinterpret_cast<T>((u64)(addr) + MEM_ADDR); }
template<class T = u64, class F>
inline constexpr T get_rmem_addr(F addr) { return reinterpret_cast<T>((u64)(addr)-MEM_ADDR); }
template<class T = u64, class F>
inline constexpr T get_kern_addr(F addr) { return reinterpret_cast<T>((u64)(addr) + KER_ADDR); }