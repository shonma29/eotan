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
#include <stdbool.h>
#include <time.h>

static int days[] = {
	0, 31, 61, 92, 122, 153,
	184, 214, 245, 275, 306, 337
};

static struct tm t;


struct tm *gmtime_(const time_t *timep)
{
	time_t x = *timep;
	time_t year;
	int month;
	int day;
	int hour;
	bool isLeap = false;

	/* calculate time */
	hour = x % (60 * 60 * 24);
	x = x / (60 * 60 * 24);
	t.tm_sec = hour % 60;
	hour = hour / 60;
	t.tm_min = hour % 60;
	t.tm_hour = hour / 60;
	t.tm_wday = (x + 4) % 7;

	/* calculate year */
	x += 719527 - (31 + 28);
	year = x / 146097 * 400;
	x %= 146097;
	isLeap = (x == 146097 - 1);
	year += x / 36524 * 100;
	x %= 36524;
	year += x / 1461 * 4;
	x %= 1461;
	year += x / 365;
	day = x % 365;

	if (!day) {
		isLeap |= (x == 1461 - 1);

		if (isLeap) {
			year--;
			day = 365;
		}
	}

	/* calculate day of month */
	for (month = 1; (month < 12) && (days[month] <= day); month++);
	t.tm_mday = day - days[month - 1] + 1;

	if (++month >= 12) {
		day -= days[10];
		month -= 12;
		year++;
	} else
		day += ((year % 400)? ((year % 100)? (!(year % 4)):0):1)?
				(31 + 29):(31 + 28);

	t.tm_mon = month;
	t.tm_year = year - 1900;
	t.tm_yday = day;
	t.tm_isdst = 0;

	return &t;
}
