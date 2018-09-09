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
#include <string.h>
#include <fs/sfs.h>
#include <sys/errno.h>
#include "func.h"


//TODO use off_t
int tfs_read(vnode_t *ip, char *dest, int offset, size_t nbytes,
		size_t *read_len)
{
	*read_len = nbytes;

	vfs_t *fs = ip->fs;
	size_t block_size = ((struct sfs_superblock*)(fs->private))->blksize;
	uint32_t skip = offset % block_size;
	for (offset /= block_size; nbytes > 0; offset++) {
		size_t size = block_size - skip;
		if (size > nbytes)
				size = nbytes;

		blkno_t block_no = tfs_get_block_no(fs, ip->private, offset);
		if (!block_no)
			return (-EIO);

		uint8_t *src = cache_get(&(fs->device), block_no);
		if (!src)
			return (-EIO);

		memcpy(dest, &src[skip], size);
		if (!cache_release(src, false))
			return (-EIO);

		skip = 0;
		dest += size;
		nbytes -= size;
	}

	return *read_len;
}
