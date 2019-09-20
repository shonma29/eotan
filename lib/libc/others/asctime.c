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
#if 0
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static char *months[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
static char *weekdays[] = {
	"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
};
static int days[] = {
	0, 31, 61, 92, 122, 153,
	184, 214, 245, 275, 306, 337
};

char *calc(char *p, time_t t, int offset)
{
	time_t x = t = t + offset;
	time_t year;
	int month;
	int day;
	int hour;
	int min;
	int sec;
	int weekday;
	int q;

	hour = x % (60 * 60 * 24);
	x = x / (60 * 60 * 24);

	sec = hour % 60;
	hour = hour / 60;
	min = hour % 60;
	hour = hour / 60;
	weekday = (x + 3) % 7;

	x += 719527 - 59;
	year = x / 146097 * 400;
	x %= 146097;
	year += x / 36524 * 100;
	x %= 36524;
	year += x / 1461 * 4;
	x %= 1461;

	q = x / 365;
	year += q;

	// day
	day = x - q * 365 + 1;

	// month
	for (month = 1; month < 12; month++)
		if (day <= days[month])
			break;

	day -= days[month - 1];

	if (++month >= 12) {
		year++;
		month -= 12;
	}

	sprintf(p, "%s %s %2d %02d:%02d:%02d %lld\n",
			weekdays[weekday], months[month], day,
			hour, min, sec, (long long int)year);
	return p;
}

int main(int argc, char **argv)
{
	//time_t t = time(NULL);
	time_t t = 0;
	int i;
	int e = 0;

	printf("start\n");

	for (i = 0; i < 1024 * 1024 * 1024; i++) {
		struct tm *tm = localtime(&t);
		char str[64];
		char *p = asctime(tm);
		char *q = calc(str, t, 9 * 60 * 60);

		if (strcmp(p, q)) {
			printf("[p]%s", p);
			printf("[q]%s", q);
			printf("----%d----\n", i);

			if (++e >= 10)
				break;
		}

		t += 10001;
	}

	printf("done\n");

	return 0;
}

#endif
