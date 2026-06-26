#ifndef FBCON_H
#define FBCON_H
void fbcon_init(void);
void fbcon_resize(void);
void fbcon_putchar(char c);
void fbcon_redraw(void);
void fbcon_set_visible(int v);
int fbcon_get_visible(void);
void fbcon_tick(void);
#endif
