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
#include <time.h>

#define EPOCH_DAYS (719162)

#define DAYS_PER_YEAR (365)
#define DAYS_PER_4YEARS (DAYS_PER_YEAR * 4 + 1)
#define DAYS_PER_100YEARS (DAYS_PER_4YEARS * 25 - 1)
#define DAYS_PER_400YEARS (DAYS_PER_100YEARS * 4 + 1)

#define SECONDS_PER_MINUTE (60)
#define SECONDS_PER_HOUR (SECONDS_PER_MINUTE * 60)
#define SECONDS_PER_DAY (SECONDS_PER_HOUR * 24)

static int days_of_month[2][12] = {
	{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
	{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

static int *get_days_of_month(const int year);


struct tm *gmtime_r(const time_t *timep, struct tm *result)
{
	//TODO 64bit div by 2038
//	time_t days = *timep / SECONDS_PER_DAY + EPOCH_DAYS;
	int t = *timep & 0xffffffff;
	int days = t / SECONDS_PER_DAY + EPOCH_DAYS;
	result->tm_wday = (days + 1) % 7;
	result->tm_isdst = 0;

	int q = days / DAYS_PER_400YEARS;
	int d = days % DAYS_PER_400YEARS;
	int y = q * 400;

	q = d / DAYS_PER_100YEARS;
	d %= DAYS_PER_100YEARS;
	y += q * 100;

	q = d / DAYS_PER_4YEARS;
	d %= DAYS_PER_4YEARS;
	y += q * 4;

	q = d / DAYS_PER_YEAR;
	d %= DAYS_PER_YEAR;
	y += q + 1;

	result->tm_year = y - 1900;
	result->tm_yday = d;

	int *months = get_days_of_month(y);
	int m;
	for (m = 0; (m < 12) && (d > months[m]); m++)
		d -= months[m];

	result->tm_mon = m;
	result->tm_mday = d + 1;

	//TODO 64bit div by 2038
//	int s = *timep % SECONDS_PER_DAY;
	int s = t % SECONDS_PER_DAY;
	result->tm_hour = s / SECONDS_PER_HOUR;
	s %= SECONDS_PER_HOUR;

	result->tm_min = s / SECONDS_PER_MINUTE;
	s %= SECONDS_PER_MINUTE;

	result->tm_sec = s;

	return result;
}

static int *get_days_of_month(const int year)
{
	return days_of_month[
			(year % 4) ? 0 : ((year % 100) ?
					1 : ((year % 400) ? 0 : 1))];
}
