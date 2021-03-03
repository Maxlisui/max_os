#pragma once

#include <types.h>

namespace IDT {

struct idtr {
    u16 size;   // size of the IDT table
    u64 offset; // address of the IDT table
} __attribute__((packed));

}