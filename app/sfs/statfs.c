/*

   B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

   GNU GENERAL PUBLIC LICENSE
   Version 2, June 1991

   (C) B-Free Project.

   (C) 2001, Tomohide Naniwa

 */

/* 
 * $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/mkfs/statfs.c,v 1.25 2000/07/02 15:29:36 kishida0 Exp $
 *
 *
 * $Log: statfs.c,v $
 * Revision 1.25  2000/07/02 15:29:36  kishida0
 * add command line argument counter
 *
 * Revision 1.24  2000/06/02 10:29:34  naniwa
 * to show error message when disk is full
 *
 * Revision 1.23  2000/05/06 03:51:00  naniwa
 * fixed file parmission
 *
 * Revision 1.22  2000/02/20 09:40:45  naniwa
 * to implement mknod
 *
 * Revision 1.21  2000/02/16 08:19:57  naniwa
 * minor fix on error message
 *
 * Revision 1.20  1999/12/23 07:04:15  naniwa
 * fixed alloc_inode
 *
 * Revision 1.19  1999/12/21 10:54:58  naniwa
 * fixed alloc_inode()
 *
 * Revision 1.18  1999/05/28 15:52:13  naniwa
 * modified to support sfs ver 1.1
 *
 * Revision 1.17  1999/05/13 08:25:20  naniwa
 * fixed free_indirect(), free_dindirect()
 *
 * Revision 1.16  1999/05/06 23:17:22  naniwa
 * fixed to keep file system consistent
 *
 * Revision 1.15  1999/04/23 18:41:52  night
 * Add free_inode function.
 *
 * Revision 1.14  1999/04/23 12:00:20  monaka
 * Added include directive (errno.h)
 *
 * Revision 1.13  1999/04/13 04:15:34  monaka
 * MAJOR FIXcvs commit -m 'MAJOR FIX!!! There are so many changes, modifys, fixes. Sorry but I can't remember all of those. For example, all the manager and driver programmer have got power to access all ITRON systemcall. (My works is just making access route to ITRON. I don't know what happens in the nuclus.'! There are so many changes, modifys, fixes. Sorry but I can't remember all of those. For example, all the manager and driver programmer have got power to access all ITRON systemcall. (My works is just making access route to ITRON. I don't know what happens in the nuclus.
 *
 * Revision 1.12  1997/08/31 13:37:21  night
 * ヘルプメッセージの変更。
 *
 * Revision 1.11  1997/04/24 15:43:54  night
 * 註釈の修正。
 *
 * Revision 1.10  1996/11/18  13:47:53  night
 * write コマンドを実行したとき、ファイルが存在しない場合に新たに作成する
 * ように機能を変更した。
 *
 * Revision 1.9  1996/11/16  13:01:23  night
 * f_mkdir () 関数の中で write_file() を呼び出しているが、第6引数の
 * 型をキャストして正しいもの (B*) に合わせた。
 * (ワーニング出力抑制のため)
 *
 * Revision 1.8  1996/11/16  12:58:43  night
 * write_file() の中で write() システムコールの返り値としてチェックする
 * 値として struct inode のサイズを使っていたが、正しく struct sfs_inode
 * を使うように変更した。
 *
 * Revision 1.7  1996/11/14  13:19:22  night
 * 一重および二重間接ブロックの処理を追加。
 *
 * Revision 1.6  1996/11/13  12:54:41  night
 * コマンド処理機能の追加。
 *
 * Revision 1.5  1996/11/12  11:33:07  night
 * mount_fs()、alloc_inode()、alloc_block() の関数を追加した。
 *
 * Revision 1.4  1996/11/11  13:53:01  night
 * read_file() 関数をメモリを消費しないものに修正。
 *
 * Revision 1.3  1996/11/11  13:38:46  night
 * ディレクトリの内容を取り出す機能の追加。
 *
 * Revision 1.2  1996/11/10  11:57:12  night
 * ファイルシステムの root ディレクトリの中身を表示するような処理を追加。
 *
 * Revision 1.1  1996/11/08  11:07:42  night
 * 最初の登録
 *
 *
 */


#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include "sfs_utils.h"
#include "../../include/mpu/bits.h"

