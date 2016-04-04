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
#include <mpu/io.h>
#include <sys/types.h>
#include "func.h"
#include "sync.h"

#define EPOCH_DAYS (719163)

static unsigned char _read(const unsigned short port);
static void _get_time(time_t *seconds);


static unsigned char _read(const unsigned short port) {
	unsigned char n;

	outb(0x70, port);
	n = inb(0x71);
	n = ((n >> 4) & 0xf) * 10 + (n & 0xf);

	return n;
}

static void _get_time(time_t *seconds) {
	int year = _read(0x32) * 100 + _read(0x09);
	int month = _read(0x08);

	if (month <= 2) {
		year--;
		month += 12;
	}

	*seconds = 365LL * year + year / 4 - year / 100 + year / 400
			+ 306 * (month + 1) / 10 + _read(0x07) - 428
			- EPOCH_DAYS;
	*seconds *= 24LL * 60LL * 60LL;
	*seconds += (time_t)((_read(0x04)) * 60 + _read(0x02)) * 60
			+ _read(0x00);
}

void rtc_get_time(time_t *seconds)
{
	time_t t1;
	time_t t2;

	do {
		enter_critical();
		_get_time(&t1);
		_get_time(&t2);
		leave_critical();
	} while (t1 != t2);

	*seconds = t1;
}
