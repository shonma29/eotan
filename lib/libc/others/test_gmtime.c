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
#include <stdio.h>
#include <string.h>
#include <time.h>

extern struct tm *gmtime_(const time_t *timep);

int main(int argc, char **argv)
{
//	time_t t = time(NULL);
	time_t t = 0;
	size_t i;
	size_t errCnt = 0;

	for (i = 0; i < 365 * 10000; i++) {
		struct tm tm1 = *gmtime(&t);
		struct tm tm2 = *gmtime_(&t);

		tm2.tm_gmtoff = tm1.tm_gmtoff;
		tm2.tm_zone = tm1.tm_zone;

		if (memcmp(&tm1, &tm2, sizeof(tm1))) {
printf("------\n");
			printf("%d %d\n", tm1.tm_sec, tm2.tm_sec);
			printf("%d %d\n", tm1.tm_min, tm2.tm_min);
			printf("%d %d\n", tm1.tm_hour, tm2.tm_hour);
			printf("%d %d\n", tm1.tm_mday, tm2.tm_mday);
			printf("%d %d\n", tm1.tm_mon, tm2.tm_mon);
			printf("%d %d\n", tm1.tm_year, tm2.tm_year);
			printf("%d %d\n", tm1.tm_wday, tm2.tm_wday);
			printf("%d %d\n", tm1.tm_yday, tm2.tm_yday);
			printf("%d %d\n", tm1.tm_isdst, tm2.tm_isdst);
			printf("%ld %ld\n", tm1.tm_gmtoff, tm2.tm_gmtoff);
			printf("%s %s\n", tm1.tm_zone, tm2.tm_zone);
if (++errCnt >= 10)
			break;
		}

		t += 86393;
	}

	return 0;
}
#endif
