#include <cpu/cpu.h>
#include <cpu/pic.h>
#include <cpu/serial.h>

namespace PIC {

void init_pic()
{
    Serial::serial_printf("Initializing PIC");

    CPU::write_to_port_u8(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    pic_wait();
    CPU::write_to_port_u8(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    pic_wait();

    CPU::write_to_port_u8(PIC1_DATA, PIC1_OFFSET);
    pic_wait();
    CPU::write_to_port_u8(PIC2_DATA, PIC2_OFFSET);
    pic_wait();

    CPU::write_to_port_u8(PIC1_DATA, 0x04);
    pic_wait();
    CPU::write_to_port_u8(PIC2_DATA, 0x02);
    pic_wait();

    CPU::write_to_port_u8(PIC1_DATA, 1);
    pic_wait();
    CPU::write_to_port_u8(PIC2_DATA, 1);
    pic_wait();

    CPU::write_to_port_u8(PIC1_DATA, 0xff); // mask all for apic
    pic_wait();
    CPU::write_to_port_u8(PIC2_DATA, 0xff);

    Serial::serial_printf("PIC Initialized");
}

void pic_ack(int intno)
{
    if (intno >= 40) {
        CPU::write_to_port_u8(PIC2_OFFSET, 0x20);
    }

    CPU::write_to_port_u8(PIC1_OFFSET, 0x20);
}
}