static int mount_fs(block_device_t *, char *, struct sfs_superblock *, struct sfs_inode *, int);
static int lookup_file(block_device_t *, struct sfs_superblock *, struct sfs_inode *, char *, struct sfs_inode *);
static int create_file(block_device_t *, struct sfs_superblock *, struct sfs_inode *, const char *, int, struct sfs_inode *);
static int read_file(block_device_t *, struct sfs_superblock *, struct sfs_inode *, int, int, char *);
static int write_inode(block_device_t *, struct sfs_superblock *, struct sfs_inode *);
static int truncate_file(block_device_t *, struct sfs_superblock *, struct sfs_inode *, int);

static int alloc_block(block_device_t *, struct sfs_superblock *);
static int free_block(block_device_t *, struct sfs_superblock *, int);
static void free_indirect(block_device_t *, struct sfs_superblock *, struct sfs_inode *, int, int);
static int get_block_num(block_device_t *, struct sfs_superblock *, struct sfs_inode *, int);
static int get_indirect_block_num(block_device_t *, struct sfs_superblock *, struct sfs_inode *, int);
static int set_block_num(block_device_t *, struct sfs_superblock *, struct sfs_inode *, int, int);
static int set_indirect_block_num(block_device_t *, struct sfs_superblock *, struct sfs_inode *, int, int);
static int locallookup_file(block_device_t *, struct sfs_superblock *, struct sfs_inode *, struct sfs_inode *, char *);


static W read_dir(block_device_t *, struct sfs_superblock *, struct sfs_inode *, int, struct sfs_dir *);

static W read_inode(block_device_t *, struct sfs_superblock *, int, struct sfs_inode *);
static int write_file(block_device_t *, struct sfs_superblock *, struct sfs_inode *,
		      int, int, char *);

static unsigned char rootdir_buf[SFS_BLOCK_SIZE];
static struct sfs_inode *rootdirp;
static block_device_t device;

static int f_create_file(block_device_t *, struct sfs_superblock *, char *);
static int f_write_file(block_device_t *, struct sfs_superblock *, char *, char *);
static int f_dir(block_device_t *, struct sfs_superblock *, char *);
static int f_mkdir(block_device_t *, struct sfs_superblock *, char *);
static int f_chmod(block_device_t *, struct sfs_superblock *, char *, char *);
static int f_statvfs(block_device_t *, struct sfs_superblock *);

static struct cmd {
    char *name;
    int options;
    int (*funcp) ();
} cmdtable[] =

{
    {
	"write", 2, f_write_file
    },
    {
	"dir", 1, f_dir
    },
    {
	"mkdir", 1, f_mkdir
    },
    {
	"chmod", 2, f_chmod
    },
    {
	"statvfs", 0, f_statvfs
    },
    {
	NULL, 0, NULL
    }
};


static void usage(void)
{
    fprintf(stderr, "usage: statfs device command args\n");
    fprintf(stderr, "Command:\n");
    fprintf(stderr, "\twrite to from\n");
    fprintf(stderr, "\tdir directory\n");
    fprintf(stderr, "\tmkdir directory\n");
    fprintf(stderr, "\tchmod mode path\n");
}

int main(int ac, char **av)
{
    int fd;
    unsigned char sb_buf[SFS_BLOCK_SIZE];
    int i;

    if (ac < 3) {
	usage();
	return (0);
    }
    block_initialize(&device);
    fd = mount_fs(&device, av[1], (struct sfs_superblock*)sb_buf,
	    (struct sfs_inode*)rootdir_buf, O_RDWR);
    rootdirp = (struct sfs_inode*)rootdir_buf;

    for (i = 0; cmdtable[i].name != NULL; i++) {
	int err;

	if (strcmp(av[2], cmdtable[i].name) == 0) {
	    if ((cmdtable[i].options) + 3 != ac) {
		fprintf(stderr, "command line option error\n");
		fprintf(stderr, "-----------------\n");
		usage();
		return (0);
	    }
	    err = (*cmdtable[i].funcp) (&device, (struct sfs_superblock*)sb_buf, av[3], av[4], av[5]);
	    if (err) {
		fprintf(stderr, "errno = %d\n", err);
		exit(err);
	    }
	    exit(0);
	}
    }
    fprintf(stderr, "Unknown command: %s\n", av[2]);
    exit(1);
}



/* ============================================================
 *  コマンド
 *
 */

/* f_create_file

 */
