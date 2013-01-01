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
#include <stdlib.h>
#include <itron/types.h>
#include <itron/rendezvous.h>
#include <itron/errno.h>

#define TEST_PORT 49157

int main(int argc, char **argv)
{
	unsigned char buf[8];
	ER_UINT size;

	printf("Hello World!\n");

	buf[0] = 'H';
	buf[1] = 'e';
	buf[2] = 'l';
	buf[3] = 'l';
	buf[4] = 'o';

	size = cal_por(TEST_PORT, 1, buf, 5);
	printf("test_cal_por_1: result=%d\n", size);
	if (size != E_NOSPT)	return -1;

	size = cal_por(0, 0xffffffff, buf, 5);
	printf("test_cal_por_2: result=%d\n", size);
	if (size != E_NOEXS)	return -1;

	size = cal_por(TEST_PORT, 0xffffffff, buf, 33);
	printf("test_cal_por_3: result=%d\n", size);
	if (size != E_PAR)	return -1;

	size = cal_por(TEST_PORT, 0xffffffff, 0, 5);
	printf("test_cal_por_4: result=%d\n", size);
	if (size != E_PAR)	return -1;

	size = cal_por(TEST_PORT, 0xffffffff, buf, 5);
	printf("test_cal_por_5: result=%d\n", size);
	if (size != 4)	return -1;

	buf[size] = 0;
	printf("buf=%s\n", buf);

	printf("success\n");
	return 0;
}
