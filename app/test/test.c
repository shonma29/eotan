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
#include <local.h>
#include <math.h>
#include <stdio.h>


int main(int argc, char **argv)
{
	double x = 1.0;

	printf("%f\n", 0.0175);
	printf("%f\n", 0.175);
	printf("%f\n", 1.75);
	printf("%f\n", -17.5);
	printf("%f\n", sin(x));
	printf("%f\n", cos(x));

	int i = 1;
	int j = 0;
	printf("div %d\n", i / j);

	thread_local_t *local = _get_thread_local();
	printf("thread_local=%p, error_no=%d, thread_id=%d\n",
			local, local->error_no, local->thread_id);

	return 0;
}
