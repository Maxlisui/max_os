#include <cpu/serial.h>
#include <memory/vmm.h>

namespace VMM {

void init_vmm(stivale2_struct_tag_memmap* mmap_tag)
{
    Serial::serial_printf("Initializing VMM");

    Serial::serial_printf("VMM Initialized");
}

}
