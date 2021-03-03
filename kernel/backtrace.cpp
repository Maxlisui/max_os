#include <backtrace.h>
#include <cpu/serial.h>

namespace Backtrace {

void add_entry(backtrace* trace, backtrace_entry_type added_entry)
{
    int max = backtrace::backtrace_max_entry_count - 1;
    if (trace->entries[max] != added_entry) {
        for (int i = 0; i < max - 1; i++) {
            trace->entries[i] = trace->entries[i + 1];
        }
        trace->entries[max] = added_entry;
    }
}

void dump_backtrace(backtrace* trace)
{
    for (int i = backtrace::backtrace_max_entry_count - 1; i >= 0; i--) {
        Serial::serial_printf("[Backtrace] ID %i = %i", i, trace->entries[i]);
    }
}

}