static int f_create_file(block_device_t *dev, struct sfs_superblock *sb, char *path)
{
    unsigned char parent_ip[SFS_BLOCK_SIZE];
    unsigned char ip[SFS_BLOCK_SIZE];
    int err;
    char *pdirname, *fname;
    int i;

    for (i = strlen(path); i > 0; i--) {
	if (path[i] == '/')
	    break;
    }

    pdirname = alloca(i + 1);
    strncpy(pdirname, path, i);
    pdirname[i] = '\0';

    fname = alloca(strlen(path) - i + 1);
    strncpy(fname, &path[i + 1], strlen(path) - i);
    fname[strlen(path) - i] = '\0';

    err = lookup_file(dev, sb, rootdirp, pdirname, (struct sfs_inode*)parent_ip);
    if (err) {
	fprintf(stderr, "cannot lookup parent directory [%s]\n", pdirname);
	exit(err);
    }
    err = create_file(dev, sb, (struct sfs_inode*)parent_ip, fname, 0666, (struct sfs_inode*)ip);
    if (err) {
	fprintf(stderr, "cannot create file [%s]\n", fname);
	exit(err);
    }
    return (0);
}



/* f_write_file

 */
static int f_write_file(block_device_t *dev, struct sfs_superblock *sb, char *path, char *src_file)
{
    unsigned char ip[SFS_BLOCK_SIZE];
    int err;
    int dfd;
    struct stat st;
    char *buf;

    dfd = open(src_file, O_RDONLY);
    if (dfd < 0) {
	fprintf(stderr, "cannot open src file [%s]\n", src_file);
	exit(-1);
    }

    fstat(dfd, &st);
    buf = (char *) malloc(st.st_size);
    if (buf == NULL) {
	fprintf(stderr, "cannot allocate buffer\n");
	exit(ENOMEM);
    }

    err = lookup_file(dev, sb, rootdirp, path, (struct sfs_inode*)ip);
    if (err) {
	if (err == ENOENT) {
	    err = f_create_file(dev, sb, path);
	    if (err)
		return (err);

	    err = lookup_file(dev, sb, rootdirp, path, (struct sfs_inode*)ip);
	    if (err)
		return (err);

	} else {
	    fprintf(stderr, "cannot open file [%s]\n", path);
	    free(buf);
	    exit(err);
	}
    }

    read(dfd, buf, st.st_size);
    err = write_file(dev, sb, (struct sfs_inode*)ip, 0, st.st_size, buf);
    close(dfd);

    if (err) {
	fprintf(stderr, "cannot write to file.\n");
	free(buf);
	exit(err);
    }

    free(buf);
    return (0);
}


static int f_dir(block_device_t *dev, struct sfs_superblock *sb, char *path)
{
    unsigned char ip[SFS_BLOCK_SIZE];
    int err;
    int nentry;
    struct sfs_dir *dirp;
    char *name;
    int i;

    err = lookup_file(dev, sb, rootdirp, path, (struct sfs_inode*)ip);
    if (err) {
	fprintf(stderr, "cannot open file [%s]\n", path);
	exit(err);
    }

    if ((((struct sfs_inode*)ip)->i_mode & S_IFMT) != S_IFDIR) {
	fprintf(stderr, "%10o %3u %5u %5u %8u %-14s\n",
		((struct sfs_inode*)ip)->i_mode, ((struct sfs_inode*)ip)->i_nlink, ((struct sfs_inode*)ip)->i_uid, ((struct sfs_inode*)ip)->i_gid,
		((struct sfs_inode*)ip)->i_size, path);
	return (0);
    }

    nentry = read_dir(dev, sb, (struct sfs_inode*)ip, 0, NULL);
    dirp = alloca(nentry * sizeof(struct sfs_dir));

    if (read_dir(dev, sb, (struct sfs_inode*)ip, nentry, dirp) != 0) {
	fprintf(stderr, "cannot read directory\n");
	exit(-1);
    }

    name = alloca(strlen(path) + SFS_MAXNAMELEN + 2);

    for (i = 0; i < nentry; i++) {
	unsigned char fip[SFS_BLOCK_SIZE];

	strcpy(name, path);
	strcat(name, "/");
	strcat(name, (char*)(dirp[i].d_name));
	err = lookup_file(dev, sb, rootdirp, name, (struct sfs_inode*)fip);
	if (err) {
	    fprintf(stderr, "err = %d\n", err);
	    exit(err);
	}

	if ((((struct sfs_inode*)fip)->i_mode & S_IFCHR) != 0) {
	    fprintf(stderr, "%10o %3u %5u %5u %8s %-14s\n",
		    ((struct sfs_inode*)fip)->i_mode, ((struct sfs_inode*)fip)->i_nlink, ((struct sfs_inode*)fip)->i_uid, ((struct sfs_inode*)fip)->i_gid,
		    "", dirp[i].d_name);
	} else {
	    fprintf(stderr, "%10o %3u %5u %5u %8u %-14s\n",
		    ((struct sfs_inode*)fip)->i_mode, ((struct sfs_inode*)fip)->i_nlink, ((struct sfs_inode*)fip)->i_uid, ((struct sfs_inode*)fip)->i_gid,
		    ((struct sfs_inode*)fip)->i_size, dirp[i].d_name);
	}
    }
    return (0);
}


