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
#include "func.h"

static ER _thread_delay(svc_arg *);
static ER_UINT _port_call(svc_arg *);

static ER (*svc_entries[])(svc_arg *) = {
	_thread_delay,
	_port_call
};


ER syscall(svc_arg *argp, UW svcno)
{
	return (svcno >= sizeof(svc_entries) / sizeof(svc_entries[0]))?
			E_NOSPT:(svc_entries[svcno](argp));
}

static ER _thread_delay(svc_arg *argp)
{
	return thread_delay((RELTIM)(argp->arg1));
}

static ER_UINT _port_call(svc_arg *argp)
{
	return port_call((ID)(argp->arg1), (VP)(argp->arg3), (UINT)(argp->arg4));
}
