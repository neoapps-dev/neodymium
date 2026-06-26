#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H
struct multiboot_info;
int fb_init(struct multiboot_info *mbd);
void fb_putpixel(int x, int y, unsigned int color);
void fb_fillrect(int x, int y, int w, int h, unsigned int color);
void fb_drawchar(int x, int y, char c, unsigned int fg, unsigned int bg);
void fb_drawstring(int x, int y, const char *s, unsigned int fg, unsigned int bg);
void fb_clear(unsigned int color);
unsigned int fb_rgb(unsigned char r, unsigned char g, unsigned char b);
unsigned int fb_get_width(void);
unsigned int fb_get_height(void);
unsigned int fb_get_bpp(void);
int fb_is_enabled(void);
#endif
