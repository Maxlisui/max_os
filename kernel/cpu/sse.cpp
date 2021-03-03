#include <cpu/serial.h>
#include <cpu/sse.h>

extern "C" void sse_init(void);
extern "C" void enable_xsave(void);
extern "C" void avx_init(void);

namespace SSE {

void init_xsave()
{
    if (has_xsave()) {
        enable_xsave();
    }
}

void init_avx()
{
    if (has_avx()) {
        avx_init();
    }
}

void init_sse()
{
    Serial::serial_printf("Initializing SIMD Support");
    Serial::serial_printf(" -Initializing SSE");
    sse_init();

    Serial::serial_printf(" -Initializing XSAVE");
    init_xsave();

    Serial::serial_printf(" -Initializing AVX");
    init_xsave();

    Serial::serial_printf(" -Initializing Floating Point Unit");
    asm("fninit");
}

bool has_xsave()
{
    uint32_t ecx = 0;
    asm volatile("cpuid"
                 : "=c"(ecx)
                 : "a"(1), "c"(0));
    return ecx & (((u32)1) << 26);
}

bool has_avx()
{
    uint32_t ecx = 0;
    asm volatile("cpuid"
                 : "=c"(ecx)
                 : "a"(1), "c"(0));
    return ecx & (1 << 28);
}

}
