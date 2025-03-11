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
#include <unistd.h>
#include <fcntl.h>

#define ERR (-1)

static char RAWON[] = { 'r', 'a', 'w', 'o', 'n' };
static char RAWOFF[] = { 'r', 'a', 'w', 'o', 'f', 'f' };

static int consctl(const char *, const size_t);


static int consctl(const char *str, const size_t size)
{
	int fd = open("/dev/consctl", O_WRONLY);
	if (fd < 0)
		return ERR;

	int result = (write(fd, str, size) == size) ? 0 : ERR;
	close(fd);
	return result;
}

int rawon(void)
{
	return consctl(RAWON, sizeof(RAWON));
}

int rawoff(void)
{
	return consctl(RAWOFF, sizeof(RAWOFF));
}
