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

//TOOD test
#define STRICT_CHECK 0

#define ISO8601_FORMAT "%Y-%m-%d"

#define MAX_DECIMAL_COLUMN (20)
#define ERR (-1)

//TODO split locale
typedef struct {
	char *month_names[12];
	char *month_abbrevs[12];
	char *wday_names[7];
	char *wday_abbrevs[7];
	char *am_pm[2];
	char *date_time_format;
	char *date_format;
	char *time_format;
	char *time_zone_name;
} locale_time_t;

static locale_time_t loc = {
	{
		"January",
		"February",
		"March",
		"April",
		"May",
		"June",
		"July",
		"August",
		"September",
		"October",
		"November",
		"December"
	},
	{
		"Jan",
		"Feb",
		"Mar",
		"Apr",
		"May",
		"Jun",
		"Jul",
		"Aug",
		"Sep",
		"Oct",
		"Nov",
		"Dec"
	},
	{
		"Sunday",
		"Monday",
		"Tuesday",
		"Wednesday",
		"Thursday",
		"Friday",
		"Saturday"
	},
	{
		"Sun",
		"Mon",
		"Tue",
		"Wed",
		"Thu",
		"Fri",
		"Sat"
	},
	{
		"AM",
		"PM",
	},
	"%a %b %d %H:%M:%S %Y",
	"%m/%d/%y",
	"%H:%M:%S",
	"GMT"
};

typedef struct {
	int pos;
	size_t size;
	char *buf;
} strbuf_t;


static int _putc(const int c, strbuf_t *s)
{
	if ((s->pos < 0)
			|| (s->pos >= s->size))
		return ERR;

	s->buf[s->pos] = (char)(c & 0xff);
	s->pos++;

	return c;
}

static int _puts(const char *str, strbuf_t *s)
{
	for (char ch, *p = (char*)str; (ch = *p); p++)
		if (_putc(ch, s) == ERR)
			return ERR;

	return 0;
}

static int _putzd(const int num, const size_t column, strbuf_t *s)
{
#if STRICT_CHECK
	if (num < 0)
		return ERR;
#endif
	char dec[MAX_DECIMAL_COLUMN + 1];
	char *p = &dec[sizeof(dec) - 1];
	*p = '\0';

	int n = (int)num;
	for (size_t rest = (size_t)column; rest > 0; rest--) {
		p--;
#if STRICT_CHECK
		if (p < dec)
			return ERR;
#endif
		*p = n % 10 + '0';
		n /= 10;
	}

	return _puts(p, s);
}

static int _putd(const int num, strbuf_t *s)
{
	char dec[MAX_DECIMAL_COLUMN + 1];
	char *p = &dec[sizeof(dec) - 1];
	*p = '\0';

	int n = (int)num;
	for (;;) {
		p--;
#if STRICT_CHECK
		if (p < dec)
			return ERR;
#endif
		*p = n % 10 + '0';
		if (!(n /= 10))
			break;
	}

	return _puts(p, s);
}

#if STRICT_CHECK
static bool is_valid_wday(const int wday)
{
	return ((wday >= 0)
			|| (wday < 7));
}

static bool is_valid_month(const int month)
{
	return ((month >= 0)
			|| (month < 12));
}

static bool is_valid_hour(const int hour)
{
	return ((hour >= 0)
			|| (hour < 24));
}

static bool is_valid_yday(const int yday)
{
	return ((yday >= 0)
			|| (yday < 366));
}
#endif
/*
static int calc_weeks(const int yday, const int wday, const int offset)
{
	return ((yday + (7 - (yday + (7 - wday) + offset) % 7)) / 7);
}
*/
static int calc_weeks(const struct tm *tm, const int offset)
{
	return ((tm->tm_yday
			+ (7 - (tm->tm_yday + (7 - tm->tm_wday) + offset) % 7))
			/ 7);
}