static int f_mkdir(block_device_t *dev, struct sfs_superblock *sb, char *path)
{
    unsigned char parent_ip[SFS_BLOCK_SIZE];
    unsigned char ip[SFS_BLOCK_SIZE];
    int err;
    char *pdirname, *fname;
    int i;
    static struct sfs_dir dir[2] =
    {
	{0, "."},
	{0, ".."}
    };

    for (i = strlen(path); i > 0; i--)
	if (path[i] == '/')
	    break;

    pdirname = alloca(i + 1);
    strncpy(pdirname, path, i);
    pdirname[i] = '\0';

    fname = alloca(strlen(path) - i + 1);
    strncpy(fname, &path[i + 1], strlen(path) - i);
    fname[strlen(path) - i] = '\0';

/*  fprintf (stderr, "parent = %s, file = %s\n", pdirname, fname); */
    err = lookup_file(dev, sb, rootdirp, pdirname, (struct sfs_inode*)parent_ip);
    if (err) {
	fprintf(stderr, "cannot lookup parent directory [%s]\n", pdirname);
	exit(err);
    }

    err = create_file(dev, sb, (struct sfs_inode*)parent_ip, fname,
	    S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH, (struct sfs_inode*)ip);
    if (err) {
	fprintf(stderr, "cannot create file [%s]\n", fname);
	exit(err);
    }

    ((struct sfs_inode*)ip)->i_nlink = 2;
    ((struct sfs_inode*)ip)->i_mode = (((struct sfs_inode*)ip)->i_mode & ~S_IFMT) | (S_IFDIR);
    err = write_inode(dev, sb, (struct sfs_inode*)ip);
    if (err) {
	fprintf(stderr, "cannot write inode\n");
	exit(err);
    }

    /* parent_ip のリンク数の更新は create_file では行わない． */
    ((struct sfs_inode*)parent_ip)->i_nlink += 1;
    err = write_inode(dev, sb, (struct sfs_inode*)parent_ip);
    if (err) {
	fprintf(stderr, "cannot write inode\n");
	exit(err);
    }

    dir[0].d_index = ((struct sfs_inode*)ip)->i_index;
    dir[1].d_index = ((struct sfs_inode*)parent_ip)->i_index;
    err = write_file(dev, sb, (struct sfs_inode*)ip, 0, sizeof(dir), (char *) dir);
    if (err) {
	fprintf(stderr, "cannot write to directory.\n");
	exit(err);
    }

    return (0);
}


static int f_chmod(block_device_t *dev, struct sfs_superblock *sb, char *num, char *path)
{
    unsigned char ip[SFS_BLOCK_SIZE];
    int err;
    int i;

    err = lookup_file(dev, sb, rootdirp, path, (struct sfs_inode*)ip);
    if (err) {
	fprintf(stderr, "cannot open file [%s]\n", path);
	exit(err);
    }

    i = strtol(num, NULL, 8);
    ((struct sfs_inode*)ip)->i_mode = i | (((struct sfs_inode*)ip)->i_mode & S_IFMT);
    err = write_inode(dev, sb, (struct sfs_inode*)ip);
    if (err) {
	fprintf(stderr, "cannot write inode\n");
	exit(err);
    }

    return (0);
}

