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
#include <core.h>
#include <sys/time.h>
#include "func.h"
#include "setting.h"

#define TICK (1000 * 1000 * 1000 / TIME_TICKS)

static struct timespec system_time;


void time_initialize(UW seconds)
{
	time_t sec = seconds;
	long nsec = 0;

	timespec_set(&system_time, &sec, &nsec);
}

ER time_set(SYSTIM *pk_systim)
{
	if (!pk_systim)
		return E_PAR;

	timespec_set(&system_time, &(pk_systim->sec), &(pk_systim->nsec));

	return E_OK;
}

ER time_get(SYSTIM *pk_systim)
{
	struct timespec t1;
	struct timespec t2;

	if (!pk_systim)
		return E_PAR;

	do {
		t1 = system_time;
		t2 = system_time;
	} while (!timespec_equals(&t1, &t2));

	timespec_get_sec(&(pk_systim->sec), &t1);
	timespec_get_nsec(&(pk_systim->nsec), &t1);

	return E_OK;
}

void time_tick(void)
{
	struct timespec add = {
		0, TICK
	};

	timespec_add(&system_time, &add);
}
