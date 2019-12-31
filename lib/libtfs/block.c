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
#include <sys/errno.h>
#include "func.h"

#define INT_BIT ((CHAR_BIT) * sizeof(int))

static size_t calc_blocks(const struct tfs *, const size_t);
static int shorten_blocks(vfs_t *, struct tfs_inode *, const size_t);
static int deallocate_2nd(vfs_t *, const size_t,
		const blkno_t, const unsigned int);
static bool is_valid_nth(const vfs_t *, const unsigned int);


blkno_t tfs_allocate_block(vfs_t *fsp)
{
	struct tfs *sb = (struct tfs *) (fsp->private);
	if (!(sb->fs_free_blocks))
		return 0;

	unsigned int start = (sb->fs_block_hand - 1) / (CHAR_BIT * sb->fs_bsize);
	int max = sb->fs_dblkno - sb->fs_sblkno - sb->fs_sbsize;
	for (unsigned int i = start; i < max; i++) {
		uint32_t *buf = cache_get(&(fsp->device),
				i + TFS_RESERVED_BLOCKS);
		if (!buf)
			return 0;

		for (unsigned int j = (i == start) ?
				(((sb->fs_block_hand - 1) / INT_BIT)
						% (sb->fs_bsize / sizeof(j))) : 0;
				j < sb->fs_bsize / sizeof(j); j++) {
			if (!buf[j])
				continue;

			unsigned int k = count_ntz(buf[j]);
			buf[j] &= ~(1 << k);
			if (!cache_release(buf, true))
				return 0;

			blkno_t block_no = i * sb->fs_bsize * CHAR_BIT
					+ j * INT_BIT + k;
			sb->fs_free_blocks--;
			sb->fs_block_hand = block_no;

			if (!cache_modify(fsp->private))
				return 0;

			return block_no;
		}

		if (!cache_release(buf, false))
			return 0;
	}

	return 0;
}

int tfs_shorten(vnode_t *vnode, const size_t size)
{
	vfs_t *fs = vnode->fs;
	struct tfs *tfs = (struct tfs *) (fs->private);
	size_t blocks = calc_blocks(tfs, size);
	if (blocks < calc_blocks(tfs, vnode->size)) {
		int error_no = shorten_blocks(fs,
				(struct tfs_inode *) vnode->private, blocks);
		if (error_no)
			return error_no;
	}

	vnode->size = size;
	return 0;
}

static size_t calc_blocks(const struct tfs *tfs, const size_t size)
{
	return ((size + tfs->fs_bsize - 1) >> tfs->fs_bshift);
}

static int shorten_blocks(vfs_t *fs, struct tfs_inode *inode,
		const size_t blocks)
{
	size_t base = num_of_2nd_blocks(fs->device.block_size);
	unsigned int index = blocks / base;
	unsigned int offset = blocks % base;
	if (offset) {
		int error_no = deallocate_2nd(fs, base, inode->i_ib[index],
				offset);
		if (error_no)
			return error_no;

		index++;
	}

	for (; index < num_of_1st_blocks(fs->device.block_size); index++) {
		if (!(inode->i_ib[index]))
			break;

		int error_no = deallocate_2nd(fs, base, inode->i_ib[index], 0);
		if (error_no)
			return error_no;

		error_no = tfs_deallocate_block(fs, inode->i_ib[index]);
		if (error_no)
			return error_no;

		inode->i_ib[index] = 0;
	}

//	if (!cache_modify(inode->private))
//		return EIO;

	return 0;
}

static int deallocate_2nd(vfs_t *fs, const size_t base,
		const blkno_t index, const unsigned int offset)
{
	blkno_t *buf = cache_get(&(fs->device), index);
	if (!buf)
		return EIO;

	for (unsigned int i = offset; (i < base) && buf[i]; i++) {
		int error_no = tfs_deallocate_block(fs, buf[i]);
		if (error_no)
			return error_no;

		buf[i] = 0;
	}

	if (!cache_release(buf, true))
		return EIO;

	return 0;
}

int tfs_deallocate_block(vfs_t *fsp, const blkno_t block_no)
{
	struct tfs *sb = (struct tfs *) (fsp->private);
	if (block_no < sb->fs_dblkno)
		return EINVAL;

	uint8_t *buf = cache_get(&(fsp->device),
			block_no / (CHAR_BIT * sb->fs_bsize)
					+ TFS_RESERVED_BLOCKS);
	if (!buf)
		return EIO;

	buf[(block_no / CHAR_BIT) % sb->fs_bsize] |=
			(1 << (block_no % CHAR_BIT)) & 0xff;

	if (!cache_release(buf, true))
		return EIO;

	if (!cache_invalidate(&(fsp->device), block_no))
		return EIO;

	sb->fs_free_blocks++;

	if ((sb->fs_block_hand >= block_no)
			&& block_no)
		sb->fs_block_hand = block_no - 1;

	if (!cache_modify(fsp->private))
		return EIO;

	return 0;
}

blkno_t tfs_get_block_no(vfs_t *fsp, const struct tfs_inode *ip,
		const unsigned int nth)
{
	if (!is_valid_nth(fsp, nth))
		return 0;

	size_t blocks = num_of_2nd_blocks(fsp->device.block_size);
	blkno_t block_no = ip->i_ib[nth / blocks];
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

blkno_t tfs_set_block_no(vfs_t *fsp, struct tfs_inode *ip,
		const unsigned int nth, const blkno_t new_block_no)
{
	if (!is_valid_nth(fsp, nth))
		return 0;

	if (!new_block_no)
		return 0;

	blkno_t *buf;
	size_t blocks = num_of_2nd_blocks(fsp->device.block_size);
	unsigned int index = nth / blocks;
	blkno_t block_no = ip->i_ib[index];
	if (block_no)
		buf = cache_get(&(fsp->device), block_no);
	else {
		block_no = tfs_allocate_block(fsp);
		if (!block_no)
			return 0;

		ip->i_ib[index] = block_no;
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

int tfs_allocate_inode(vfs_t *fs, vnode_t *vnode)
{
	blkno_t block_no = tfs_allocate_block(fs);
	if (!block_no)
		return 0;

	struct tfs_inode *buf = cache_get(&(fs->device), block_no);
	if (!buf)
		return EIO;

	fs->device.clear(&(fs->device), buf);
	buf->i_inumber = block_no;

	if (!cache_modify(buf)) {
		cache_release(buf, false);
		return EIO;
	}

	vnode->private = buf;
	return block_no;
}

int tfs_deallocate_inode(vfs_t *fs, vnode_t *vnode)
{
	blkno_t block_no = vnode->index;

	//TODO private may not be initialized
	int error_no = tfs_shorten(vnode, 0);
	if (error_no)
		return error_no;

	vnode->dirty = true;
	fs->device.clear(&(fs->device), vnode->private);

	if (!cache_modify(vnode->private))
		return EIO;

	vnode->dirty = false;
	return tfs_deallocate_block(fs, block_no);
}
