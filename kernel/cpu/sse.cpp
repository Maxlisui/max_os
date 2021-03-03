#include <cpu/serial.h>
#include <cpu/smp.h>

extern "C" void sse_init(void);
extern "C" void enable_xsave(void);
extern "C" void avx_init(void);
extern "C" void asm_sse_save(u64 addr);
extern "C" void asm_sse_load(u64 addr);
extern "C" void asm_avx_save(u64 addr);
extern "C" void asm_avx_load(u64 addr);

#define FPU_SIZE 128

u64 fpu_data[FPU_SIZE] __attribute__((aligned(64)));

static bool use_xsave = false;

namespace SSE {

bool has_xsave()
{
    u32 ecx = 0;
    asm volatile("cpuid"
                 : "=c"(ecx)
                 : "a"(1), "c"(0));
    return ecx & (((u32)1) << 26);
}

bool has_avx()
{
    u32 ecx = 0;
    asm volatile("cpuid"
                 : "=c"(ecx)
                 : "a"(1), "c"(0));
    return ecx & (1 << 28);
}

void init_xsave()
{
    if (has_xsave()) {
        enable_xsave();

        use_xsave = true;
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

void load_sse_context(u64* context)
{
    // TODO: Locking
    for (int i = 0; i < FPU_SIZE; i++) {
        fpu_data[i] = context[i];
    }

    if (use_xsave) {
        asm_avx_load((u64)fpu_data);
    } else {
        asm_sse_load(((u64)fpu_data));
    }
}

void save_sse_context(u64* context)
{
    // TODO: Locking
    if (use_xsave) {
        asm_avx_save((u64)(fpu_data));
    } else {
        asm_sse_save((u64)(fpu_data));
    }
    for (int i = 0; i < 128; i++) {
        context[i] = fpu_data[i];
    }
}

}
