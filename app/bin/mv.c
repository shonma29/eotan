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
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "_perror.h"

#define BUFSIZ (8192)

#define MSG_ARG "usage: mv file1 file2\n       mv file ... directory\n"
#define MSG_TOO_LONG_PATH ": too long path\n"
#define MSG_BAD_PATH ": bad path\n"
#define MSG_IS_DIRECTORY ": is directory\n"
#define MSG_IS_IDENTICAL ": is identical\n"

static char path_buf[PATH_MAX];

static int print(const int, const int, const char *, const char *);
static int process_file(const char *, const char *);
static int process_directory(const char *, const char *, const int);


static int print(const int dest, const int src, const char *dest_name,
		const char *src_name)
{
	for (;;) {
		char buf[BUFSIZ];
		int len = read(src, buf, sizeof(buf));
		if (len == -1) {
			_perror(src_name);
			return EXIT_FAILURE;
		}

		if (len == 0)
			break;

		if (write(dest, buf, len) == -1) {
			_perror(dest_name);
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

static int process_file(const char *dest_name, const char *src_name)
{
	do {
		int src = open(src_name, O_RDONLY);
		if (src == -1) {
			_perror(src_name);
			break;
		}

		struct stat src_st;
		if (fstat(src, &src_st)) {
			_perror(src_name);
			close(src);
			break;
		}

		if (src_st.st_mode & S_IFDIR) {
			_put_error(src_name);
			_put_error(MSG_IS_DIRECTORY);
			close(src);
			break;
		}

		int dest = open(dest_name, O_WRONLY | O_TRUNC);
		if (dest == -1) {
			dest = creat(dest_name, src_st.st_mode & 0777);
			if (dest == -1) {
				_perror(dest_name);
				close(src);
				break;
			}
		}

		struct stat dest_st;
		if (fstat(dest, &dest_st)) {
			_perror(dest_name);
			close(dest);
			close(src);
			break;
		}

		if ((src_st.st_dev == dest_st.st_dev)
				&& (src_st.st_ino == dest_st.st_ino)) {
			_put_error(src_name);
			_put_error(MSG_IS_IDENTICAL);
			close(dest);
			close(src);
			break;
		}

		int result = print(dest, src, dest_name, src_name);

		if (close(dest)) {
			_perror(dest_name);
			close(src);
			break;
		}

		if (close(src)) {
			_perror(src_name);
			break;
		}

		if (unlink(src_name)) {
			_perror(src_name);
			break;
		}

		return result;
	} while (false);

	return EXIT_FAILURE;
}

static int process_directory(const char *dest_path, const char *src_path,
		const int dest_len)
{
	char *file_name = strrchr(src_path, '/');
	if (file_name)
		file_name++;
	else
		file_name = (char *) src_path;

	if (!(*file_name)) {
		_put_error(src_path);
		_put_error(MSG_BAD_PATH);
		return EXIT_FAILURE;
	}

	if (strlen(file_name) >= sizeof(path_buf) - dest_len) {
		_put_error(src_path);
		_put_error(MSG_TOO_LONG_PATH);
		return EXIT_FAILURE;
	}

	strcpy((char *) &(dest_path[dest_len]), file_name);
	return process_file(dest_path, src_path);
}

void _main(int argc, char *argv[], char *envp[])
{
	do {
		if (argc <= 2)
			break;

		//TODO delete src
		//TODO set mode and time from src if dest exists
		//TODO support rename file
		//TODO support rename directory

		int dest = argc - 1;
		int result = EXIT_SUCCESS;
		struct stat st;
		if (!stat(argv[dest], &st)
				&& (st.st_mode & S_IFDIR)) {
			size_t len = strlen(argv[dest]);
			if (!len)
				break;

			if (argv[dest][len - 1] != '/')
				len++;

			if (len >= sizeof(path_buf)) {
				_put_error(argv[dest]);
				_put_error(MSG_TOO_LONG_PATH);
				_exit(EXIT_FAILURE);
			}

			strcpy(path_buf, argv[dest]);
			path_buf[len - 1] = '/';

			for (int i = 1; i < dest; i++)
				if (process_directory(path_buf, argv[i], len)) {
					result = EXIT_FAILURE;
					break;
				}
		} else if (argc != 3)
			break;
		else
			result = process_file(argv[dest], argv[1]);

		_exit(result);
	} while (false);

	_put_error(MSG_ARG);
	_exit(EXIT_FAILURE);
}
