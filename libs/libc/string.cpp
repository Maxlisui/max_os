#include <string.h>
#include <types.h>

void* memset(void* data, u8 value, usize lenght)
{
    u8* d = (u8*)data;
    for (size_t i = 0; i < lenght; i++) {
        d[i] = value;
    }
    return data;
}
