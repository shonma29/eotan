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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BUF_HEIGHT (8)
#define TITLE_HEIGHT (2)
#define LINE_WIDTH_1MONTH (21)

#define JULIAN_LAST_YEAR (1752)
#define JULIAN_LAST_MONTH (9)
#define JULIAN_LAST_DAY (2)
#define GREGORIAN_FIRST_DAY (14)

#define WEEKDAY_FIRST (0)
#define WEEKDAY_LAST (6)

#define MSG_ARG "usage: cal [[month] year]\n"

typedef struct {
	// current month. (1 .. 12)
	int month;
	// current day of month. (1 .. 31)
	int day;
	// last day of month. (1 .. 31)
	int last_day;
	// weekday of 1st day. (0 .. 6)
	int weekday;
} calendar_t;

static int days_of_month[2][12] = {
	{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
	{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};
static char *month_names[] = {
	"January", "February", "March", "April", "May", "June",
	"July", "August", "September", "October", "November", "December"
};
static char *weekday_row = " S  M Tu  W Th  F  S";

static bool gregorian_is_leap(const int);
static bool julian_is_leap(const int);
static int gregorian_gamma(const int);
static int julian_gamma(const int);
static int get_weekday(const int, const int, const int, int (*)(const int));
static void show(const int, const int);
static int get_current_month(int *, int *);
static int parse_natural_number(int *, const char *, const int);


static bool gregorian_is_leap(const int year)
{
	return ((year & 3) ? false : ((year % 100) ? true : (!(year % 400))));
}

static bool julian_is_leap(const int year)
{
	return !(year & 3);
}

static int gregorian_gamma(const int year)
{
	int century = year / 100;
	return ((century >> 2) - century);
}

static int julian_gamma(const int year)
{
	return 5;
}

// Zeller's congruence
// 0: sunday, 1: monday, .., 6: saturday
static int get_weekday(const int year, const int month, const int day,
		int (*gamma)(const int))
{
	int y = year;
	int q = month;
	switch (month) {
	case 1:
	case 2:
		y--;
		q += 12;
		break;
	default:
		break;
	}

	int h = (day + (13 * (q + 1) / 5) + y + (y >> 2) + gamma(y)) % 7;
	return (h ? (h - 1) : 6);
}

static void show(const int year, const int month)
{
	bool (*is_leap)(const int);
	int (*get_gamma)(const int);
	if ((year < JULIAN_LAST_YEAR)
			|| ((year == JULIAN_LAST_YEAR)
					&& (month <= JULIAN_LAST_MONTH))) {
		is_leap = julian_is_leap;
		get_gamma = julian_gamma;
	} else {
		is_leap = gregorian_is_leap;
		get_gamma = gregorian_gamma;
	}

	//TODO show yearly
	//TODO use line buffer
	calendar_t c;
	c.month = month;
	c.day = 1;
	c.weekday = get_weekday(year, c.month, c.day, get_gamma);

	if ((year == JULIAN_LAST_YEAR)
			&& (c.month == JULIAN_LAST_MONTH)) {
		c.last_day = JULIAN_LAST_DAY;
	} else
		c.last_day = days_of_month[is_leap(year) ? 1 : 0][c.month - 1];

	int len = strlen(month_names[c.month - 1]) + 1;
	if (year >= 1000)
		len += 4;
	else if (year >= 100)
		len += 3;
	else if (year >= 10)
		len += 2;
	else
		len += 1;

	for (int i = 0; i < (((LINE_WIDTH_1MONTH - 1 - len) >> 1)); i++)
		printf(" ");

	printf("%s %d\n", month_names[c.month - 1], year);
	printf("%s\n", weekday_row);
	for (int i = 0; i < c.weekday; i++)
		printf("   ");

	int row = 0;
	for (;;) {
		if (c.day < 10)
			printf(" ");

		printf("%d", c.day);
		c.day++;
		if (c.day > c.last_day) {
			if (c.last_day == JULIAN_LAST_DAY) {
				c.day = GREGORIAN_FIRST_DAY;
				c.last_day = days_of_month[1][c.month - 1];
			} else {
				printf("\n");
				row++;
				break;
			}
		}

		if (c.weekday == WEEKDAY_LAST) {
			c.weekday = WEEKDAY_FIRST;
			printf("\n");
			row++;
		} else {
			printf(" ");
			c.weekday++;
		}
	}

	for (; row < BUF_HEIGHT - TITLE_HEIGHT; row++)
		printf("\n");
}

static int get_current_month(int *year, int *month)
{
	time_t t = time(NULL);
	if (t == -1)
		return (-1);

	struct tm tm;
	//TODO use localtime
	if (!gmtime_r(&t, &tm))
		//TODO show error, while it does not ocuur
		return (-1);

	*year = tm.tm_year + 1900;
	*month = tm.tm_mon + 1;
	return 0;
}

static int parse_natural_number(int *num, const char *arg, const int max)
{
	errno = 0;

	char *end;
	int value = strtol(arg, &end, 10);
	if (errno)
		//TODO show error
		return (-1);

	if (arg[0] && !(*end)) {
		if (value <= 0)
			return (-1);

		if (value > max)
			return (-1);

		*num = value;
		return 0;
	}

	return (-1);
}

int main(int argc, char **argv)
{
	//TODO support year only
	//TODO support month only (plan9)
	int year;
	int month;
	switch (argc) {
	case 1:
		if (get_current_month(&year, &month)) {
			printf(MSG_ARG);
			return EXIT_FAILURE;
		}

		break;
	case 3:
		if (parse_natural_number(&month, argv[1], 12)) {
			printf(MSG_ARG);
			return EXIT_FAILURE;
		}

		if (parse_natural_number(&year, argv[2], 9999)) {
			printf(MSG_ARG);
			return EXIT_FAILURE;
		}

		break;
	default:
		printf(MSG_ARG);
		return EXIT_FAILURE;
	}

	show(year, month);
	return EXIT_SUCCESS;
}
