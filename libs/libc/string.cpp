#include <string.h>
#include <types.h>

void* memset(void* data, u8 value, usize length)
{
    u8* d = (u8*)data;
    for (size_t i = 0; i < length; i++) {
        d[i] = value;
    }
    return data;
}

void memzero(void const* s, const usize length)
{
    for (usize i = 0; i < length; i++) {
        ((u8*)s)[i] = 0;
    }
}