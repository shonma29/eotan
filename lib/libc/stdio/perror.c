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

static const char * const sys_errlist[] = {
	NULL,
	"E2BIG",
	"EACCES",
	"EADDRINUSE",
	"EADDRNOTAVAIL",
	"EAFNOSUPPORT",
	"EAGAIN",
	"EALREADY",
	"EBADF",
	"EBADMSG",
	"EBUSY",
	"ECANCELED",
	"ECHILD",
	"ECONNABORTED",
	"ECONNREFUSED",
	"ECONNRESET",
	"EDEADLK",
	"EDESTADDRREQ",
	"EDOM",
	"EDQUOT",
	"EEXIST",
	"EFAULT",
	"EFBIG",
	"EHOSTUNREACH",
	"EIDRM",
	"EILSEQ",
	"EINPROGRESS",
	"EINTR",
	"EINVAL",
	"EIO",
	"EISCONN",
	"EISDIR",
	"ELOOP",
	"EMFILE",
	"EMLINK",
	"EMSGSIZE",
	"EMULTIHOP",
	"ENAMETOOLONG",
	"ENETDOWN",
	"ENETRESET",
	"ENETUNREACH",
	"ENFILE",
	"ENOBUFS",
	"ENODATA",
	"ENODEV",
	"ENOENT",
	"ENOEXEC",
	"ENOLCK",
	"ENOLINK",
	"ENOMEM",
	"ENOMSG",
	"ENOPROTOOPT",
	"ENOSPC",
	"ENOSR",
	"ENOSTR",
	"ENOSYS",
	"ENOTCONN",
	"ENOTDIR",
	"ENOTEMPTY",
	"ENOTSOCK",
	"ENOTSUP",
	"ENOTTY",
	"ENXIO",
	"EOPNOTSUPP",
	"EOVERFLOW",
	"EPERM",
	"EPIPE",
	"EPROTO",
	"EPROTONOSUPPORT",
	"EPROTOTYPE",
	"ERANGE",
	"EROFS",
	"ESPIPE",
	"ESRCH",
	"ESTALE",
	"ETIME",
	"ETIMEDOUT",
	"ETXTBSY",
	"EWOULDBLOCK",
	"EXDEV"
};

#define sys_nerr (sizeof(sys_errlist) / sizeof(sys_errlist[0]))

void perror(const char *string)
{
	int n = errno;

	if (string)
		if (*string)
			fprintf(stderr, "%s: ", string);

	if ((n > 0) && (n < sys_nerr))
		fprintf(stderr, "%s\n", sys_errlist[n]);

	else
		fprintf(stderr, "Undefined error: %d\n", n);
}
