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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <libc.h>
#include <sys/wait.h>
#include <event.h>
#include <win/keyboard.h>
#include <win/window.h>
#include <hmi/terminal.h>
#include "_perror.h"

#define MYNAME "nterm"

#define ERR (-1)

#define BUF_SIZE (8192)

#define WIDTH (512)
#define HEIGHT (374)

typedef struct {
	size_t position;
	bool raw_mode;
	int out;
	Window *window;
	char buf[BUF_SIZE];
} in_buffer_t;

static Screen screen;
static esc_state_t state;

static char const * const array[] = { "/bin/shell", NULL };
static long draw_semaphore;

extern void __malloc_initialize(void);

static void _show_error(char const *);
static void _initialize_screen(Window * const);
static int _redraw_text(Window * const, char const * const, ssize_t const);
static void _collect(void);
static int _tunnel_out(Window * const, int const, int const);
static void _reset(in_buffer_t * const);
static void _put(in_buffer_t * const, int const);
static void _control(in_buffer_t * const, bool const);
static ssize_t _flush(in_buffer_t * const);
static int _tunnel_in(in_buffer_t * const, int const, int const);
static int _execute(in_buffer_t * const, char const * const *,
		char const * const *, int const fds[2]);


static void _show_error(char const *message) {
	char *error_string = strerror(errno);
	_put_error(MYNAME ": ");
	_put_error(message);
	_put_error(" ");
	_put_error(error_string);
	_put_error("\n");
}

static void _initialize_screen(Window * const w)
{
	state.screen = &screen;
	terminal_initialize(&state, &(w->display));

	screen.base = (void *) ((uintptr_t) w->display.base
			+ w->inner.min.y * w->display.bpl
			+ w->inner.min.x * w->display.bpp);
	screen.p = (uint8_t *) (screen.base);
	screen.width = w->inner.max.x - w->inner.min.x;
	screen.height = w->inner.max.y - w->inner.min.y;
	screen.chr_width = screen.width / screen.font.width;
	screen.chr_height = screen.height / screen.font.height;
	terminal_write(&state, STR_CONS_INIT, LEN_CONS_INIT);
}

static int _redraw_text(Window * const w, char const * const buf,
		ssize_t const len)
{
	for (;;) {
		errno = 0;
		int cnt = semacquire(&draw_semaphore, 1);
		if (cnt == 1)
			break;

		if (cnt == -1) {
			_show_error("failed to semacquire.");
			return ERR;
		}
	}

	terminal_write(&state, (char *) buf, len);

	int result = window_blit(w, &(w->inner));

	errno = 0;
	if (semrelease(&draw_semaphore, 1) != 1) {
		_show_error("failed to semrelease.");
		return ERR;
	}

	if (result) {
		_put_error(MYNAME ": failed to blit\n");
		return ERR;
	}

	return 0;
}

static void _collect(void)
{
	for (int status; wait(&status) > 0;);
}

static int _tunnel_out(Window * const w, int const out, int const in)
{
	for (;;) {
		errno = 0;

		char buf[BUF_SIZE];
		ssize_t len = read(in, buf, sizeof(buf));
		if (!len)
			break;
		else if (len < 0) {
			_show_error("failed to read tunnel.");
			return ERR;
		}

		int result = _redraw_text(w, buf, len);
		if (result)
			return result;
	}

	return 0;
}

static void _reset(in_buffer_t * const ib)
{
	ib->position = 0;
}

static void _put(in_buffer_t * const ib, int const ch)
{
	if (ib->position >= sizeof(ib->buf))
		_flush(ib);

	ib->buf[ib->position] = ch & 0xff;
	ib->position++;
}

static void _control(in_buffer_t * const ib, bool const flag)
{
	if (ib->raw_mode != flag) {
		_flush(ib);
		ib->raw_mode = flag;
	}
}

static ssize_t _flush(in_buffer_t * const ib)
{
	if (ib->position <= 0)
		return 0;

	if (!(ib->raw_mode)) {
		int result = _redraw_text(ib->window, ib->buf, ib->position);
		if (result)
			return result;
	}

	errno = 0;
	ssize_t result = write(ib->out, ib->buf, ib->position);
	if (result != ib->position) {
		_show_error("failed to write tunnel.");
		return ERR;
	}

	_reset(ib);
	return 0;
}

