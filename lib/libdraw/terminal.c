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
#include <console.h>
#include <hmi/terminal.h>

#define ESC 0x1b
#define CSI '['
#define DELIM ';'

static Screen root;
static Console *cns = NULL;

static inline bool is_decimal(const char ch)
{
	return (('0' <= ch)
			&& (ch <= '9'));
}

//TODO return adequate error code
static void _push(esc_state_t *, const char);
static void state_null(esc_state_t *, const char);
static void state_esc(esc_state_t *, const char);
static void state_csi(esc_state_t *, const char);
static void state_csi_private(esc_state_t *, const char);
static void state_exec_csi(esc_state_t *);
static void eputc(esc_state_t *, const char);


void terminal_initialize(esc_state_t *state, const Display *display)
{
	if (!cns)
		cns = getConsole(&root, display, &default_font);

	state->func = state_null;
	*(state->screen) = root;
}

int terminal_write(char *inbuf, esc_state_t *state, const size_t size)
{
	if (!state)
		return (-1);

	for (int i = 0; i < size; i++)
		eputc(state, inbuf[i]);

	//TODO signed int?
	return size;
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

	//TODO show control char if buf < ' '
	cns->putc(state->screen, ch);
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
	} else if (ch == '?') {
		if ((state->num_of_params == 0)
				&& (state->columns == 0)) {
			state->func = state_csi_private;
			return;
		}
	} else if (is_decimal(ch)) {
		if (state->columns < ESC_MAX_COLUMNS) {
			state->columns++;
			state->params[state->num_of_params] *= 10;
			state->params[state->num_of_params] += ch - '0';
			return;
		}
	} else
		state_exec_csi(state);

	state->func = state_null;
}

static void state_csi_private(esc_state_t *state, const char ch)
{
	_push(state, ch);

	if (is_decimal(ch)) {
		if (state->columns < ESC_MAX_COLUMNS) {
			state->columns++;
			state->params[state->num_of_params] *= 10;
			state->params[state->num_of_params] += ch - '0';
			return;
		}
	} else
		switch (state->buf[state->len - 1]) {
		case 'h':
			if (state->params[0] == 7)
				state->screen->wrap = false;

			break;
		case 'l':
			if (state->params[0] == 7)
				state->screen->wrap = true;

			break;
		default:
			break;
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
	case 'K':
		if (state->num_of_params < 1)
			switch (state->params[0]) {
			case 0:
				cns->erase(state->screen, EraseLineFromCursor);
				break;
			case 1:
				cns->erase(state->screen, EraseLineToCursor);
				break;
			case 2:
				cns->erase(state->screen, EraseLineEntire);
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
