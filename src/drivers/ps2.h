#ifndef PS2_H
#define PS2_H
#define KEY_UP       256
#define KEY_DOWN     257
#define KEY_LEFT     258
#define KEY_RIGHT    259
#define KEY_HOME     260
#define KEY_END      261
#define KEY_PGUP     262
#define KEY_PGDN     263
#define KEY_INS      264
#define KEY_DEL      265
#define KEY_F1       266
#define KEY_F2       267
#define KEY_F3       268
#define KEY_F4       269
#define KEY_F5       270
#define KEY_F6       271
#define KEY_F7       272
#define KEY_F8       273
#define KEY_F9       274
#define KEY_F10      275
#define KEY_F11      276
#define KEY_F12      277
void ps2_init(void);
int ps2_getchar(void);
int ps2_poll(void);
int ps2_is_ctrl(void);
int ps2_is_shift(void);
int ps2_is_alt(void);
#endif
