/*

   B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

   GNU GENERAL PUBLIC LICENSE
   Version 2, June 1991

   (C) B-Free Project.

   (C) 2001, Tomohide Naniwa

 */

/*
 * $Log: mkfs.c,v $
 * Revision 1.12  2000/05/06 03:50:59  naniwa
 * fixed file parmission
 *
 * Revision 1.11  1999/05/28 15:52:11  naniwa
 * modified to support sfs ver 1.1
 *
 * Revision 1.10  1999/05/17 12:46:34  naniwa
 * changed 4th argument to 'KB per inode'
 *
 * Revision 1.9  1999/05/06 23:17:21  naniwa
 * fixed to keep file system consistent
 *
 * Revision 1.8  1999/04/18 17:13:18  monaka
 * Fixed memory allocation in writing bitmap. (Thanx for Mr. Naniwa.)
 *
 * Revision 1.7  1999/04/13 04:15:33  monaka
 * MAJOR FIXcvs commit -m 'MAJOR FIX!!! There are so many changes, modifys, fixes. Sorry but I can't remember all of those. For example, all the manager and driver programmer have got power to access all ITRON systemcall. (My works is just making access route to ITRON. I don't know what happens in the nuclus.'! There are so many changes, modifys, fixes. Sorry but I can't remember all of those. For example, all the manager and driver programmer have got power to access all ITRON systemcall. (My works is just making access route to ITRON. I don't know what happens in the nuclus.
 *
 * Revision 1.6  1997/09/09 13:51:20  night
 * MAX_MODULE_NAME マクロの定義を追加。
 *
 * Revision 1.5  1996/11/14 13:19:07  night
 * root および lost+found ディレクトリの分のブロック数をフリーブロック数
 * から減らしてファイルシステムを作成するように変更。
 *
 * Revision 1.4  1996/11/13  12:54:19  night
 * 作成するディレクトリのパーミッションの変更。
 *
 * Revision 1.3  1996/11/11  13:38:18  night
 * ルードディレクトリの内容を一部変更。
 * ('lost+found' ディレクトリを追加)
 *
 * Revision 1.2  1996/11/10  11:56:48  night
 * mkfs するときにブートブロックの分を考慮するようにした。
 *
 * Revision 1.1  1996/11/08  11:07:09  night
 * 最初の登録
 *
 *
 */


#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include "../../include/fs/sfs.h"

#define ROUNDUP(x,align)	(((((int)x) + ((align) - 1))/(align))*(align))

static int nblock;
static int blocksize;
static int inodecount;
static int boot_block;
static int super_block;
static int bitmap_block;
static int inode_block;

static void write_superblock(int formatfd);
static void write_bitmap(int formatfd);
static void write_inode(int formatfd);
static void write_rootdir(int formatfd);
static void set_bit(char buf[], int index);

static struct tfs_dir rootentry[] = {
    {1, 1, ".\0"},
    {1, 2, ".."}
};


static struct sfs_inode rootdir = {
    1,				/* sfs_i_index */
    2,				/* sfs_i_nlink */
    sizeof(rootentry),		/* sfs_i_size */
    1,				/* sfs_i_size_blk */
    (S_IFDIR | S_IRWXU | S_IRWXG | S_IRWXO),	/* sfs_i_mode */
    0,				/* sfs_i_uid */
    0,				/* sfs_i_gid */
    {0, 0},				/* sfs_i_atime (now time) */
    {0, 0},				/* sfs_i_ctime (now time) */
    {0, 0},				/* sfs_i_mtime (now time) */
};


static void usage(void)
{
    printf("usage: mkfs file nblock KB-per-inode\n");
}

/* main -

 */
