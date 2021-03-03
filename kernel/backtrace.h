#pragma once

#include <arch.h>

namespace Backtrace {

struct backtrace {
    static const int backtrace_max_entry_count = 32;
    backtrace_entry_type entries[backtrace_max_entry_count];
};

void add_entry(backtrace* trace, const backtrace_entry_type added_entry);
void dump_backtrace(backtrace* trace);

}
