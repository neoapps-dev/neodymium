#include "printf.h"
#include <stdarg.h>
#include "../drivers/serial.h"
#include "../drivers/vga.h"
static void putchar(char c) {
    serial_putchar(c);
    vga_putchar(c);
}

void printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    for (const char *p = fmt; *p; p++) {
        if (*p == '%') {
            p++;
            switch (*p) {
                case 's': {
                    const char *s = va_arg(args, const char *);
                    while (*s) putchar(*s++);
                    break;
                }
                case 'd': {
                    int n = va_arg(args, int);
                    if (n < 0) { putchar('-'); n = -n; }
                    char buf[12];
                    int i = 0;
                    if (n == 0) buf[i++] = '0';
                    while (n > 0) { buf[i++] = '0' + n % 10; n /= 10; }
                    while (i > 0) putchar(buf[--i]);
                    break;
                }
                case 'u': {
                    unsigned int n = va_arg(args, unsigned int);
                    char buf[12];
                    int i = 0;
                    if (n == 0) buf[i++] = '0';
                    while (n > 0) { buf[i++] = '0' + n % 10; n /= 10; }
                    while (i > 0) putchar(buf[--i]);
                    break;
                }
                case 'x': {
                    unsigned int n = va_arg(args, unsigned int);
                    char buf[10];
                    int i = 0;
                    if (n == 0) buf[i++] = '0';
                    while (n > 0) { buf[i++] = "0123456789abcdef"[n & 0xF]; n >>= 4; }
                    while (i > 0) putchar(buf[--i]);
                    break;
                }
                case 'c':
                    putchar((char)va_arg(args, int));
                    break;
                case '%':
                    putchar('%');
                    break;
            }
        } else {
            putchar(*p);
        }
    }
    va_end(args);
}
