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
#include <services.h>
#include <string.h>
#include <nerve/kcall.h>
#include <sys/syscall.h>


ER bind_device(UW id, UB *name, ID port, UW size)
{
	ER_UINT err;
	struct posix_request req;
	struct posix_response *res = (struct posix_response *)&req;
	kcall_t *kcall = (kcall_t*)KCALL_ADDR;

	if (strlen((char*)name) > MAX_DEVICE_NAME)
		return E_PAR;

	req.operation = PSC_BIND_DEVICE;
	req.param.par_bind_device.id = id;
	req.param.par_bind_device.name = name;
	req.param.par_bind_device.port = port;
	req.param.par_bind_device.size = size;
	err = kcall->port_call(PORT_FS, &req, sizeof(req));

	return (err < 0)? err:(res->error_no? E_SYS:E_OK);
}
