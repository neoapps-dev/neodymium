#include "ps2.h"
#include "../kernel/idt.h"
#include "../kernel/asm/io.h"
#define PS2_DATA 0x60
#define PS2_CMD  0x64
#define PS2_STATUS 0x64
#define KEYBUF_SIZE 128
static volatile int keybuf[KEYBUF_SIZE];
static volatile unsigned int keybuf_head;
static volatile unsigned int keybuf_tail;
static int extended;
static int shift_pressed;
static int ctrl_pressed;
static int alt_pressed;
static int caps_pressed;
static const char keymap_lower[128] = {
    0,    27,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    '-',  '=',  '\b', '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
    'o',  'p',  '[',  ']',  '\n', 0,   'a', 's', 'd', 'f', 'g', 'h',
    'j',  'k',  'l',  ';',  '\'', '`', 0,   '\\', 'z', 'x', 'c', 'v',
    'b',  'n',  'm',  ',',  '.',  '/', 0,   '*', 0,   ' ', 0,   0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,   0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,   0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,   0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,   0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,   0,
    0,    0,    0,    0,    0,    0,    0,    0
};

static const char keymap_upper[128] = {
    0,    27,   '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
    '_',  '+',  '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
    'O',  'P',  '{',  '}',  '\n', 0,   'A', 'S', 'D', 'F', 'G', 'H',
    'J',  'K',  'L',  ':',  '"',  '~', 0,   '|', 'Z', 'X', 'C', 'V',
    'B',  'N',  'M',  '<',  '>',  '?',  0,   '*', 0,   ' ', 0,   0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,   0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,   0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,   0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,   0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,   0,
    0,    0,    0,    0,    0,    0,    0,    0
};

static void buf_put(int val) {
    unsigned int next = (keybuf_head + 1) % KEYBUF_SIZE;
    if (next != keybuf_tail) {
        keybuf[keybuf_head] = val;
        keybuf_head = next;
    }
}

static void keyboard_handler(struct regs *r) {
    (void)r;
    unsigned char sc = inb(PS2_DATA);
    if (sc == 0xE0) { extended = 1; return; }
    if (extended) {
        extended = 0;
        if (sc & 0x80) return;
        int val = 0;
        switch (sc) {
            case 0x48: val = KEY_UP; break;
            case 0x50: val = KEY_DOWN; break;
            case 0x4B: val = KEY_LEFT; break;
            case 0x4D: val = KEY_RIGHT; break;
            case 0x47: val = KEY_HOME; break;
            case 0x4F: val = KEY_END; break;
            case 0x49: val = KEY_PGUP; break;
            case 0x51: val = KEY_PGDN; break;
            case 0x52: val = KEY_INS; break;
            case 0x53: val = KEY_DEL; break;
        }
        if (val) buf_put(val);
        return;
    }

    switch (sc) {
        case 0x2A: case 0x36: shift_pressed = 1; return;
        case 0xAA: case 0xB6: shift_pressed = 0; return;
        case 0x1D: ctrl_pressed = 1; return;
        case 0x9D: ctrl_pressed = 0; return;
        case 0x38: alt_pressed = 1; return;
        case 0xB8: alt_pressed = 0; return;
        case 0x3A: caps_pressed = !caps_pressed; return;
        case 0xBA: return;
    }

    if (sc & 0x80) return;
    if (sc >= 0x3B && sc <= 0x44) { buf_put(KEY_F1 + (sc - 0x3B)); return; }
    if (sc == 0x57) { buf_put(KEY_F11); return; }
    if (sc == 0x58) { buf_put(KEY_F12); return; }
    int c = shift_pressed ? keymap_upper[sc] : keymap_lower[sc];
    if (!c) return;
    if (caps_pressed && ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')))
        c ^= 32;
    if (ctrl_pressed) {
        if (c >= 'a' && c <= 'z') c -= 'a' - 1;
        else if (c >= 'A' && c <= 'Z') c -= 'A' - 1;
    }
    buf_put(c);
}

void ps2_init(void) {
    irq_install_handler(1, keyboard_handler);
    while (inb(PS2_STATUS) & 0x02);
    outb(PS2_CMD, 0xAE);
    while (inb(PS2_STATUS) & 0x02);
    outb(PS2_DATA, 0xF4);
    while (inb(PS2_STATUS) & 0x01)
        inb(PS2_DATA);
    outb(0x21, inb(0x21) & ~0x02);
}

int ps2_getchar(void) {
    while (keybuf_head == keybuf_tail);
    unsigned int tail = keybuf_tail;
    int c = keybuf[tail];
    keybuf_tail = (tail + 1) % KEYBUF_SIZE;
    return c;
}

int ps2_poll(void) {
    if (keybuf_head != keybuf_tail) {
        unsigned int tail = keybuf_tail;
        int c = keybuf[tail];
        keybuf_tail = (tail + 1) % KEYBUF_SIZE;
        return c;
    }
    return -1;
}

int ps2_is_ctrl(void) { return ctrl_pressed; }
int ps2_is_shift(void) { return shift_pressed; }
int ps2_is_alt(void) { return alt_pressed; }