static size_t _printf(strbuf_t *s, const char *format, const struct tm *tm)
{
	char *r = (char*)format;
	bool conversion = false;

	for (char c; (c = *r); r++) {
		if (conversion) {
			conversion = false;
			switch (c) {
			case 'a':
#if STRICT_CHECK
				if (!is_valid_wday(tm->tm_wday))
					return ERR;
#endif
				if (_puts(loc.wday_abbrevs[tm->tm_wday], s))
					return ERR;
				break;
			case 'A':
#if STRICT_CHECK
				if (!is_valid_wday(tm->tm_wday))
					return ERR;
#endif
				if (_puts(loc.wday_names[tm->tm_wday], s))
					return ERR;
				break;
			case 'b':
#if STRICT_CHECK
				if (!is_valid_month(tm->tm_mon))
					return ERR;
#endif
				if (_puts(loc.month_abbrevs[tm->tm_mon], s))
					return ERR;
				break;
			case 'B':
#if STRICT_CHECK
				if (!is_valid_month(tm->tm_mon))
					return ERR;
#endif
				if (_puts(loc.month_names[tm->tm_mon], s))
					return ERR;
				break;
			case 'c':
				if (_printf(s, loc.date_time_format, tm))
					return ERR;
				break;
			case 'd':
				if (_putzd(tm->tm_mday, 2, s))
					return ERR;
				break;
			case 'F':
				if (_printf(s, ISO8601_FORMAT, tm))
					return ERR;
				break;
			case 'H':
				if (_putzd(tm->tm_hour, 2, s))
					return ERR;
				break;
			case 'I':
				if (_putzd(tm->tm_hour % 12, 2, s))
					return ERR;
				break;
			case 'j':
				if (_putzd(tm->tm_yday + 1, 3, s))
					return ERR;
				break;
			case 'm':
				if (_putzd(tm->tm_mon + 1, 2, s))
					return ERR;
				break;
			case 'M':
				if (_putzd(tm->tm_min, 2, s))
					return ERR;
				break;
			case 'p':
#if STRICT_CHECK
				if (!is_valid_hour(tm->tm_hour))
					return ERR;
#endif
				if (_puts(loc.am_pm[tm->tm_hour / 12], s))
					return ERR;
				break;
			case 'S':
				if (_putzd(tm->tm_sec, 2, s))
					return ERR;
				break;
			case 'U':
#if STRICT_CHECK
				if (!is_valid_yday(tm->tm_yday))
					return ERR;
				if (!is_valid_wday(tm->tm_wday))
					return ERR;
#endif
				if (_putzd(calc_weeks(tm, 0), 2, s))
					return ERR;
				break;
			case 'W':
#if STRICT_CHECK
				if (!is_valid_yday(tm->tm_yday))
					return ERR;
				if (!is_valid_wday(tm->tm_wday))
					return ERR;
#endif
				if (_putzd(calc_weeks(tm, 1), 2, s))
					return ERR;
				break;
			case 'x':
				if (_printf(s, loc.date_format, tm))
					return ERR;
				break;
			case 'X':
				if (_printf(s, loc.time_format, tm))
					return ERR;
				break;
			case 'y':
				if (_putzd(tm->tm_year % 100, 2, s))
					return ERR;
				break;
			case 'Y':
				if (_putd(tm->tm_year + 1900, s))
					return ERR;
				break;
			case 'Z':
				if (_puts(loc.time_zone_name, s))
					return ERR;
				break;
			case '%':
				if (_puts("%", s))
					return ERR;
				break;
			default:
				return ERR;
			}
		} else {
			switch (c) {
			case '%':
				conversion = true;
				break;
			default:
				if (_putc(c, s) == ERR)
					return ERR;
				break;
			}
		}
	}

	return 0;
}

size_t strftime(char *buf, size_t maxsize, const char *format,
		const struct tm *timeptr)
{
	strbuf_t s = { 0, maxsize, buf };
	if (_printf(&s, format, timeptr) == ERR)
		return 0;

	if (s.pos >= s.size)
		return 0;

	s.buf[s.pos] = '\0';
	return s.pos;
}
