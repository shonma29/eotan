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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "../../include/fs/config.h"
#include "../../include/fs/tfs.h"
#include "../../include/mpu/bits.h"
#include "../../include/mpu/limits.h"

#define ERR_ARG (1)
#define ERR_MEMORY (2)
#define ERR_FILE (3)
#define ERR_UNKNOWN (4)

#define MIN_BLOCK_SIZE (512)

// inode blocks
#define ROOT_BLOCK_NUM (1)

static uint8_t *buf;
static struct tfs super;
static struct tfs_inode root_inode = {
	S_IFDIR | S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH,
	0,
	0,
	0, 0,
	0, 0,
	0, 0,
	ROOT_UID,
	ROOT_GID,
};

static int usage(void);
static int calc_super_block(const int, const int);
static int skip_boot_block(FILE *);
static int write_super_block(FILE *);
static int write_bitmap(FILE *);
static int write_inodes(FILE *);


static int usage(void)
{
	fprintf(stderr, "usage: mkfs "
			"file_name block_size block_num\n");
	return ERR_ARG;
}

static int calc_super_block(const int block_size, const int block_num)
{
	int bshift = count_ntz(block_size);
	if ((1 << bshift) != block_size) {
		fprintf(stderr, "bad block size %d\n", block_size);
		return ERR_ARG;
	}

	int bits_per_block = block_size * CHAR_BIT;
	int bitmap_block_num = (block_num + bits_per_block - 1)
			/ bits_per_block;
	int free_block_num = block_num - TFS_RESERVED_BLOCKS - bitmap_block_num;

	// root inode and blocks should be allocated
	if (free_block_num < ROOT_BLOCK_NUM) {
		fprintf(stderr, "too small block_num\n");
		return ERR_ARG;
	}

	printf("superblock = %d, bitmap block = %d, free block = %d\n",
			TFS_SUPER_BLOCKS, bitmap_block_num, free_block_num);

	super.fs_sblkno = TFS_BOOT_BLOCKS;
	super.fs_sbsize = TFS_SUPER_BLOCKS;
	super.fs_dblkno = super.fs_sblkno + super.fs_sbsize + bitmap_block_num;
	super.fs_bsize = block_size;
	super.fs_bmask = (1 << bshift) - 1;
	super.fs_bshift = bshift;
	super.fs_size = block_num;
	super.fs_dsize = block_num - super.fs_dblkno;
	super.fs_maxfilesize = 1 << (bshift * 3 - count_ntz(sizeof(int)) * 2);
	super.fs_qbmask = super.fs_bmask;
	super.fs_magic = TFS_MAGIC;

	// root inode and blocks are allocated
	super.fs_free_blocks = super.fs_dsize - ROOT_BLOCK_NUM;
	super.fs_block_hand = super.fs_dblkno + ROOT_BLOCK_NUM - 1;

	return 0;
}

static int skip_boot_block(FILE *fp)
{
	// boot block occupies 1 sector
	if (fseek(fp, super.fs_bsize, SEEK_SET)) {
		perror("fseek failed in skip_boot_block\n");
		return ERR_FILE;
	}

	return 0;
}

static int write_super_block(FILE *fp)
{
	// super block occupies 1 sector
	memset(buf, 0, super.fs_bsize);
	memcpy(buf, &super, sizeof(super));

	if (fwrite(buf, super.fs_bsize, 1, fp) != 1) {
		perror("fwrite failed in write_super_block\n");
		return ERR_FILE;
	}

	return 0;
}

static int write_bitmap(FILE *fp)
{
	memset(buf, 0, super.fs_bsize);

	int edge_bit = super.fs_dblkno + ROOT_BLOCK_NUM;
	int edge_byte = edge_bit / CHAR_BIT;
	int edge_block = edge_byte / super.fs_bsize;
	int i = 0;

	while (i++ < edge_block)
		if (fwrite(buf, super.fs_bsize, 1, fp) != 1) {
			perror("fwrite failed in write_bitmap\n");
			return ERR_FILE;
		}

	edge_byte %= super.fs_bsize;
	buf[edge_byte] = ~((1 << edge_bit % CHAR_BIT) - 1);
	memset(&(buf[edge_byte + 1]), 0xff, super.fs_bsize - edge_byte - 1);

	if (fwrite(buf, super.fs_bsize, 1, fp) != 1) {
		perror("fwrite failed in write_bitmap\n");
		return ERR_FILE;
	}

	memset(buf, 0xff, super.fs_bsize);

	size_t max = super.fs_dblkno - super.fs_sblkno - super.fs_sbsize;
	while (++i < max)
		if (fwrite(buf, super.fs_bsize, 1, fp) != 1) {
			perror("fwrite failed in write_bitmap\n");
			return ERR_FILE;
		}

	return 0;
}

static int write_inodes(FILE *fp)
{
	// inode occupies 1 sector
	root_inode.i_inumber += super.fs_dblkno;

	time_t t = time(NULL);
	root_inode.i_atime = t;
	root_inode.i_mtime = t;
	root_inode.i_ctime = t;

	memset(buf, 0, super.fs_bsize);
	memcpy(buf, &root_inode, sizeof(root_inode));

	if (fwrite(buf, super.fs_bsize, 1, fp) != 1) {
		perror("fwrite failed in write_inodes\n");
		return ERR_FILE;
	}

	return 0;
}

int main(int argc, char **argv)
{
	if (argc != 4)
		return usage();

	int block_size = atoi(argv[2]);
	if (block_size < MIN_BLOCK_SIZE) {
		fprintf(stderr, "bad block_size\n");
		return ERR_ARG;
	}

	int block_num = atoi(argv[3]);
	if (block_num <= 0) {
		fprintf(stderr, "bad block_num\n");
		return ERR_ARG;
	}

	int error = 0;
	error = calc_super_block(block_size, block_num);
	if (error)
		return error;

	buf = malloc(block_size);
	if (!buf) {
		perror("malloc failed\n");
		return ERR_MEMORY;
	}

	FILE *fp = fopen(argv[1], "w");
	if (!fp) {
		perror("fopen failed\n");
		free(buf);
		return ERR_FILE;
	}

	do {
		error = skip_boot_block(fp);
		if (error)
			break;

		error = write_super_block(fp);
		if (error)
			break;

		error = write_bitmap(fp);
		if (error)
			break;

		error = write_inodes(fp);
		if (error)
			break;
	} while (false);

	fclose(fp);
	free(buf);

	return error;
}
