#include <cpu/cpu.h>
#include <cpu/rtc.h>
#include <cpu/serial.h>

namespace RTC {

bool is_bcd = false;
bool is_12_format = false;

bool rtc_is_updating()
{
    CPU::write_to_port_u8(0x70, 0x0A);
    return CPU::read_from_port_u8(0x71) & 0x80;
}

u8 bcd_to_binary(u64 value)
{
    if (is_bcd) {
        return value;
    }
    return (value & 0x0F) + ((value >> 4) * 10);
}

u8 read(u8 reg)
{
    while (rtc_is_updating())
        ;

    CPU::write_to_port_u8(0x70, reg);
    return CPU::read_from_port_u8(0x71);
}

u64 get_sec()
{
    return bcd_to_binary(read(0));
}

u64 get_min()
{
    return bcd_to_binary(read(0x2));
}

u64 get_hour()
{
    u64 value = bcd_to_binary(read(0x4) & 0x7F);
    bool is_pm = value & 0x80;

    if (is_12_format) {
        value %= 12;
        if (is_pm) {
            value += 12;
        }
    }
    return value;
}

u64 get_day()
{
    return bcd_to_binary(read(0x7));
}

u64 get_month()
{
    return bcd_to_binary(read(0x8));
}

u64 get_year()
{
    return bcd_to_binary(read(0x9)) + 2000;
}

u64 get_total_sec()
{
    u64 result = 0;
    result += (get_year() - 1) * 365 * 24 * 60 * 60;
    result += (get_month() - 1) * 12 * 24 * 60 * 60;
    result += (get_day() - 1) * 24 * 60 * 60;
    result += (get_hour() - 1) * 60 * 60;
    result += (get_min() - 1) * 60;
    result += (get_sec());
    return result;
}

bool is_leap_year(u64 year)
{
    return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

u64 days_in_year(u64 year)
{
    return 365 + is_leap_year(year);
}

u64 years_to_days_since_epoch(u8 year)
{
    u64 days = 0;
    for (u64 current_year = 1970; current_year < year; ++current_year)
        days += days_in_year(current_year);
    for (u64 current_year = year; current_year < 1970; ++current_year)
        days -= days_in_year(current_year);
    return days;
}

u64 day_of_year(u64 year, u64 month, u64 day)
{
    static const u64 seek_table[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
    int day_of_year = seek_table[month - 1] + day - 1;

    if (is_leap_year(year) && month >= 3)
        day_of_year++;

    return day_of_year;
}

time_t now()
{
    u8 year = get_year();
    u8 month = get_month();
    u8 day = get_day();
    u8 hour = get_hour();
    u8 min = get_min();
    u8 sec = get_sec();

    time_t days_since_epoch = years_to_days_since_epoch(year) + day_of_year(year, month, day);

    return ((days_since_epoch * 24 + hour) * 60 + min) * 60 + sec;
}

void init_rtc()
{
    Serial::serial_printf("Initializing RTC");

    u8 status = read(0x0b);

    if (!(status & 0x04)) {
        is_bcd = true;
        Serial::serial_printf("RTC detected BCD Format");
    }

    if (!(status & 0x02)) {
        is_12_format = true;
        Serial::serial_printf("RTC detected 12 Format");
    }

    Serial::serial_printf("Total Seconds: %d", get_total_sec());
    Serial::serial_printf("Year: %d", get_year());
    Serial::serial_printf("Month: %d", get_month());
    Serial::serial_printf("Day: %d", get_day());
    Serial::serial_printf("Hour: %d", get_hour());
    Serial::serial_printf("Min: %d", get_min());
    Serial::serial_printf("Seconds: %d", get_sec());

    // boot_time = now();
    Serial::serial_printf("RTC Initialized");
}

}