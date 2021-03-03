#include "serial.h"
#include "cpu.h"
#include <stdarg.h>

static const char CONVERSION_TABLE[] = "0123456789abcdef";

namespace Serial {

inline u8 line_sts(SerialPort* port)
{
    return CPU::read_from_port_u8(port->line_sts);
    return 0;
}

void init_serial_out(u16 base)
{
    DEBUG_OUT_PORT.data = base;
    DEBUG_OUT_PORT.init_en = base + 1;
    DEBUG_OUT_PORT.fifo_ctrl = base + 2;
    DEBUG_OUT_PORT.line_ctrl = base + 3;
    DEBUG_OUT_PORT.modem_ctrl = base + 4;
    DEBUG_OUT_PORT.line_sts = base + 5;

    CPU::write_to_port_u8(DEBUG_OUT_PORT.init_en, 0x00);
    CPU::write_to_port_u8(DEBUG_OUT_PORT.line_ctrl, 0x80);
    CPU::write_to_port_u8(DEBUG_OUT_PORT.data, 0x03);
    CPU::write_to_port_u8(DEBUG_OUT_PORT.init_en, 0x00);
    CPU::write_to_port_u8(DEBUG_OUT_PORT.line_ctrl, 0x03);
    CPU::write_to_port_u8(DEBUG_OUT_PORT.fifo_ctrl, 0xC7);
    CPU::write_to_port_u8(DEBUG_OUT_PORT.modem_ctrl, 0x0B);
    CPU::write_to_port_u8(DEBUG_OUT_PORT.init_en, 0x01);
}

void send(SerialPort* port, u8 data)
{
    if (data == (8 | 0x7F)) {
        CPU_SLEEP(line_sts(port) == (u8)LineStsFlags::OUTPUT_EMPTY)
        CPU::write_to_port_u8(port->data, (u8)8);
        CPU_SLEEP(line_sts(port) == (u8)LineStsFlags::OUTPUT_EMPTY)
        CPU::write_to_port_u8(port->data, (u8)' ');
        CPU_SLEEP(line_sts(port) == (u8)LineStsFlags::OUTPUT_EMPTY)
        CPU::write_to_port_u8(port->data, (u8)8);
    } else {
        CPU_SLEEP(line_sts(port) == (u8)LineStsFlags::OUTPUT_EMPTY)
        CPU::write_to_port_u8(port->data, data);
    }
}

void print(const u8* msg)
{
    for (size_t i = 0; msg[i]; i++) {
        send(&DEBUG_OUT_PORT, msg[i]);
    }
}

static void printhex(usize num)
{
    int i;
    u8 buf[17];

    if (!num) {
        print((u8*)"0x0");
        return;
    }

    buf[16] = 0;

    for (i = 15; num; i--) {
        buf[i] = CONVERSION_TABLE[num % 16];
        num /= 16;
    }

    i++;
    print((u8*)"0x");
    print(&buf[i]);
}

static void printdec(usize num)
{
    int i;
    u8 buf[21] = { 0 };

    if (!num) {
        send(&DEBUG_OUT_PORT, (u8)' ');
        return;
    }

    for (i = 19; num; i--) {
        buf[i] = (num % 10) + 0x30;
        num = num / 10;
    }

    i++;
    print(buf + i);
}

void serial_printf(const char* format, ...)
{
    va_list argp;
    va_start(argp, format);

    while (*format != '\0') {
        if (*format == '%') {
            format++;
            if (*format == 'x') {
                printhex(va_arg(argp, usize));
            } else if (*format == 'd') {
                printdec(va_arg(argp, usize));
            } else if (*format == 's') {
                print(va_arg(argp, u8*));
            }
        } else {
            send(&DEBUG_OUT_PORT, (u8)*format);
        }
        format++;
    }

    send(&DEBUG_OUT_PORT, (u8)'\n');
    va_end(argp);
}
}