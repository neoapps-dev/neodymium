#ifndef CMOS_H
#define CMOS_H
#include "asm/io.h"
static inline unsigned char cmos_read(unsigned char reg)
{
    outb(0x70, reg);
    return inb(0x71);
}

static inline unsigned int cmos_get_second(void)
{
    unsigned int b = cmos_read(0x00);
    return (b & 0xF) + ((b >> 4) * 10);
}

static inline unsigned int cmos_get_minute(void)
{
    unsigned int b = cmos_read(0x02);
    return (b & 0xF) + ((b >> 4) * 10);
}

static inline unsigned int cmos_get_hour(void)
{
    unsigned int b = cmos_read(0x04);
    return (b & 0xF) + ((b >> 4) * 10);
}

static inline unsigned int cmos_get_day(void)
{
    unsigned int b = cmos_read(0x07);
    return (b & 0xF) + ((b >> 4) * 10);
}

static inline unsigned int cmos_get_month(void)
{
    unsigned int b = cmos_read(0x08);
    return (b & 0xF) + ((b >> 4) * 10);
}

static inline unsigned int cmos_get_year(void)
{
    unsigned int b = cmos_read(0x09);
    return (b & 0xF) + ((b >> 4) * 10);
}

static inline unsigned int cmos_get_century(void)
{
    unsigned int b = cmos_read(0x32);
    return (b & 0xF) + ((b >> 4) * 10);
}
#endif
