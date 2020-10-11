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
#include "monitor.h"

#define ESC 0x1b
#define CSI '['
#define DELIM ';'

#define ESC_MAX_PARAMS (2)
#define ESC_MAX_COLUMNS (5)
#define ESC_BUF_SIZE (16)

typedef struct _esc_state {
	void (*func)(struct _esc_state *, const char);
	Screen *screen;
	int len;
	int num_of_params;
	int columns;
	int params[ESC_MAX_PARAMS];
	char buf[ESC_BUF_SIZE];
} esc_state_t;

Screen window[MAX_WINDOW];
Console *cns;

static inline bool is_decimal(const char ch)
{
	return (('0' <= ch)
			&& (ch <= '9'));
}

//TODO return adequate error code
static bool check_channel(const int);
static bool check_param(const int, const off_t, const size_t);
static void _putc(esc_state_t *, const char);
static void _push(esc_state_t *, const char);
static void state_null(esc_state_t *, const char);
static void state_esc(esc_state_t *, const char);
static void state_csi(esc_state_t *, const char);
static void state_exec_csi(esc_state_t *);
static void eputc(esc_state_t *, const char);

static esc_state_t state = { state_null };


int detach(void)
{
	return 0;
}

int open(const char *name)
{
	return 0;
}

int close(const int channel)
{
	return (check_channel(channel) ? 0 : (-1));
}

int read(char *outbuf, const int channel, const off_t offset, const size_t size)
{
	return (-1);
}

int write(char *inbuf, const int channel, const off_t offset, const size_t size)
{
	if (!check_param(channel, offset, size))
		return (-1);

	Screen *s = &(window[channel]);
	if (channel)
		for (int i = 0; i < size; i++)
			cns->putc(s, inbuf[i]);
	else {
		state.screen = s;
		for (int i = 0; i < size; i++)
			eputc(&state, inbuf[i]);
	}

	return size;
}

static bool check_channel(const int channel)
{
	if ((channel < 0)
			|| (channel >= sizeof(window) / sizeof(window[0])))
		return false;

	return true;
}

static bool check_param(const int channel, const off_t offset,
		const size_t size)
{
	if (!check_channel(channel))
		return false;

	if (offset < 0)
		return false;

	return true;
}

#if 0
int console_panic(const int dd)
{
	if (dd >= sizeof(window) / sizeof(window[0]))
		return E_PAR;

	Screen *s = &(window[dd]);
	s->fgcolor.rgb.b = 0;
	s->fgcolor.rgb.g = 0;
	s->fgcolor.rgb.r = 255;
	s->bgcolor.rgb.b = 0;
	s->bgcolor.rgb.g = 0;
	s->bgcolor.rgb.r = 0;
	return E_OK;
}
#endif

static void _putc(esc_state_t *state, const char ch)
{
	cns->putc(state->screen, ch);
}

static void _push(esc_state_t *state, const char ch)
{
	state->buf[state->len++] = ch;
}

static void state_null(esc_state_t *state, const char ch)
{
	if (ch == ESC) {
		state->buf[0] = ch;
		state->len = 1;
		state->func = state_esc;
		return;
	}

	_putc(state, ch);
}

static void state_esc(esc_state_t *state, const char ch)
{
	_push(state, ch);

	if (ch == CSI) {
		for (int i = 0; i < ESC_MAX_PARAMS; i++)
			state->params[i] = 0;

		state->num_of_params = 0;
		state->columns = 0;
		state->func = state_csi;
		return;
	}

	state->func = state_null;
}

static void state_csi(esc_state_t *state, const char ch)
{
	_push(state, ch);

	if (ch == DELIM) {
		if (state->num_of_params < ESC_MAX_PARAMS - 1) {
			state->num_of_params++;
			state->columns = 0;
			return;
		}
	} else if (is_decimal(ch)) {
		if (state->columns < ESC_MAX_COLUMNS) {
			state->columns++;
			state->params[state->num_of_params] *= 10;
			state->params[state->num_of_params] += ch - '0';
			return;
		}
	} else {
		state_exec_csi(state);
		state->func = state_null;
		return;
	}

	state->func = state_null;
}

static void state_exec_csi(esc_state_t *state)
{
	switch (state->buf[state->len - 1]) {
	case 'H':
		if (!(state->params[0]))
			state->params[0] = 1;

		if (!(state->params[1]))
			state->params[1] = 1;

		cns->locate(state->screen, state->params[1] - 1,
				state->params[0] - 1);
		break;
	case 'J':
		if (state->num_of_params < 1)
			switch (state->params[0]) {
			case 0:
				cns->erase(state->screen,
						EraseScreenFromCursor);
				break;
			case 1:
				cns->erase(state->screen, EraseScreenToCursor);
				break;
			case 2:
				cns->erase(state->screen, EraseScreenEntire);
				break;
			default:
				break;
			}
		break;
	default:
		break;
	}
}

static void eputc(esc_state_t *state, const char ch)
{
	state->func(state, ch);
}
