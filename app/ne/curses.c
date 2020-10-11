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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "curses.h"

#define NUL '\0'

#define TRANSMIT_MAX (4096)

static chtype *buf_front;
static chtype *buf_back;
static chtype *buf_transmit;
static int cursor_x;
static int cursor_y;
static int cursor_prev_x;
static int cursor_prev_y;
static int attr;
static int attr_prev;
static int pos_transmit;
static bool dirty;

extern int rawon(void);
extern int rawoff(void);

static int _puts(char *);
static int _send_str(const char *);
static int _send_char(const char);
static int _flush(void);
static void _release(void);


int getch(void)
{
	unsigned char buf;
	return (read(STDIN_FILENO, &buf, sizeof(buf) == sizeof(buf)) ?
			((int) buf) : ERR);
}

int move(int y, int x)
{
	cursor_x = x;
	cursor_y = y;
	return OK;
}

int addch(chtype ch)
{
	//TODO save min {x, y} and max {x, y}
	if (ch == '\n') {
		unsigned int offset = cursor_y * COLS;
		for (unsigned int i = cursor_x; i < COLS; i++)
			buf_back[offset + i] = NUL;

		if (cursor_y != LINES - 1) {
			cursor_x = 0;
			cursor_y++;
		}
	} else if (ch == '\t') {
		unsigned int len = 8 - (cursor_x & 7);
		if (COLS - cursor_x < len)
			len = COLS - cursor_x;

		unsigned int offset = cursor_y * COLS;
		for (; len; len--)
			buf_back[offset + cursor_x++] = ' ';

		if (cursor_x == COLS) {
			if (cursor_y != LINES - 1) {
				cursor_x = 0;
				cursor_y++;
			}
		}
	} else {
		buf_back[cursor_y * COLS + cursor_x] = ch & 0xff;
		if (cursor_x == COLS - 1) {
			if (cursor_y != LINES - 1) {
				cursor_x = 0;
				cursor_y++;
			}
		} else
			cursor_x++;
	}

	dirty = true;
	return OK;
}

int mvaddstr(int y, int x, char *str)
{
	int result = move(y, x);
	return (result ? result : _puts(str));
}

int attron(chtype ch)
{
	return OK;
	/*
	//TODO support multiple attribute
	switch (ch) {
	case A_REVERSE: {
			char buf[16];
			sprintf(buf, "\x1b[%dm", ch);
			return _puts(buf);
		}
	default:
		return ERR;
	}
	*/
}

int attroff(chtype ch)
{
	return OK;
	/*
	//TODO support multiple attribute
	return _puts("\x1b[0m");
	*/
}

int clear(void)
{
	size_t size = COLS * LINES;
	for (unsigned int i = 0; i < size; i++)
		buf_back[i] = NUL;

	dirty = true;
	return OK;
}

int clrtobot(void)
{
	size_t size = COLS * LINES;
	for (unsigned int i = cursor_y * COLS + cursor_x; i < size; i++)
		buf_back[i] = NUL;

	dirty = true;
	return OK;
}

static int _puts(char *str)
{
	for (char *p = str; *p; p++)
		addch((chtype) (*p));

	return OK;
}

//TODO optimize csi
int refresh(void)
{
	pos_transmit = 0;

	char buf[16];
	if (dirty) {
		unsigned int lastx = 0xffffffff;
		unsigned int lasty = 0xffffffff;
		unsigned int x = 0;
		unsigned int y = 0;
		unsigned int i = 0;
		size_t size = COLS * LINES;

		do {
			for (; i < size; i++) {
				if (buf_back[i] != buf_front[i])
					break;

				if (!buf_back[i]) {
					x = 0;
					y++;
					i = y * COLS;
					break;
				}

				if (++x == COLS) {
					x = 0;
					y++;
				}
			}

			if (i >= size)
				break;

			//TODO put characters if gap is smaller than 8
			//TODO use LF or move relatively
			if ((x != lastx)
					|| (y != lasty)) {
				sprintf(buf, "\x1b[%d;%dH", y + 1, x + 1);
				if (_send_str(buf) != OK)
					return ERR;
			}

			for (; i < size; i++) {
				if (buf_back[i] == buf_front[i])
					break;

				chtype ch = buf_back[i];
				if (_send_char(ch & 0xff) != OK)
					return ERR;

				buf_front[i] = ch;

				//TODO use clear to eol
				//TODO put spaces if gap is smaller than 3
				//TODO convert speces to tab or clear from head
				if (++x == COLS) {
					x = 0;
					y++;
				}
			}

			lastx = x;
			lasty = y;
		} while (i < size);

		dirty = false;
	}

	//TODO use LF or move relatively
	sprintf(buf, "\x1b[%d;%dH", cursor_y + 1, cursor_x + 1);
	if (_send_str(buf) != OK)
		return ERR;

	_flush();
	cursor_prev_x = cursor_x;
	cursor_prev_y = cursor_y;
	return OK;
}

static int _send_str(const char *str)
{
	for (char *p = (char *) str; *p; p++)
		if (_send_char(*p) != OK)
			return ERR;

	return OK;
}

static int _send_char(const char ch)
{
	if (pos_transmit == TRANSMIT_MAX)
		if (_flush() != OK)
			return ERR;

	buf_transmit[pos_transmit++] = ch;
	return OK;
}

static int _flush(void)
{
	if (!pos_transmit)
		return OK;

	int result = (write(STDOUT_FILENO, buf_transmit, pos_transmit)
					== pos_transmit) ?
			OK : ERR;
	pos_transmit = 0;
	return result;
}

int initscr(void)
{
	do {
		size_t size = COLS * LINES;
		buf_front = (char *) malloc(sizeof(chtype) * size);
		if (!buf_front)
			break;

		buf_back = (char *) malloc(sizeof(chtype) * size);
		if (!buf_back)
			break;

		buf_transmit = (char *) malloc(TRANSMIT_MAX);
		if (!buf_transmit)
			break;

		for (unsigned int i = 0; i < size; i++) {
			buf_front[i] = NUL;
			buf_back[i] = NUL;
		}

		cursor_x = cursor_y = cursor_prev_x = cursor_prev_y = 0;
		attr_prev = attr = 0;
		dirty = false;

		pos_transmit = 0;
		if (_send_str("\x1b[0m\x1b[2J\x1b[1;1H"))
			return ERR;

		return _flush();
	} while (false);

	_release();
	return ERR;
}

static void _release(void)
{
	if (buf_front)
		free(buf_front);

	if (buf_back)
		free(buf_back);

	if (buf_transmit)
		free(buf_transmit);

	buf_front = buf_back = buf_transmit = NULL;
}

int endwin(void)
{
	//TODO reset mode, attr, color
	int result = ((clear() | move(0, 0) | refresh() | rawoff()) ?
			ERR : OK);
	_release();
	return result;
}

int raw(void)
{
	return (rawon() ? ERR : OK);
}
