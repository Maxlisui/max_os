#pragma once

#include <types.h>

namespace RTC {

// static time_t boot_time;

void init_rtc();
time_t now();

}