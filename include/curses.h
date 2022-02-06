#ifndef _CURSES_H_
#define _CURSES_H_ 1
/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/
#include <stdbool.h>

#define OK (0)
#define ERR (-1)

#ifdef USE_VESA
#define COLS (85)
#define LINES (32)
#else
#define COLS (80)
#define LINES (25)
#endif

#define stdscr NULL

typedef enum {
	A_REVERSE = 7
} curses_attr_t;

typedef char chtype;
typedef void WINDOW;

extern int getch(void);

extern int move(int, int);

extern int addch(chtype);
extern int mvaddstr(int, int, char *);

extern int clear(void);
extern int clrtobot(void);

extern int attron(chtype);
extern int attroff(chtype);

extern int refresh(void);

extern int initscr(void);
extern int endwin(void);
extern int raw(void);

static inline int noecho(void)
{
	return OK;
}

static inline int idlok(WINDOW *w, bool f)
{
	return OK;
}

#endif