int main(int ac, char **av)
{
    int formatfd;
    int kbpinode;

    if (ac != 4) {
	usage();
	return (0);
    }

    nblock = atoi(av[2]);
    blocksize = SFS_BLOCK_SIZE;
    kbpinode = atoi(av[3]);

    formatfd = open(av[1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (formatfd < 0) {
	printf("Cannot open file or device (%s)\n", av[1]);
	return (1);
    }
    if (kbpinode > 99) {
	printf
	    ("WARNING: The 3rd argument has been changed to 'KB per inode' from 'number of inode'.\n");
    }
    boot_block = 1;
    super_block = 1;
    bitmap_block = ROUNDUP(nblock / 8, blocksize) / blocksize;
    inodecount = (nblock - boot_block - super_block - bitmap_block) /
	(kbpinode * 1024 / SFS_BLOCK_SIZE + ROUNDUP(SFS_BLOCK_SIZE,
				blocksize) / blocksize);
    inode_block =
	ROUNDUP(inodecount * SFS_BLOCK_SIZE,
		blocksize) / blocksize;

    printf("superblock = %d, bitmap block = %d, inode block = %d\n",
	   super_block, bitmap_block, inode_block);

    write_superblock(formatfd);
    write_bitmap(formatfd);
    write_inode(formatfd);
    write_rootdir(formatfd);

    close(formatfd);
    exit(0);
}

static void write_superblock(int formatfd)
{
    int error;
    char buf[SFS_BLOCK_SIZE];

    struct sfs_superblock superblock;

    superblock.magic = TFS_MAGIC;
    superblock.blksize = blocksize;
    superblock.nblock = nblock;
    superblock.freeblock =
	nblock - (boot_block + super_block + bitmap_block + inode_block +
		  2);
    superblock.bitmapsize = bitmap_block;
    superblock.ninode = inodecount;
    superblock.freeinode = inodecount - 1;	/* root ディレクトリの分 */
    superblock.datablock =
	(boot_block + super_block + bitmap_block + inode_block);
    superblock.isearch = 1;
    superblock.bsearch = (boot_block + super_block + bitmap_block
			      + inode_block + 2) - 1;

    memset(buf, 0, sizeof(buf));
    memcpy(buf, &superblock, sizeof(superblock));
    lseek(formatfd, blocksize, 0);
    error = write(formatfd, buf, sizeof(buf));
    if (error < 0) {
	perror("Write error in write_superblock().\n");
    }
}


static void write_bitmap(int formatfd)
{
    char *buf;
    int i;
    int error;

    buf = alloca(blocksize * bitmap_block);
    memset(buf, 0xff, blocksize * bitmap_block);
    for (i = 0;
	 i < (boot_block + super_block + bitmap_block + inode_block + 2);
	 i++) {
	set_bit(buf, i);
    }

    lseek(formatfd, blocksize * (super_block + boot_block), 0);
    error = write(formatfd, buf, blocksize * bitmap_block);
    if (error < 0) {
	perror("Write error in write_bitmap().\n");
    }
}



static void set_bit(char buf[], int index)
{
    int byte_offset;
    int bit_offset;
    int value;

    byte_offset = index / 8;
    bit_offset = index % 8;

    value = 1 << bit_offset;

    buf[byte_offset] &= ~value;
}



static void write_inode(int formatfd)
{
    char *buf;
    int i;
    int error;
    time_t t;

    buf = alloca(blocksize);
    memset(buf, 0, blocksize);
    for (i = 0; i < inode_block; i++) {
	error = write(formatfd, buf, blocksize);
	if (error < 0) {
	    perror("Write error in write_inode().\n");
	}
    }

    t = time(NULL);
    rootdir.i_atime.sec = t;
    rootdir.i_atime.nsec = 0;
    rootdir.i_mtime.sec = t;
    rootdir.i_mtime.nsec = 0;
    rootdir.i_ctime.sec = t;
    rootdir.i_ctime.nsec = 0;
    lseek(formatfd, blocksize * (boot_block + super_block + bitmap_block),
	  0);
    write(formatfd, &rootdir, sizeof(rootdir));

    uint32_t i_indirect = boot_block + super_block + bitmap_block
	    + inode_block;
    write(formatfd, &i_indirect, sizeof(i_indirect));
}



static void write_rootdir(int formatfd)
{
    char buf[SFS_BLOCK_SIZE];
    int *intp = (int*)buf;
    struct tfs_dir *dirp = (struct tfs_dir *)buf;

    memset(buf, 0, sizeof(buf));
    *intp = boot_block + super_block + bitmap_block
	    + inode_block + 1;
    lseek(formatfd,
	  blocksize * (boot_block + super_block + bitmap_block +
		       inode_block), 0);
    write(formatfd, buf, sizeof(buf));

    memset(buf, 0, sizeof(buf));
    memcpy(dirp, rootentry, sizeof(rootentry));
    lseek(formatfd,
	  blocksize * (boot_block + super_block + bitmap_block +
		       inode_block + 1), 0);
    write(formatfd, buf, sizeof(buf));
}