static int f_statvfs(block_device_t *dev, struct sfs_superblock *sb)
{
	printf("f_bsize: %d\n", sb->blksize);
	printf("f_frsize: %d\n", sb->blksize);
	printf("f_blocks: %d\n", sb->nblock);
	printf("f_bfree: %d\n", sb->freeblock);
	printf("f_bavail: %d\n", sb->freeblock);
	printf("f_files: %d\n", sb->ninode);
	printf("f_ffree: %d\n", sb->freeinode);
	printf("f_favail: %d\n", sb->freeinode);
	printf("f_fsid: %d\n", 0);
	printf("f_flag: %d\n", 0);
	printf("f_namemax: %d\n", SFS_MAXNAMELEN);

	return 0;
}


/* ファイルシステム全体に関係する処理

 * mount_fs()
 *
 */


static int mount_fs(block_device_t *dev, char *path, struct sfs_superblock *sb, struct sfs_inode *root, int mode)
{
    int fd;

    if (mode == O_RDONLY)
	fd = open(path, O_RDONLY);
    else
	fd = open(path, O_RDWR);

    if (fd < 0) {
	fprintf(stderr, "Cannot open file.\n");
	exit(-1);
    }

    dev->channel = fd;
    dev->block_size = SFS_BLOCK_SIZE;
    
    if (dev->read(dev, sb, 1))
	return (-1);

    read_inode(dev, sb, 1, root);

    return (fd);
}


static W
read_dir(block_device_t *dev,
	 struct sfs_superblock * sb,
	 struct sfs_inode * ip,
	 int nentry,
	 struct sfs_dir * dirp)
{
    int size;

    if ((nentry <= 0) || (dirp == NULL))
	return (ip->i_size / sizeof(struct sfs_dir));

    size = (nentry * sizeof(struct sfs_dir) <= ip->i_size) ?
     nentry * sizeof(struct sfs_dir) :
     ip->i_size;

    read_file(dev, sb, ip, 0, size, (char *) dirp);		/* エラーチェックが必要! */
    return (0);
}


/* inode に関係している処理

 * get_inode_offset()
 * read_inode()
 * alloc_inode()
 * write_inode()
 */
int get_inode_offset(struct sfs_superblock *sb, int ino)
{
    return (2 + (ROUNDUP(sb->nblock / 8, sb->blksize) / sb->blksize) + ino - 1);
}


static W
read_inode(block_device_t *dev, struct sfs_superblock *sb, int ino, struct sfs_inode *ip)
{
    return dev->read(dev, ip, get_inode_offset(sb, ino));
}


int alloc_inode(block_device_t *dev, struct sfs_superblock *sb)
{
    int i;
    unsigned char ip[SFS_BLOCK_SIZE];

    if (sb->freeinode <= 0)
	return (0);

    for (i = sb->isearch; i <= sb->ninode; i++) {
	if (dev->read(dev, ip, get_inode_offset(sb, i)))
	    return (0);

	if (((struct sfs_inode*)ip)->i_index != i) {
	    memset(ip, 0, sizeof(ip));
	    ((struct sfs_inode*)ip)->i_index = i;
	    if (dev->write(dev, ip, get_inode_offset(sb, ((struct sfs_inode*)ip)->i_index))) {
		fprintf(stderr, "write fail\n");
	    	exit(-1);
	    }

	    sb->freeinode--;
	    sb->isearch = (i + 1);

	    if (dev->write(dev, sb, 1)) {
		fprintf(stderr, "write fail\n");
	    	exit(-1);
	    }

	    return (i);
	}
    }

    return (0);
}


static int write_inode(block_device_t *dev, struct sfs_superblock *sb, struct sfs_inode *ip)
{
    if (dev->write(dev, ip, get_inode_offset(sb, ip->i_index)))
	return (EIO);

    /* rootdir_buf の内容の更新 */
    if (ip->i_index == 1)
	memcpy(rootdirp, ip, sizeof(struct sfs_inode));

    return (0);
}



/* ファイルに関係している処理

 * write_file()
 * read_file()
 * create_file()
 * lookup()
 */
