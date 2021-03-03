#pragma once

#include <types.h>

#define PAGE_SIZE 4096
#define PAGE_ALIGN __attribute__((aligned(PAGE_SIZE)));
typedef u64 backtrace_entry_type;