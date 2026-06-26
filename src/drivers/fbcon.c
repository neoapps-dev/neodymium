#include "fbcon.h"
#include "framebuffer.h"
#include "../kernel/idt.h"
#define CHAR_W 10
#define CHAR_H 10
#define MAX_COLS 200
#define MAX_ROWS 200
#define TAB 4
#define BLINK_RATE 125
static char buf[MAX_ROWS][MAX_COLS];
static int COLS;
static int ROWS;
static int cx;
static int cy;
static int visible;
static int cursor_shown;
static int cursor_blink;
static unsigned int cursor_last_toggle;
static int cursor_x;
static int cursor_y;
static unsigned int fg(void) { return fb_rgb(200, 200, 200); }
static unsigned int bg(void) { return fb_rgb(0, 0, 0); }
static void cursor_show(void) {
    if (!visible || !fb_is_enabled()) return;
    fb_fillrect(cx * CHAR_W, cy * CHAR_H, 8, 8, fg());
    cursor_shown = 1;
    cursor_x = cx;
    cursor_y = cy;
}
static void cursor_hide(void) {
    if (!visible || !fb_is_enabled()) return;
    if (cursor_shown) {
        fb_drawchar(cursor_x * CHAR_W, cursor_y * CHAR_H, buf[cursor_y][cursor_x], fg(), bg());
        cursor_shown = 0;
    }
}
static void scroll(void) {
    for (int y = 1; y < ROWS; y++)
        for (int x = 0; x < COLS; x++)
            buf[y - 1][x] = buf[y][x];
    for (int x = 0; x < COLS; x++)
        buf[ROWS - 1][x] = ' ';
    cy--;
}
void fbcon_init(void) {
    COLS = 80;
    ROWS = 25;
    cx = 0; cy = 0; visible = 0;
    cursor_shown = 0; cursor_blink = 0; cursor_last_toggle = 0;
    for (int y = 0; y < MAX_ROWS; y++) for (int x = 0; x < MAX_COLS; x++) buf[y][x] = ' ';
}
void fbcon_resize(void) {
    unsigned int fw = fb_get_width();
    unsigned int fh = fb_get_height();
    if (fw < 1 || fh < 1) return;
    unsigned int nc = fw / CHAR_W;
    unsigned int nr = fh / CHAR_H;
    if (nc < 1) nc = 1;
    if (nr < 1) nr = 1;
    if (nc > MAX_COLS) nc = MAX_COLS;
    if (nr > MAX_ROWS) nr = MAX_ROWS;
    COLS = nc;
    ROWS = nr;
    if (cx >= COLS) cx = COLS - 1;
    if (cy >= ROWS) cy = ROWS - 1;
    cursor_shown = 0;
}
void fbcon_putchar(char c) {
    if (!COLS || !ROWS) return;
    cursor_hide();
    if (c == '\n') {
        cx = 0; cy++;
    } else if (c == '\r') {
        cx = 0;
    } else if (c == '\b') {
        if (cx > 0) cx--;
        else if (cy > 0) { cy--; cx = COLS - 1; }
        buf[cy][cx] = ' ';
        if (visible) fb_drawchar(cx * CHAR_W, cy * CHAR_H, ' ', fg(), bg());
    } else if (c == '\t') {
        cx = (cx + TAB) & ~(TAB - 1);
    } else {
        buf[cy][cx] = c;
        if (visible) fb_drawchar(cx * CHAR_W, cy * CHAR_H, c, fg(), bg());
        cx++;
    }
    if (cx >= COLS) { cx = 0; cy++; }
    if (cy >= ROWS) { scroll(); if (visible) fbcon_redraw(); }
    cursor_blink = 1;
    cursor_last_toggle = get_tick();
    cursor_show();
}
void fbcon_redraw(void) {
    if (!fb_is_enabled()) return;
    unsigned int f = fg();
    unsigned int b = bg();
    for (int y = 0; y < ROWS; y++) for (int x = 0; x < COLS; x++) fb_drawchar(x * CHAR_W, y * CHAR_H, buf[y][x], f, b);
    cursor_shown = 0;
    if (cursor_blink) cursor_show();
}
void fbcon_set_visible(int v) {
    visible = v;
    if (v) {
        cursor_blink = 1;
        cursor_last_toggle = get_tick();
    }
}
int fbcon_get_visible(void) { return visible; }
void fbcon_tick(void) {
    if (!visible) {
        cursor_hide();
        cursor_blink = 0;
        return;
    }
    unsigned int now = get_tick();
    if (now - cursor_last_toggle >= BLINK_RATE) {
        cursor_last_toggle = now;
        cursor_blink = !cursor_blink;
        if (cursor_blink)
            cursor_show();
        else
            cursor_hide();
    }
}
