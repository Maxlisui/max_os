#pragma once

#include <types.h>

namespace Serial {

enum LineStsFlags {
    INPUT_FULL = 1,
    OUTPUT_EMPTY = 1 << 5,
};

struct SerialPort {
    u16 data;
    u16 init_en;
    u16 fifo_ctrl;
    u16 line_ctrl;
    u16 modem_ctrl;
    u16 line_sts;
};

static SerialPort DEBUG_OUT_PORT = SerialPort {};

void init_serial_out(u16 base);
void serial_printf(const char* format, ...);

}