static int write_file(block_device_t *dev,
	       struct sfs_superblock *sb,
	       struct sfs_inode *ip,
	       int start,
	       int size,
	       char * buf)
{
    B *blockbuf;
    int filesize = start + size;

    blockbuf = (B *) alloca(sb->blksize);

    while (size > 0) {
	int copysize;
	int offset;

	if (get_block_num(dev, sb, ip, start / sb->blksize) <= 0) {
	    /* ファイルサイズを越えて書き込む場合には、新しくブロックをアロケートする
	     */
	    set_block_num(dev, sb, ip, start / sb->blksize, alloc_block(dev, sb));
	    memset(blockbuf, 0, sb->blksize);
	} else if (dev->read(dev, blockbuf,
		get_block_num(dev, sb, ip, start / sb->blksize)))
	    exit(-1);

	/* 読み込んだブロックの内容を更新する
	 */
	offset = start % sb->blksize;
	copysize = MIN(sb->blksize - offset, size);
	memcpy(&blockbuf[offset], buf, copysize);

	/* 更新したブロックを書き込む
	 */
	if (dev->write(dev, blockbuf,
		    get_block_num(dev, sb, ip, start / sb->blksize)))
	    exit(-1);

	buf += copysize;
	start += copysize;
	size -= copysize;
    }

    /* もし、書き込みをおこなった後にファイルのサイズが増えていれば、
     * サイズを更新して inode を書き込む。
     * ファイルのサイズが減っていればファイルを切り詰める．
     */
    if (filesize > ip->i_size) {
	ip->i_size = filesize;
	ip->i_nblock = ROUNDUP(filesize, sb->blksize) / sb->blksize;
	write_inode(dev, sb, ip);
    } else
	truncate_file(dev, sb, ip, filesize);

    return (0);
}


static int read_file(block_device_t *dev,
	      struct sfs_superblock *sb,
	      struct sfs_inode *ip,
	      int start,
	      int size,
	      char * buf)
{
    B *blockbuf;

    if (start + size > ip->i_size)
	size = ip->i_size - start;

/*  fprintf (stderr, "read_file: offset = %d, size = %d\n", start, size); */
    blockbuf = (B *) alloca(sb->blksize);

    while (size > 0) {
	int offset = start % sb->blksize;
	int copysize = MIN(sb->blksize - offset, size);

	if (dev->read(dev, blockbuf,
		   get_block_num(dev, sb, ip, start / sb->blksize)))
	    exit(-1);
	memcpy(buf, &blockbuf[offset], copysize);

	buf += copysize;
	start += copysize;
	size -= copysize;
    }

    return (0);
}



static int create_file(block_device_t *dev,
		struct sfs_superblock *sb,
		struct sfs_inode *parent_dir,
		const char *name,
		int mode,
		struct sfs_inode *newinode)
{
    int inode_index;
    struct sfs_dir *dirp;
    struct sfs_dir newdir_entry;
    int nentry;
    int dir_index;
    time_t t;

    if (strlen(name) > SFS_MAXNAMELEN)
	return (ENAMETOOLONG);

    nentry = read_dir(dev, sb, parent_dir, 0, NULL);
    dirp = alloca(nentry * sizeof(struct sfs_dir));
    if (read_dir(dev, sb, parent_dir, nentry, dirp) != 0)
	return (ENOENT);

    for (dir_index = 0; dir_index < nentry; dir_index++) {
	if (dirp[dir_index].d_index <= 0)
	    /* 削除したエントリがある */
	    break;

	if (strncmp((char *) (dirp[dir_index].d_name), name, SFS_MAXNAMELEN) == 0)
	    return (EEXIST);
    }

    /*
     * Inode の作成
     */
    if ((inode_index = alloc_inode(dev, sb)) == 0)
	return (ENOMEM);

    memset(newinode, 0, sizeof(struct sfs_inode));
    newinode->i_index = inode_index;
    newinode->i_nlink = 1;
    newinode->i_size = 0;
    newinode->i_nblock = 0;
    newinode->i_mode = mode | S_IFREG;
    newinode->i_uid = newinode->i_gid = 0;
    t = time(NULL);
    newinode->i_atime.sec = t;
    newinode->i_atime.nsec = 0;
    newinode->i_mtime.sec = t;
    newinode->i_mtime.nsec = 0;
    newinode->i_ctime.sec = t;
    newinode->i_ctime.nsec = 0;
    write_inode(dev, sb, newinode);

    /* parent_dir のリンクカウントは増やさない．
       parent_dir->i_nlink += 1;
       write_inode (dev, sb, parent_dir);
     */

