#include "panic.h"
#include "printf.h"
#include "asm/cpu.h"
#include "../drivers/framebuffer.h"
#include <stdarg.h>
static void hex_str(char *buf, unsigned int v) {
    buf[0] = '0'; buf[1] = 'x';
    int i = 2;
    char tmp[10]; int ti = 0;
    if (v == 0) tmp[ti++] = '0';
    while (v > 0) { tmp[ti++] = "0123456789abcdef"[v & 0xF]; v >>= 4; }
    while (ti > 0) buf[i++] = tmp[--ti];
    buf[i] = 0;
}

static void draw_evil_af_scary_screen(const char *msg, struct regs *r, unsigned int *frames, int nframes) {
    if (!fb_is_enabled()) return;
    unsigned int h = fb_get_height();
    unsigned int white = fb_rgb(255, 255, 255);
    unsigned int red = fb_rgb(180, 0, 0);
    fb_clear(red);
    int y = 8;
    fb_drawstring(8, y, "[neodymium]", white, red); y += 20;
    fb_drawstring(8, y, "KERNEL PANIC", white, red); y += 20;
    fb_drawstring(8, y, msg, white, red); y += 20;
    if (r) {
        y += 4;
        char line[64];
        fb_drawstring(8, y, "eax:", white, red);
        hex_str(line, r->eax); fb_drawstring(8 + 40, y, line, white, red);
        fb_drawstring(8 + 100, y, "ebx:", white, red);
        hex_str(line, r->ebx); fb_drawstring(8 + 140, y, line, white, red);
        fb_drawstring(8 + 200, y, "ecx:", white, red);
        hex_str(line, r->ecx); fb_drawstring(8 + 240, y, line, white, red);
        fb_drawstring(8 + 300, y, "edx:", white, red);
        hex_str(line, r->edx); fb_drawstring(8 + 340, y, line, white, red);
        y += 16;
        fb_drawstring(8, y, "esi:", white, red);
        hex_str(line, r->esi); fb_drawstring(8 + 40, y, line, white, red);
        fb_drawstring(8 + 100, y, "edi:", white, red);
        hex_str(line, r->edi); fb_drawstring(8 + 140, y, line, white, red);
        fb_drawstring(8 + 200, y, "ebp:", white, red);
        hex_str(line, r->ebp); fb_drawstring(8 + 240, y, line, white, red);
        fb_drawstring(8 + 300, y, "esp:", white, red);
        hex_str(line, r->esp); fb_drawstring(8 + 340, y, line, white, red);
        y += 16;
        fb_drawstring(8, y, "eip:", white, red);
        hex_str(line, r->eip); fb_drawstring(8 + 40, y, line, white, red);
        fb_drawstring(8 + 100, y, "cs:", white, red);
        hex_str(line, r->cs); fb_drawstring(8 + 140, y, line, white, red);
        fb_drawstring(8 + 200, y, "eflags:", white, red);
        hex_str(line, r->eflags); fb_drawstring(8 + 260, y, line, white, red);
        fb_drawstring(8 + 360, y, "err:", white, red);
        hex_str(line, r->err_code); fb_drawstring(8 + 400, y, line, white, red);
        y += 16;
        y += 8;
    }

    for (int i = 0; i < nframes && y < (int)h - 20; i++) {
        char buf[32];
        hex_str(buf, frames[i]);
        fb_drawstring(8, y, buf, white, red);
        y += 16;
    }
}

