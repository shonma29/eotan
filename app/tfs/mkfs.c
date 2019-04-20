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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "../../include/fs/nconfig.h"
#include "../../include/fs/sfs.h"
#include "../../include/mpu/limits.h"

#define ERR_ARG (1)
#define ERR_MEMORY (2)
#define ERR_FILE (3)
#define ERR_UNKNOWN (4)

#define MIN_BLOCK_SIZE (512)

#define ROOT_INODE_INDEX (1)

// index and body blocks
#define ROOT_BLOCK_NUM (1 + 1)

// entries in root directory
#define CURRENT_NAME ".\0"
#define PARENT_NAME ".."

static uint8_t *buf;
static struct sfs_superblock super;
static struct tfs_dir root_directory[] = {
	{ ROOT_INODE_INDEX, strlen(CURRENT_NAME), CURRENT_NAME },
	{ ROOT_INODE_INDEX, strlen(PARENT_NAME), PARENT_NAME }
};
static struct sfs_inode root_inode = {
	ROOT_INODE_INDEX,
	sizeof(root_directory) / sizeof(root_directory[0]),
	sizeof(root_directory),
	1,
	S_IFDIR | S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH,
	ROOT_UID,
	ROOT_GID,
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
};

static int usage(void);
static int calc_super_block(const int block_size, const int block_num,
		const int kb_per_inode);
static int skip_boot_block(FILE *fp);
static int write_super_block(FILE *fp);
static int write_bitmap(FILE *fp);
static int write_inodes(FILE *fp);
static int write_index(FILE *fp);
static int write_directory(FILE *fp);


static int usage(void)
{
	fprintf(stderr, "usage: mkfs "
			"file_name block_size block_num kb_per_inode\n");
	return ERR_ARG;
}

static int calc_super_block(const int block_size, const int block_num,
		const int kb_per_inode)
{
	int bits_per_block = block_size * CHAR_BIT;
	int bitmap_block_num = (block_num + bits_per_block - 1)
			/ bits_per_block;
	int free_block_num = block_num - TFS_RESERVED_BLOCKS - bitmap_block_num;

	// root inode and blocks should be allocated
	if (free_block_num < 1 + ROOT_BLOCK_NUM) {
		fprintf(stderr, "too small block_num\n");
		return ERR_ARG;
	}

	int bytes_per_inode = kb_per_inode * 1024;
	//int blocks_per_inode = (bytes_per_inode + block_size - 1) / block_size + 2;
	int blocks_per_inode = bytes_per_inode / block_size + 1;
	int inode_num = free_block_num / blocks_per_inode;
	if (inode_num <= 0) {
		fprintf(stderr, "too small block_num\n");
		return ERR_ARG;
	}

	printf("superblock = %d, bitmap block = %d, inode block = %d\n",
			1, bitmap_block_num, inode_num);

	super.magic = TFS_MAGIC;
	super.blksize = block_size;
	super.nblock = block_num;
	super.bitmapsize = bitmap_block_num;
	super.ninode = inode_num;

	int reserved = TFS_RESERVED_BLOCKS + bitmap_block_num + inode_num;
	super.datablock = reserved;

	// root inode is allocated
	super.freeinode = inode_num - 1;
	super.isearch = 1;

	// root blocks are allocated
	super.freeblock = block_num - reserved - ROOT_BLOCK_NUM;
	super.bsearch = reserved + ROOT_BLOCK_NUM - 1;

	return 0;
}

static int skip_boot_block(FILE *fp)
{
	// boot block occupies 1 sector
	if (fseek(fp, super.blksize, SEEK_SET)) {
		perror("fseek failed in skip_boot_block\n");
		return ERR_FILE;
	}

	return 0;
}

static int write_super_block(FILE *fp)
{
	// super block occupies 1 sector
	memset(buf, 0, super.blksize);
	memcpy(buf, &super, sizeof(super));

	if (fwrite(buf, super.blksize, 1, fp) != 1) {
		perror("fwrite failed in write_super_block\n");
		return ERR_FILE;
	}

	return 0;
}

static int write_bitmap(FILE *fp)
{
	memset(buf, 0, super.blksize);

	int edge_bit = super.datablock + ROOT_BLOCK_NUM;
	int edge_byte = edge_bit / CHAR_BIT;
	int edge_block = edge_byte / super.blksize;
	int i = 0;

	while (i++ < edge_block)
		if (fwrite(buf, super.blksize, 1, fp) != 1) {
			perror("fwrite failed in write_bitmap\n");
			return ERR_FILE;
		}

	edge_byte %= super.blksize;
	buf[edge_byte] = ~((1 << edge_bit % CHAR_BIT) - 1);
	memset(&(buf[edge_byte + 1]), 0xff, super.blksize - edge_byte - 1);

	if (fwrite(buf, super.blksize, 1, fp) != 1) {
		perror("fwrite failed in write_bitmap\n");
		return ERR_FILE;
	}

	memset(buf, 0xff, super.blksize);

	while (++i < super.bitmapsize)
		if (fwrite(buf, super.blksize, 1, fp) != 1) {
			perror("fwrite failed in write_bitmap\n");
			return ERR_FILE;
		}

	return 0;
}

static int write_inodes(FILE *fp)
{
	// inode occupies 1 sector
	time_t t = time(NULL);
	root_inode.i_atime.sec = t;
	root_inode.i_mtime.sec = t;
	root_inode.i_ctime.sec = t;

	memset(buf, 0, super.blksize);
	memcpy(buf, &root_inode, sizeof(root_inode));

	// 1st index points to 2nd index block
	struct sfs_inode *inode = (struct sfs_inode*)buf;
	inode->i_indirect[0] = super.datablock;

	if (fwrite(buf, super.blksize, 1, fp) != 1) {
		perror("fwrite failed in write_inodes\n");
		return ERR_FILE;
	}

	// skip unused inodes
	if (fseek(fp, (super.ninode - 1) * super.blksize, SEEK_CUR)) {
		perror("fseek failed in write_inodes\n");
		return ERR_FILE;
	}

	return 0;
}

static int write_index(FILE *fp)
{
	// index block occupies 1 sector
	memset(buf, 0, super.blksize);

	// 2nd index points to body block
	uint32_t *index = (uint32_t*)buf;
	*index = super.datablock + 1;

	if (fwrite(buf, super.blksize, 1, fp) != 1) {
		perror("fwrite failed in write_index\n");
		return ERR_FILE;
	}

	return 0;
}

static int write_directory(FILE *fp)
{
	// body block occupies 1 sector
	memset(buf, 0, super.blksize);
	memcpy(buf, root_directory, sizeof(root_directory));

	if (fwrite(buf, super.blksize, 1, fp) != 1) {
		perror("fwrite failed in write_directory\n");
		return ERR_FILE;
	}

	return 0;
}

int main(int argc, char **argv)
{
	if (argc != 5)
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

	int kb_per_inode = atoi(argv[4]);
	if (kb_per_inode <= 0) {
		fprintf(stderr, "bad kb_per_inode\n");
		return ERR_ARG;
	}

	int error = 0;
	error = calc_super_block(block_size, block_num, kb_per_inode);
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

		error = write_index(fp);
		if (error)
			break;

		error = write_directory(fp);
		if (error)
			break;
	} while (false);

	fclose(fp);
	free(buf);

	return error;
}
