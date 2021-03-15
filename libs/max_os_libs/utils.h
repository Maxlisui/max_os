#pragma once

#define KB(x) (1024ULL * x)
#define MB(x) (1024ULL * KB(x))
#define GB(x) (MB(x) * 1024ULL)

#define __break__ asm volatile("1: jmp 1b");