static int _tunnel_in(in_buffer_t * const ib, int const out, int const in)
{
	ib->out = out;

	for (;;) {
		errno = 0;

		event_message_t message[BUF_SIZE / sizeof(event_message_t)];
		ssize_t len = read(ib->window->event_fd, message,
				sizeof(message));
		if ((len <= 0)
				|| (len % sizeof(event_message_t))) {
			_show_error("failed to read event.");
			break;
		}

		_reset(ib);
		len /= sizeof(event_message_t);
		for (int i = 0; i < len; i++) {
			if (message[i].type == event_consctl) {
				_control(ib, message[i].data);
				continue;
			}

			if (message[i].type != event_keyboard)
				continue;

			int k = keyboard_convert(message[i].data);
			if (k < 0)
				continue;

			_put(ib, k);
#if 0
			if (!raw_mode) {
				if (buf == 0x03) {
					// ^c
					sys_args_t args = {
						syscall_kill,
						focused_session->tid,
						SIGINT
					};
					int error_no = kcall->ipc_send(PORT_MM, &args,
							sizeof(args));
					if (error_no)
						log_err(MYNAME ": kill error %d\n",
								error_no);

					return;
				} else
#endif
#if 0
				if (buf == 0x04) {
					// ^d
					event_buf_t *p = &(focused_session->event);
					list_t *head = list_dequeue(&(p->readers));
					if (head) {
						fs_request_t *req = getRequestFromQueue(head);
						int error_no = reply_read(req);
						if (error_no)
							log_warning(MYNAME ": reply error %d\n",
									error_no);
					}

					return;
				}
#endif
#if 0
			}
#endif
		}

		ssize_t result = _flush(ib);
		if (result)
			return result;
	}

	return 0;
}

static int _execute(in_buffer_t * const ib, char const * const *array,
		char const * const *env, int const fds[2])
{
	errno = 0;

	pid_t child = fork();
	if (child == ERR) {
		_show_error("failed to fork child.");
		return ERR;
	} else if (child) {
		close(fds[0]);
		errno = 0;

		pid_t out = rfork(RFPROC | RFMEM);
		if (out == ERR) {
			_show_error("failed to fork out.");
			return ERR;
		} else if (out) {
			int result = _tunnel_in(ib, fds[1], STDIN_FILENO);
			_collect();
			return result;
		} else
			return _tunnel_out(ib->window, STDOUT_FILENO, fds[1]);
	} else {
		for (int i = STDIN_FILENO; i <= STDERR_FILENO; i++) {
			errno = 0;
			if (dup2(fds[0], i) < 0)
				return ERR;
		}
#if 0
		close(fds[0]);
#endif
		close(fds[1]);
		errno = 0;
		execve(array[0], (char * const *) array, (char * const *) env);

		// put to the parent process
		_show_error("failed to execve.");
		return ERR;
	}
}

void _main(int argc, char **argv, char **env)
{
	errno = 0;
	__malloc_initialize();

	in_buffer_t *ib = malloc(sizeof(in_buffer_t));
	if (!ib) {
		_put_error(MYNAME ": memory exhausted\n");
		_exit(EXIT_FAILURE);
	}

	//TODO get global Display and window
	Window *w;
	if (window_initialize(&w, WIDTH, HEIGHT, WINDOW_ATTR_WINDOW)) {
		_put_error(MYNAME ": failed to open window\n");
		_exit(EXIT_FAILURE);
	}

	window_set_title(w, MYNAME);
	window_draw_frame(w);
	_initialize_screen(w);

	if (window_blit(w, &(w->display.r))) {
		_put_error(MYNAME ": failed to blit\n");
		_exit(EXIT_FAILURE);
	}

	ib->raw_mode = false;
	ib->window = w;
	errno = 0;

	if (semrelease(&draw_semaphore, 1) != 1) {
		_show_error("failed to create semaphore.");
		_exit(EXIT_FAILURE);
	}

	int fds[2];
	if (pipe(fds)) {
		_show_error("failed to pipe.");
		_exit(EXIT_FAILURE);
	}

	char fileno[12 + 11 + 1];
	sprintf(fileno, "CONS_FILENO=%d", fds[0]);

	char columns[8 + 11 + 1];
	char lines[6 + 11 + 1];
	sprintf(columns, "COLUMNS=%d",
			(w->inner.max.x - w->inner.min.x) / screen.font.width);
	sprintf(lines, "LINES=%d",
			(w->inner.max.y - w->inner.min.y) / screen.font.height);

	char const * const envp[] = { columns, lines, fileno, NULL };
	_exit(_execute(ib, array, envp, fds) ?
			EXIT_FAILURE : EXIT_SUCCESS);
}
