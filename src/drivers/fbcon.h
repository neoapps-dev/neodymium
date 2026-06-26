#ifndef FBCON_H
#define FBCON_H
void fbcon_init(void);
void fbcon_resize(void);
void fbcon_putchar(char c);
void fbcon_redraw(void);
void fbcon_set_visible(int v);
int fbcon_get_visible(void);
void fbcon_cursor_up(void);
void fbcon_cursor_down(void);
void fbcon_cursor_left(void);
void fbcon_cursor_right(void);
void fbcon_tick(void);
#endif
