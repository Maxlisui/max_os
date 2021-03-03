#include <cpu/smp.h>
#include <cpu/sse.h>

namespace SMP {

extern "C" void asm_sse_save(u64 addr);
extern "C" void asm_sse_load(u64 addr);
cpu procData[MAX_CPU_COUNT];

void load_sse(cpu* cpu, u64* data)
{
    SSE::load_sse_context(data);
}

void save_sse(cpu* cpu, u64* data)
{
    SSE::save_sse_context(data);
}

}