#ifndef _FS_VFS_H_
#define _FS_VFS_H_
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

typedef struct _block_device_t {
	int channel;
	size_t block_size;
	int (*read)(struct _block_device_t *, void *, int);
	int (*write)(struct _block_device_t *, void *, int);
	int (*invalidate)(struct _block_device_t *, int);
} block_device_t;

typedef struct {
	int (*mount)();
	int (*unmount)();
	int (*sync)();
	int (*statvfs)();
	int (*getdents)();
	int (*lookup)();
	int (*link)();
	int (*unlink)();
	int (*mkdir)();
	int (*rmdir)();
	int (*stat)();
	int (*wstat)();
	int (*permit)();
	int (*create)();
	int (*close)();
	int (*read)();
	int (*write)();
} vfs_operation_t;

typedef struct {
	block_device_t *device;
	vfs_operation_t *operation;
} vfs_t;

extern void block_initialize(block_device_t *);

#endif
