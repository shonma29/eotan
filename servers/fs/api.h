#ifndef _FS_API_H_
#define _FS_API_H_
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
#include <device.h>
#include <sys/syscall.h>
#include <sys/syslimits.h>
#include <sys/types.h>

typedef struct {
	pm_args_t packet;
	RDVNO rdvno;
	char buf[PATH_MAX + 1];
} fs_request;

extern int if_attach(fs_request*);
extern int if_walk(fs_request*);
extern int if_open(fs_request*);
extern int if_create(fs_request*);
extern int if_read(fs_request*);
extern int if_write(fs_request*);
extern int if_clunk(fs_request*);
extern int if_remove(fs_request*);
extern int if_stat(fs_request*);
extern int if_wstat(fs_request*);


static inline int unpack_sid(const fs_request *req)
{
	return (req->packet.process_id & 0xffff);
}

static inline int unpack_tid(const fs_request *req)
{
	return ((req->packet.process_id >> 16) & 0xffff);
}

//TODO exclude mpu dependency
extern int reply2(const RDVNO, const int32_t, const int32_t,
		const int32_t);
extern int reply_dev(const RDVNO, const devmsg_t *, const size_t);
extern int reply_dev_error(const RDVNO, const int, const int);

#endif
