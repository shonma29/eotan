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
#include <stdint.h>
#include <mpu/bits.h>
#include <mpu/limits.h>
#include <sys/errno.h>
#include "func.h"

static int tfs_deallocate_block(vfs_t *fsp, const blkno_t block_no);
static bool is_valid_nth(const vfs_t *fsp, const unsigned int nth);

blkno_t tfs_allocate_block(vfs_t *fsp)
{
	struct sfs_superblock *sb = (struct sfs_superblock*)(fsp->private);
	if (!(sb->freeblock))
		return 0;

	unsigned int start = (sb->bsearch - 1) / (CHAR_BIT * sb->blksize);
	for (unsigned int i = start; i < sb->bitmapsize; i++) {
		uint32_t *buf = cache_get(&(fsp->device),
				i + TFS_RESERVED_BLOCKS);
		if (!buf)
			return 0;

		for (unsigned int j = (i == start) ?
				(((sb->bsearch - 1) / INT_BIT)
						% (sb->blksize / sizeof(j))) : 0;
				j < sb->blksize / sizeof(j); j++) {
			if (!buf[j])
				continue;

			unsigned int k = count_ntz(buf[j]);
			buf[j] &= ~(1 << k);
			if (!cache_release(buf, true))
				return 0;

			blkno_t block_no = i * sb->blksize * CHAR_BIT
					+ j * INT_BIT + k;
			sb->freeblock--;
			sb->bsearch = block_no;

			if (!cache_modify(fsp->private))
				return 0;

			return block_no;
		}

		if (!cache_release(buf, false))
			return 0;
	}

	return 0;
}

int tfs_deallocate_1st(vfs_t *fsp, struct sfs_inode *ip,
		const unsigned int index, const unsigned int offset)
{
	unsigned int pos = index;

	if (offset) {
		blkno_t *buf = cache_get(&(fsp->device), ip->i_indirect[pos]);
		if (!buf)
			return EIO;

		for (unsigned int i = offset;
				i < num_of_2nd_blocks(fsp->device.block_size);
				i++)
			if (buf[i]) {
				int error_no = tfs_deallocate_block(fsp,
						buf[i]);
				if (error_no)
					return error_no;

				buf[i] = 0;
			}

		if (!cache_release(buf, true))
			return EIO;

		pos++;
	}

	for (unsigned int i = pos;
			i < num_of_1st_blocks(fsp->device.block_size);
			i++) {
		if (!(ip->i_indirect[i]))
			continue;

		blkno_t *buf = cache_get(&(fsp->device), ip->i_indirect[i]);
		if (!buf)
			return EIO;

		for (unsigned int j = 0;
				j < num_of_2nd_blocks(fsp->device.block_size);
				j++)
			if (buf[j]) {
				int error_no = tfs_deallocate_block(fsp,
						buf[j]);
				if (error_no)
					return error_no;

				buf[j] = 0;
			}

		int error_no = tfs_deallocate_block(fsp, ip->i_indirect[i]);
		if (error_no)
			return error_no;

		ip->i_indirect[i] = 0;
	}

//	if (!cache_modify(ip->private))
//		return EIO;

	return 0;
}

static int tfs_deallocate_block(vfs_t *fsp, const blkno_t block_no)
{
	struct sfs_superblock *sb = (struct sfs_superblock*)(fsp->private);
	if (block_no < sb->datablock)
		return EINVAL;

	uint8_t *buf = cache_get(&(fsp->device),
			block_no / (CHAR_BIT * sb->blksize)
					+ TFS_RESERVED_BLOCKS);
	if (!buf)
		return EIO;

	buf[(block_no / CHAR_BIT) % sb->blksize] |=
			(1 << (block_no % CHAR_BIT)) & 0xff;

	if (!cache_release(buf, true))
		return EIO;

	if (!cache_invalidate(&(fsp->device), block_no))
		return EIO;

	sb->freeblock++;

	if ((sb->bsearch >= block_no)
			&& block_no)
		sb->bsearch = block_no - 1;

	if (!cache_modify(fsp->private))
		return EIO;

	return 0;
}

blkno_t tfs_get_block_no(vfs_t *fsp, const struct sfs_inode *ip,
		const unsigned int nth)
{
	if (!is_valid_nth(fsp, nth))
		return 0;

	size_t blocks = num_of_2nd_blocks(fsp->device.block_size);
	blkno_t block_no = ip->i_indirect[nth / blocks];
	if (!block_no)
		return 0;

	blkno_t *buf = cache_get(&(fsp->device), block_no);
	if (!buf)
		return 0;

	block_no = buf[nth % blocks];

	if (!cache_release(buf, false))
		return 0;

	return block_no;
}

blkno_t tfs_set_block_no(vfs_t *fsp, struct sfs_inode *ip,
		const unsigned int nth, const blkno_t new_block_no)
{
	if (!is_valid_nth(fsp, nth))
		return 0;

	if (!new_block_no)
		return 0;

	blkno_t *buf;
	size_t blocks = num_of_2nd_blocks(fsp->device.block_size);
	unsigned int index = nth / blocks;
	blkno_t block_no = ip->i_indirect[index];
	if (block_no)
		buf = cache_get(&(fsp->device), block_no);
	else {
		block_no = tfs_allocate_block(fsp);
		if (!block_no)
			return 0;

		ip->i_indirect[index] = block_no;
//		if (!cache_modify(ip->private))
//			return 0;

		buf = cache_create(&(fsp->device), block_no);
	}

	if (!buf)
		return 0;

	buf[nth % blocks] = new_block_no;

	if (!cache_release(buf, true))
		return 0;

	return new_block_no;
}

static bool is_valid_nth(const vfs_t *fsp, const unsigned int nth)
{
	return (nth < num_of_1st_blocks(fsp->device.block_size)
			* num_of_2nd_blocks(fsp->device.block_size));
}
