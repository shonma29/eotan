#ifndef __CONSOLE_H__
#define __CONSOLE_H__	

#define MAX_HEIGHT	25
#define MAX_WIDTH	80

extern void write_cr (void);
extern void write_tab (void);
extern int init_console (void);
extern int putchar (int ch);
extern void set_cursor(int sx, int sy);
extern void get_cursor(int *gx,int *gy);

#endif /* __CONSOLE_H__ */


