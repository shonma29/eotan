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
#ifndef _FS_VFSFUNCS_H_
#define _FS_VFSFUNCS_H_

#include "vfs.h"

static inline struct inode *getINodeParent(const list_t *p) {
	return (struct inode*)((intptr_t)p - offsetof(struct inode, bros));
}

static inline struct fs *getFsParent(const list_t *p) {
	return (struct fs*)((intptr_t)p - offsetof(struct fs, bros));
}

static inline W fs_sync_file(struct inode *ip)
{
	return OPS(ip).sync(ip, 0);
}

static inline W fs_getdents(struct inode *ip, ID caller, W offset,
		VP buf, UW length, W * rsize, W * fsize)
{
	return OPS(ip).getdents(ip, caller, offset, buf, length, rsize, fsize);
}

#endif