static void fmt_msg(char *msg, const char *fmt, va_list args) {
    int mi = 0;
    for (const char *p = fmt; *p && mi < 250; p++) {
        if (*p == '%' && *(p + 1)) {
            p++;
            switch (*p) {
                case 's': {
                    const char *s = va_arg(args, const char *);
                    while (*s && mi < 250) msg[mi++] = *s++;
                    break;
                }
                case 'd': {
                    int n = va_arg(args, int);
                    if (n < 0) { msg[mi++] = '-'; n = -n; }
                    char nb[12]; int ni = 0;
                    if (n == 0) nb[ni++] = '0';
                    while (n > 0) { nb[ni++] = '0' + n % 10; n /= 10; }
                    while (ni > 0 && mi < 250) msg[mi++] = nb[--ni];
                    break;
                }
                case 'u': {
                    unsigned int n = va_arg(args, unsigned int);
                    char nb[12]; int ni = 0;
                    if (n == 0) nb[ni++] = '0';
                    while (n > 0) { nb[ni++] = '0' + n % 10; n /= 10; }
                    while (ni > 0 && mi < 250) msg[mi++] = nb[--ni];
                    break;
                }
                case 'x': {
                    unsigned int n = va_arg(args, unsigned int);
                    msg[mi++] = '0'; msg[mi++] = 'x';
                    char nb[10]; int ni = 0;
                    if (n == 0) nb[ni++] = '0';
                    while (n > 0) { nb[ni++] = "0123456789abcdef"[n & 0xF]; n >>= 4; }
                    while (ni > 0 && mi < 250) msg[mi++] = nb[--ni];
                    break;
                }
                case 'c':
                    msg[mi++] = (char)va_arg(args, int);
                    break;
                default:
                    if (mi < 249) { msg[mi++] = *(p - 1); msg[mi++] = *p; }
                    break;
            }
        } else {
            msg[mi++] = *p;
        }
    }
    msg[mi] = 0;
}

static void walk_frames(unsigned int *frames, int *nframes, unsigned int ebp) {
    *nframes = 0;
    unsigned int *fp = (unsigned int *)ebp;
    while (fp && *nframes < 16) {
        frames[*nframes] = fp[1];
        (*nframes)++;
        fp = (unsigned int *)fp[0];
    }
}

void panic(const char *fmt, ...) {
    cli();
    va_list args;
    va_start(args, fmt);
    char msg[256];
    fmt_msg(msg, fmt, args);
    va_end(args);
    struct regs r;
    __asm__ volatile(
        "mov %%eax, %0\n"
        "mov %%ebx, %1\n"
        "mov %%ecx, %2\n"
        "mov %%edx, %3\n"
        "mov %%esi, %4\n"
        "mov %%edi, %5\n"
        "mov %%ebp, %6\n"
        "mov %%esp, %7\n"
        : "=m"(r.eax), "=m"(r.ebx), "=m"(r.ecx), "=m"(r.edx),
          "=m"(r.esi), "=m"(r.edi), "=m"(r.ebp), "=m"(r.esp)
    );
    r.eip = (unsigned int)__builtin_return_address(0);
    r.cs = 0x8;
    r.eflags = 0;
    r.err_code = 0;
    unsigned int frames[16]; int nframes;
    walk_frames(frames, &nframes, r.ebp);
    draw_evil_af_scary_screen(msg, &r, frames, nframes);
    printf("\n[neodymium]\nKERNEL PANIC\n%s\n", msg);
    printf("eax=0x%x ebx=0x%x ecx=0x%x edx=0x%x\n", r.eax, r.ebx, r.ecx, r.edx);
    printf("esi=0x%x edi=0x%x ebp=0x%x esp=0x%x\n", r.esi, r.edi, r.ebp, r.esp);
    printf("eip=0x%x cs=0x%x eflags=0x%x err=%u\n", r.eip, r.cs, r.eflags, r.err_code);
    printf("backtrace:\n");
    for (int i = 0; i < nframes; i++) printf("  [0x%x]\n", frames[i]);
    for (;;) hlt();
}

void panic_regs(const char *fmt, struct regs *r, ...) {
    cli();
    va_list args;
    va_start(args, r);
    char msg[256];
    fmt_msg(msg, fmt, args);
    va_end(args);
    unsigned int frames[16]; int nframes;
    walk_frames(frames, &nframes, r->ebp);
    draw_evil_af_scary_screen(msg, r, frames, nframes);
    printf("\n[neodymium]\nKERNEL PANIC\n%s\n", msg);
    printf("eax=0x%x ebx=0x%x ecx=0x%x edx=0x%x\n", r->eax, r->ebx, r->ecx, r->edx);
    printf("esi=0x%x edi=0x%x ebp=0x%x esp=0x%x\n", r->esi, r->edi, r->ebp, r->esp);
    printf("eip=0x%x cs=0x%x eflags=0x%x err=%u\n", r->eip, r->cs, r->eflags, r->err_code);
    printf("backtrace:\n");
    for (int i = 0; i < nframes; i++) printf("  [0x%x]\n", frames[i]);
    for (;;) hlt();
}