    /* 親ディレクトリの更新
     */
    newdir_entry.d_index = newinode->i_index;
    strcpy((char *) (newdir_entry.d_name), name);
    write_file(dev,
	       sb,
	       parent_dir,
	       sizeof(struct sfs_dir) * dir_index,
	       sizeof(struct sfs_dir),
	        (char *) &newdir_entry);
    return (0);
}


static int lookup_file(block_device_t *dev,
		struct sfs_superblock *sb,
		struct sfs_inode *cwd,
		char *path,
		struct sfs_inode *ip)
{
    char name[SFS_MAXNAMELEN + 1];
    struct sfs_inode *dirp;
    struct sfs_inode *pdirp;
    unsigned char dirbuf[SFS_BLOCK_SIZE];
    int i;
    int err;

    if (strcmp(path, "/") == 0) {
	memcpy(ip, cwd, sizeof(struct sfs_inode));
	return (0);
    }

    if (*path == '/')
	path++;

    pdirp = cwd;
    dirp = (struct sfs_inode*)dirbuf;

    while (*path) {
	if (*path == '/')
	    path++;

	for (i = 0;; i++) {
	    if (i > SFS_MAXNAMELEN)
		return (ENAMETOOLONG);

	    if ((*path == '/') || (*path == '\0'))
		break;

	    name[i] = *path++;
	}

	if (i == 0)
	    break;

	name[i] = '\0';

#ifdef DEBUG
	fprintf (stderr, "local lookup = %s\n", name);
#endif
	err = locallookup_file(dev, sb, pdirp, dirp, name);
	if (err)
	    return (err);

	pdirp = dirp;
	dirp = pdirp;
    }

    memcpy(ip, pdirp, sizeof(struct sfs_inode));
    return (0);
}


static int locallookup_file(block_device_t *dev,
		     struct sfs_superblock *sb,
		     struct sfs_inode *parent,
		     struct sfs_inode *ip,
		     char *name)
{
    int nentry;
    struct sfs_dir *dirp;
    int i;

    nentry = read_dir(dev, sb, parent, 0, NULL);
    dirp = alloca(sizeof(struct sfs_dir) * nentry);
    read_dir(dev, sb, parent, nentry, dirp);

    for (i = 0; i < nentry; i++)
	if (strcmp(name, (char *) (dirp[i].d_name)) == 0) {
	    read_inode(dev, sb, dirp[i].d_index, ip);
	    return (0);
	}

    return (ENOENT);
}


static int truncate_file(block_device_t *dev,
		  struct sfs_superblock *sb,
		  struct sfs_inode *ip,
		  int newsize)
{
    int nblock = ROUNDUP(newsize, sb->blksize);

    if (nblock < ROUNDUP(ip->i_size, sb->blksize)) {
	int blockno = nblock / sb->blksize;
	int inblock, offset;
	int i;

	/* 余分なブロックを解放する
	 */
	for (i = blockno;
	     i < ROUNDUP(ip->i_size, sb->blksize) / sb->blksize;
	     i++) {
	    free_block(dev, sb, get_block_num(dev, sb, ip, i));
	}

	/* 間接ブロックの block の開放 */
	offset = blockno % SFS_INDIRECT_BLOCK;
	inblock = blockno / SFS_INDIRECT_BLOCK;
	free_indirect(dev, sb, ip, offset, inblock);
    }

    ip->i_size = newsize;
    ip->i_nblock = nblock;

    return (write_inode(dev, sb, ip));
}

static void free_indirect(block_device_t *dev, struct sfs_superblock *sb, struct sfs_inode *ip,
		   int offset, int inblock)
{
    int i;

    if (offset) {
	struct sfs_indirect inbuf;

	if (dev->read(dev, &inbuf, ip->i_indirect[inblock]))
	    exit(-1);

	for (i = offset; i < SFS_INDIRECT_BLOCK; ++i)
	    inbuf.in_block[i] = 0;

	if (dev->write(dev, &inbuf, ip->i_indirect[inblock]))
	    exit(-1);

	++inblock;
    }

    for (i = inblock; i < SFS_INDIRECT_BLOCK_ENTRY; ++i)
	if (ip->i_indirect[i] > 0) {
	    free_block(dev, sb, ip->i_indirect[i]);
	    ip->i_indirect[i] = 0;
	}
}


/* ブロックに関係している処理

 * alloc_block()
 * get_block_num()
 * set_block_num()
 *
 */
