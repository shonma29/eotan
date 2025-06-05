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

#include <unistd.h>
#include <fcntl.h>
#include <libc.h>
#include <sys/wait.h>

#include <event.h>
#include <win/keyboard.h>
#include <hmi/terminal.h>

#define MYNAME "term"

#define ERR (-1)

#define WIDTH (512 - 2 - 2)
#define HEIGHT (374 - 13 - 2 - 3)

//TODO get global Display and window
static Display display = {
	{ { 0, 0 }, { WIDTH, HEIGHT } },
	NULL,
	WIDTH * sizeof(Color_Rgb),
	sizeof(Color_Rgb),
	B8G8R8
};
//static Rectangle viewport = { { 2, 13 + 2 }, { WIDTH - 3, HEIGHT - 3 } };
static Screen screen;
static esc_state_t state;

static struct {
	int op;
	blit_param_t param;
} packet;

static char const * const array[] = { "/bin/shell", NULL };
static long draw_semaphore;
static int event_fd;
static int draw_fd;

static void _put_error(char const *);
static void _show_error(char const *);
static int _initialize_screen(void);
static int _redraw_text(char const * const, ssize_t const);
static int _blit(int const, Rectangle const * const);
static void _collect(void);
static int _tunnel_out(int const, int const);
static int _tunnel_in(int const, int const);
static int _execute(char const * const *, char const * const *, int const fds[2]);


static void _put_error(char const *message) {
	write(STDERR_FILENO, message, strlen(message));
}

static void _show_error(char const *message) {
	char *error_string = strerror(errno);
	_put_error(MYNAME ": ");
	_put_error(message);
	_put_error(" ");
	_put_error(error_string);
	_put_error("\n");
}

static int _initialize_screen(void)
{
	errno = 0;
	void *buf = calloc(sizeof(Color_Rgb), WIDTH * HEIGHT);
	if (!buf) {
		_show_error("memory exhausted.");
		return ERR;
	}

	display.base = buf;
	packet.op = draw_op_blit;
	packet.param.bpl = display.bpl;
	packet.param.type = display.type;

	state.screen = &screen;
	terminal_initialize(&state, &display);

	screen.base = (void *) ((uintptr_t) display.base
			+ display.r.min.y * display.bpl
			+ display.r.min.x * display.bpp);
	screen.p = (uint8_t *) (screen.base);
	screen.width = display.r.max.x - display.r.min.x;
	screen.height = display.r.max.y - display.r.min.y;
	screen.chr_width = screen.width / screen.font.width;
	screen.chr_height = screen.height / screen.font.height;
	terminal_write(STR_CONS_INIT, &state, LEN_CONS_INIT);
	return _blit(draw_fd, &(display.r));
}

static int _redraw_text(char const * const buf, ssize_t const len)
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

	terminal_write((char *) buf, &state, len);

	int result = _blit(draw_fd, &(display.r));

	errno = 0;
	if (semrelease(&draw_semaphore, 1) != 1) {
		_show_error("failed to semrelease.");
		return ERR;
	}

	if (result)
		return ERR;

	return 0;
}

static int _blit(int const fd, Rectangle const * const r)
{
	blit_param_t *par = &(packet.param);
	par->dest = *r;
	par->base = (void *) ((uintptr_t) (display.base)
			+ r->min.y * display.bpl
			+ r->min.x * display.bpp);

	errno = 0;
	int result = write(fd, &packet, sizeof(packet));
	if (result != sizeof(packet)) {
		_show_error("failed to write draw.");
		return ERR;
	}

	return 0;
}

static void _collect(void)
{
	for (int status; waitpid(-1, &status, WNOHANG) > 0;)//;
	{fprintf(stderr, "term collect %d\n",  status);fflush(stderr);}
		;
		fprintf(stderr, "term collect done\n");fflush(stderr);
}

static int _tunnel_out(int const out, int const in)
{
	for (;;) {
		errno = 0;

		char buf[BUFSIZ];
		ssize_t len = read(in, buf, sizeof(buf));
		if (!len)
			break;
		else if (len < 0) {
			_show_error("failed to read tunnel.");
			return ERR;
		}

		int result = _redraw_text(buf, len);
		if (result)
			return result;
	}

	return 0;
}

static int _tunnel_in(int const out, int const in)
{return 0;
	for (;;) {
		errno = 0;

		event_message_t message[BUFSIZ / sizeof(event_message_t)];
		ssize_t len = read(event_fd, message, sizeof(message));
		if ((len <= 0)
				|| (len % sizeof(event_message_t))) {
			_show_error("failed to read event.");
			break;
		}

		char buf[BUFSIZ];
		char *p = buf;
		len /= sizeof(event_message_t);
		for (int i = 0; i < len; i++) {
			if (message[i].type != event_keyboard)
				continue;

			int k = keyboard_convert(message[i].data);
			if (k < 0)
				continue;

			*p = k & 0xff;
			p++;
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

		len = p - buf;
		if (len <= 0)
			continue;

		ssize_t result = _redraw_text(buf, len);
		if (result)
			return result;

		errno = 0;
		result = write(out, buf, len);
		if (result != len) {
			_show_error("failed to write tunnel.");
			return ERR;
		}
	}

	return 0;
}

static int _execute(char const * const *array, char const * const *env,
		int const fds[2])
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
			int result = _tunnel_in(fds[1], STDIN_FILENO);
			_collect();
			return result;
		} else
			return _tunnel_out(STDOUT_FILENO, fds[1]);
	} else {
		for (int i = STDIN_FILENO; i <= STDERR_FILENO; i++) {
			close(i);
			errno = 0;
			if (dup2(fds[0], i) < 0)
				return ERR;
		}

		close(fds[0]);
		close(fds[1]);
		errno = 0;
		execve(array[0], (char * const *) array, (char * const *) env);

		// put to the parent process
		_show_error("failed to execve.");
		return ERR;
	}
}

int main(int argc, char **argv, char **env)
{
	//TODO 'bind' returns positive integer
	if (bind("#i", "/dev", MREPL) < 0) {
		_show_error("failed to bind.");
		return EXIT_FAILURE;
	}

	event_fd = open("/dev/event", O_RDONLY);
	if (event_fd < 0) {
		_show_error("failed to open event.");
		return EXIT_FAILURE;
	}

	draw_fd = open("/dev/draw", O_WRONLY);
	if (draw_fd < 0) {
		_show_error("failed to open draw.");
		return EXIT_FAILURE;
	}

	if (semrelease(&draw_semaphore, 1) != 1) {
		_show_error("failed to create semaphore.");
		return EXIT_FAILURE;
	}

	if (_initialize_screen())
		return EXIT_FAILURE;

	errno = 0;

	int fds[2];
	if (pipe(fds)) {
		_show_error("failed to pipe.");
		return EXIT_FAILURE;
	}

	//TODO caluculate from Display
	char const * const envp[] = { "COLUMNS=84", "LINES=29", NULL };
	return (_execute(array, envp, fds) ? EXIT_FAILURE : EXIT_SUCCESS);
}
