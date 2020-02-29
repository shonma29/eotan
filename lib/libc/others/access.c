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
#include <fcntl.h>
#include <local.h>
#include <unistd.h>
#include <sys/stat.h>

static int can_access(const struct stat *, const int);


int access(const char *path, int mode)
{
	if (mode == F_OK)
		mode = 0;
	else {
		int m = mode & (X_OK | W_OK | R_OK);

		if (!m || m != mode) {
			_set_local_errno(EINVAL);
			return -1;
		}

		mode = m;
	}

	int fd = open(path, O_RDONLY);
	if (fd < 0)
		return fd;

	if (!mode) {
		close(fd);
		return 0;
	}

	struct stat stat;
	int result = fstat(fd, &stat);
	close(fd);

	if (result)
		return result;

	if (can_access(&stat, mode))
		return 0;

	_set_local_errno(EACCES);
	return -1;
}

static int can_access(const struct stat *stat, const int mode)
{
	int bits = stat->st_mode & mode;
	uid_t uid = getuid();

	if (stat->st_uid == uid)
		bits |= (stat->st_mode >> 6) & mode;

	if (stat->st_gid == getgid())
		bits |= (stat->st_mode >> 3) & mode;

	return (bits == mode);
}