static int alloc_block(block_device_t *dev, struct sfs_superblock *sb)
{
    int i, s;
    unsigned int buf[SFS_BLOCK_SIZE / 4];

    if (sb->freeblock <= 0) {
	fprintf(stderr, "cannot allocate block\n");
	exit(-1);
    }

    for (i = s = (sb->bsearch - 1) / (8 * sb->blksize); i < sb->bitmapsize; i++) {
	int j;

	if (dev->read(dev, buf, i + 2))
	    return (-1);

	for (j = (i == s)? ((((sb->bsearch - 1) / 8) % sb->blksize) / 4):0;
		j < sb->blksize / 4; j++)
	    if (buf[j]) {
		int k = count_ntz(buf[j]);
		int free_block = (i * sb->blksize * 8)
			+ (j * 32)
			+ k;

		buf[j] &= ~(1 << k);

		if (dev->write(dev, buf, i + 2)) {
		    fprintf(stderr, "write fail\n");
		    exit(-1);
		}

		sb->freeblock--;
		sb->bsearch = free_block;

		if (dev->write(dev, sb, 1)) {
		    fprintf(stderr, "write fail\n");
		    exit(-1);
		}

		return (free_block);
	    }
    }

    return (-1);
}


static int free_block(block_device_t *dev, struct sfs_superblock *sb, int blockno)
{
    int no = (blockno / (8 * SFS_BLOCK_SIZE)) + 2;
    unsigned int block[SFS_BLOCK_SIZE / 4];

    if (dev->read(dev, block, no)) {
	fprintf(stderr, "read fail\n");
	exit(-1);
    }

    block[(blockno / 8) % (SFS_BLOCK_SIZE / 4)] |= 1 << (blockno % 32);

    if (dev->write(dev, block, no)) {
	fprintf(stderr, "write fail\n");
	exit(-1);
    }

    sb->freeblock++;

    if (sb->bsearch >= blockno && blockno > 0)
	sb->bsearch = blockno - 1;

    if (dev->write(dev, sb, 1)) {
	fprintf(stderr, "write fail\n");
	exit(-1);
    }

    return (0);
}


static int get_block_num(block_device_t *dev,
		  struct sfs_superblock *sb,
		  struct sfs_inode *ip,
		  int blockno)
{
    if (blockno < (SFS_INDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK))
	/* 一重間接ブロックの範囲内
	 */
	return (get_indirect_block_num(dev, sb, ip, blockno));

    return (-1);
}


static int get_indirect_block_num(block_device_t *dev, struct sfs_superblock *sb, struct sfs_inode *ip, int blockno)
{
    int inblock = blockno / SFS_INDIRECT_BLOCK;
    int inblock_offset = blockno % SFS_INDIRECT_BLOCK;
    struct sfs_indirect inbuf;

    if (ip->i_indirect[inblock] <= 0)
	return (0);

    if (dev->read(dev, &inbuf, ip->i_indirect[inblock]))
	exit(-1);

    return (inbuf.in_block[inblock_offset]);
}

static int set_block_num(block_device_t *dev,
		  struct sfs_superblock *sb,
		  struct sfs_inode *ip,
		  int blockno,
		  int newblock)
{
    if (newblock < 0)
	return (-1);

    if (blockno < (SFS_INDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK))
	/* 一重間接ブロックの範囲内
	 */
	return (set_indirect_block_num(dev, sb, ip, blockno, newblock));

    return (-1);
}



static int set_indirect_block_num(block_device_t *dev,
			   struct sfs_superblock *sb,
			   struct sfs_inode *ip,
			   int blockno,
			   int newblock)
{
    int inblock = blockno / SFS_INDIRECT_BLOCK;
    int inblock_offset = blockno % SFS_INDIRECT_BLOCK;
    struct sfs_indirect inbuf;

    if (ip->i_indirect[inblock] <= 0) {
	int newinblock = alloc_block(dev, sb);

	ip->i_indirect[inblock] = newinblock;
	memset(&inbuf, 0, sizeof(inbuf));

    } else if (dev->read(dev, &inbuf, ip->i_indirect[inblock]))
	exit(-1);

    inbuf.in_block[inblock_offset] = newblock;
    if (dev->write(dev, &inbuf, ip->i_indirect[inblock]))
    	exit(-1);
    write_inode(dev, sb, ip);

    return (inbuf.in_block[inblock_offset]);
}
