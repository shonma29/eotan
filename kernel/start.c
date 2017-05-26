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
#include <fstype.h>
#include <major.h>
#include <nerve/global.h>
#include "func.h"
#include "ready.h"
#include "arch/archfunc.h"
#include "mpu/mpufunc.h"


void kern_start(void (*callback)(void))
{
	sysinfo->root.device = get_device_id(DEVICE_MAJOR_ATA, 0);
	sysinfo->root.fstype = FS_SFS;
	sysinfo->initrd.start = 0;
	sysinfo->initrd.size = 0;
	sysinfo->delay_thread_start = FALSE;
	sysinfo->delay_thread_id = TSK_NONE;

	context_initialize();
	kcall_initialize();
	service_initialize();
	port_initialize();
	mutex_initialize();
	thread_initialize();

	//callback();
	load_modules();

	for (;;)
		halt();
}
