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
#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/dirent.h>
#include <sys/stat.h>

#define MYNAME "ls"

#define MSG_OPEN "No such file or directory"
#define MSG_READ "failed to read"
#define MSG_CLOSE "failed to close"
#define MSG_STAT "failed to get status"
#define MSG_MALLOC "No memory"
#define MSG_NOT_DIR "Not a directory"

#define DIRENT_BUFSIZ 16348

#define OK (0)
#define NG (1)

static char dir_buf[DIRENT_BUFSIZ];

static void puterror(const char *, const char *);
static void format(FILE *, const char *, const char *);
static int process(FILE *, const char *, const int, bool *);


static void puterror(const char *name, const char *message)
{
	fprintf(stderr, MYNAME ": %s: %s\n", name, message);
}

static void format(FILE *out, const char *path, const char *name)
{
	struct stat st;
	if (stat(path, &st)) {
		puterror(path, MSG_STAT);
		return;
	}

	char time_buf[13];
	struct tm t;
	strftime(time_buf, sizeof(time_buf), "%b %d %H:%M",
			gmtime_r(&(st.st_mtime), &t));

	int size_buf[2];
	long long *p = (long long *) size_buf;
	*p = st.st_size;

	fprintf(out, "%o %d %d %d %d %s %s\n",
			st.st_mode, st.st_nlink, st.st_uid, st.st_gid,
			size_buf[0], time_buf, name);
}

static int process(FILE *out, const char *name, const int argc, bool *is_block)
{
	*is_block = false;

	int in = open(name, O_RDONLY);
	if (in == -1) {
		puterror(name, MSG_OPEN);
		return NG;
	}

	struct stat st;
	if (fstat(in, &st)) {
		close(in);
		puterror(name, MSG_STAT);
		return NG;
	}

	int result = OK;
	size_t path_len = strlen(name);
	if (st.st_mode & S_IFDIR) {
		if (name[path_len - 1] == '/')
			path_len--;

		char *buf = malloc(path_len + 1 + PATH_MAX + 1);
		if (!buf) {
			close(in);
			puterror(name, MSG_MALLOC);
			return NG;
		}

		strcpy(buf, name);
		buf[path_len] = '/';

		if (argc > 2)
			fprintf(out, "%s:\n", name);

		for (;;) {
			int len = read(in, dir_buf, sizeof(dir_buf));
			if (len < 0) {
				puterror(name, MSG_READ);
				result = NG;
				break;
			} else if (!len)
				break;

			struct dirent *p = (struct dirent *) dir_buf;
			for (int i = len / sizeof(*p); i > 0; p++, i--) {
				strcpy(&(buf[path_len + 1]), p->d_name);
				format(out, buf, p->d_name);
			}

			*is_block = true;;
		}

		free(buf);
	} else if (name[path_len - 1] == '/') {
		puterror(name, MSG_NOT_DIR);
		result = NG;
	} else
		format(out, name, name);

	if (close(in)) {
		puterror(name, MSG_CLOSE);
		result = NG;
	}

	return result;
}

int main(int argc, char **argv)
{
	int result = OK;
	bool is_block = false;

	if (argc == 1)
		result |= process(stdout, ".", argc, &is_block);
	else
		for (int i = 1; i < argc; i++) {
			if (is_block)
				fprintf(stdout, "\n");

			result |= process(stdout, argv[i], argc, &is_block);
		}

	return (result ? EXIT_FAILURE : EXIT_SUCCESS);
}
