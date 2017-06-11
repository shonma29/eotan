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
#include "sfs_utils.h"

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

static struct sfs_inode rootdir_buf;
static struct sfs_inode *rootdirp;
static block_device_t device;

static int f_create_file(block_device_t *, struct sfs_superblock *, char *);
static int f_write_file(block_device_t *, struct sfs_superblock *, char *, char *);
static int f_dir(block_device_t *, struct sfs_superblock *, char *);
static int f_mkdir(block_device_t *, struct sfs_superblock *, char *);
static int f_chmod(block_device_t *, struct sfs_superblock *, char *, char *);

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
    struct sfs_superblock sb;
    int i;

    if (ac < 3) {
	usage();
	return (0);
    }
    block_initialize(&device);
    fd = mount_fs(&device, av[1], &sb, &rootdir_buf, O_RDWR);
    rootdirp = &rootdir_buf;

    for (i = 0; cmdtable[i].name != NULL; i++) {
	int err;

	if (strcmp(av[2], cmdtable[i].name) == 0) {
	    if ((cmdtable[i].options) + 3 != ac) {
		fprintf(stderr, "command line option error\n");
		fprintf(stderr, "-----------------\n");
		usage();
		return (0);
	    }
	    err = (*cmdtable[i].funcp) (&device, &sb, av[3], av[4], av[5]);
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
    struct sfs_inode parent_ip, ip;
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

    err = lookup_file(dev, sb, rootdirp, pdirname, &parent_ip);
    if (err) {
	fprintf(stderr, "cannot lookup parent directory [%s]\n", pdirname);
	exit(err);
    }
    err = create_file(dev, sb, &parent_ip, fname, 0666, &ip);
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
    struct sfs_inode ip;
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

    err = lookup_file(dev, sb, rootdirp, path, &ip);
    if (err) {
	if (err == ENOENT) {
	    err = f_create_file(dev, sb, path);
	    if (err) {
		return (err);
	    }
	    err = lookup_file(dev, sb, rootdirp, path, &ip);
	    if (err) {
		return (err);
	    }
	} else {
	    fprintf(stderr, "cannot open file [%s]\n", path);
	    free(buf);
	    exit(err);
	}
    }
    read(dfd, buf, st.st_size);
    err = write_file(dev, sb, &ip, 0, st.st_size, buf);
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
    struct sfs_inode ip;
    int err;
    int nentry;
    struct sfs_dir *dirp;
    char *name;
    int i;

    err = lookup_file(dev, sb, rootdirp, path, &ip);
    if (err) {
	fprintf(stderr, "cannot open file [%s]\n", path);
	exit(err);
    }
    if ((ip.i_mode & S_IFMT) != S_IFDIR) {
	fprintf(stderr, "%3u  %4.4o\t%4.4u\t%4.4u\t%-14s\t%u bytes\n",
		ip.i_nlink, ip.i_mode, ip.i_uid, ip.i_gid,
		path, ip.i_size);
	return (0);
    }
    nentry = read_dir(dev, sb, &ip, 0, NULL);
    dirp = alloca(nentry * sizeof(struct sfs_dir));
    if (read_dir(dev, sb, &ip, nentry, dirp) != 0) {
	fprintf(stderr, "cannot read directory\n");
	exit(-1);
    }
    name = alloca(strlen(path) + SFS_MAXNAMELEN + 2);
    for (i = 0; i < nentry; i++) {
	struct sfs_inode ip;

	strcpy(name, path);
	strcat(name, "/");
	strcat(name, (char*)(dirp[i].d_name));
	err = lookup_file(dev, sb, rootdirp, name, &ip);
	if (err) {
	    fprintf(stderr, "err = %d\n", err);
	    exit(err);
	}
	if ((ip.i_mode & S_IFCHR) != 0) {
	    fprintf(stderr, "%3u  %4.4o\t%4.4u\t%4.4u\t%-14s\n",
	       ip.i_nlink, ip.i_mode, ip.i_uid, ip.i_gid,
		    dirp[i].d_name);
	} else {
	    fprintf(stderr, "%3u  %4.4o\t%4.4u\t%4.4u\t%-14s\t%u bytes\n",
	       ip.i_nlink, ip.i_mode, ip.i_uid, ip.i_gid,
		    dirp[i].d_name, ip.i_size);
	}
    }
    return (0);
}


static int f_mkdir(block_device_t *dev, struct sfs_superblock *sb, char *path)
{
    struct sfs_inode parent_ip, ip;
    int err;
    char *pdirname, *fname;
    int i;
    static struct sfs_dir dir[2] =
    {
	{0, "."},
	{0, ".."}
    };

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

/*  fprintf (stderr, "parent = %s, file = %s\n", pdirname, fname); */
    err = lookup_file(dev, sb, rootdirp, pdirname, &parent_ip);
    if (err) {
	fprintf(stderr, "cannot lookup parent directory [%s]\n", pdirname);
	exit(err);
    }
    err = create_file(dev, sb, &parent_ip, fname,
	    S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH, &ip);
    if (err) {
	fprintf(stderr, "cannot create file [%s]\n", fname);
	exit(err);
    }
    ip.i_nlink = 2;
    ip.i_mode = (ip.i_mode & ~S_IFMT) | (S_IFDIR);
    err = write_inode(dev, sb, &ip);
    if (err) {
	fprintf(stderr, "cannot write inode\n");
	exit(err);
    }
    /* parent_ip のリンク数の更新は create_file では行わない． */
    parent_ip.i_nlink += 1;
    err = write_inode(dev, sb, &parent_ip);
    if (err) {
	fprintf(stderr, "cannot write inode\n");
	exit(err);
    }
    dir[0].d_index = ip.i_index;
    dir[1].d_index = parent_ip.i_index;
    err = write_file(dev, sb, &ip, 0, sizeof(dir), (char *) dir);
    if (err) {
	fprintf(stderr, "cannot write to directory.\n");
	exit(err);
    }
    return (0);
}


static int f_chmod(block_device_t *dev, struct sfs_superblock *sb, char *num, char *path)
{
    struct sfs_inode ip;
    int err;
    int i;

    err = lookup_file(dev, sb, rootdirp, path, &ip);
    if (err) {
	fprintf(stderr, "cannot open file [%s]\n", path);
	exit(err);
    }
    sscanf(num, "%o", &i);
    ip.i_mode = i | (ip.i_mode & S_IFMT);
    err = write_inode(dev, sb, &ip);
    if (err) {
	fprintf(stderr, "cannot write inode\n");
	exit(err);
    }
    return (0);
}


/* ファイルシステム全体に関係する処理

 * mount_fs()
 *
 */


static int mount_fs(block_device_t *dev, char *path, struct sfs_superblock *sb, struct sfs_inode *root, int mode)
{
    int fd;

    if (mode == O_RDONLY) {
	fd = open(path, O_RDONLY);
    } else {
	fd = open(path, O_RDWR);
    }

    if (fd < 0) {
	fprintf(stderr, "Cannot open file.\n");
	exit(-1);
    }
    if (lseek(fd, SFS_BLOCK_SIZE, 0) < 0) {
	return (-1);
    }
    if (read(fd, sb, sizeof(struct sfs_superblock)) != sizeof(struct sfs_superblock)) {
	return (-1);
    }
    dev->channel = fd;
    dev->block_size = sb->blksize;
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

    if ((nentry <= 0) || (dirp == NULL)) {
	return (ip->i_size / sizeof(struct sfs_dir));
    }
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
    int offset;
    int nblock;
    int blocksize;

    nblock = sb->nblock;
    blocksize = sb->blksize;
    offset = 1 + 1 + (ROUNDUP(nblock / 8, blocksize) / blocksize);
    offset *= blocksize;
    return (offset + ((ino - 1) * sizeof(struct sfs_inode)));
}


static W
read_inode(block_device_t *dev, struct sfs_superblock *sb, int ino, struct sfs_inode *ip)
{
    int offset;
    offset = get_inode_offset(sb, ino);
    lseek(dev->channel, offset, 0);
    read(dev->channel, ip, sizeof(struct sfs_inode));
    return (0);
}


int alloc_inode(int fd, struct sfs_superblock *sb)
{
    int i;
    struct sfs_inode ipbuf;

    if (sb->freeinode <= 0) {
	return (0);
    }
    if (lseek(fd, get_inode_offset(sb, sb->isearch), 0) < 0) {
	perror("lseek");
	exit(1);
    }
    for (i = sb->isearch; i <= sb->ninode; i++) {
	if (read(fd, &ipbuf, sizeof(struct sfs_inode)) < sizeof(struct sfs_inode)) {
	    return (0);
	}
	if (ipbuf.i_index != i) {
	    bzero(&ipbuf, sizeof(ipbuf));
	    ipbuf.i_index = i;
	    lseek(fd, get_inode_offset(sb, ipbuf.i_index), 0);
	    write(fd, &ipbuf, sizeof(ipbuf));
	    sb->freeinode--;
	    sb->isearch = (i + 1);
	    lseek(fd, 1 * sb->blksize, 0);
	    write(fd, sb, sizeof(struct sfs_superblock));

	    return (i);
	}
    }

    return (0);
}


static int write_inode(block_device_t *dev, struct sfs_superblock *sb, struct sfs_inode *ip)
{
    lseek(dev->channel, get_inode_offset(sb, ip->i_index), 0);
    if (write(dev->channel, ip, sizeof(struct sfs_inode)) < sizeof(struct sfs_inode)) {
	return (EIO);
    }
    /* rootdir_buf の内容の更新 */
    if (ip->i_index == 1) {
	memmove(rootdirp, ip, sizeof(struct sfs_inode));
    }
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
    int copysize;
    int offset;
    int retsize;
    int filesize;


    retsize = size;
    filesize = start + retsize;

    blockbuf = (B *) alloca(sb->blksize);
    while (size > 0) {
	if (get_block_num(dev, sb, ip, start / sb->blksize) <= 0) {
	    /* ファイルサイズを越えて書き込む場合には、新しくブロックをアロケートする
	     */
	    set_block_num(dev, sb, ip, start / sb->blksize, alloc_block(dev, sb));
/*
 *   ip->i_direct[start / sb->blksize] = alloc_block (dev, sb);
 */
	    bzero(blockbuf, sb->blksize);
	} else {
	    dev->read(dev, blockbuf,
		    get_block_num(dev, sb, ip, start / sb->blksize));
	}

	/* 読み込んだブロックの内容を更新する
	 */
	offset = start % sb->blksize;
	copysize = MIN(sb->blksize - offset, size);
	bcopy(buf, &blockbuf[offset], copysize);

	/* 更新したブロックを書き込む
	 */
	dev->write(dev, blockbuf,
		    get_block_num(dev, sb, ip, start / sb->blksize));

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
    } else {
	truncate_file(dev, sb, ip, filesize);
    }
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
    int copysize;
    int offset;

    if (start + size > ip->i_size) {
	size = ip->i_size - start;
    }

/*  fprintf (stderr, "read_file: offset = %d, size = %d\n", start, size); */
    blockbuf = (B *) alloca(sb->blksize);
    while (size > 0) {
	dev->read(dev, blockbuf,
		   get_block_num(dev, sb, ip, start / sb->blksize));
	offset = start % sb->blksize;
	copysize = MIN(sb->blksize - offset, size);
	bcopy(&blockbuf[offset], buf, copysize);

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
    if (read_dir(dev, sb, parent_dir, nentry, dirp) != 0) {
	return (ENOENT);
    }
    for (dir_index = 0; dir_index < nentry; dir_index++) {
	if (dirp[dir_index].d_index <= 0) {
	    /* 削除したエントリがある */
	    break;
	}
	if (strncmp((char *) (dirp[dir_index].d_name), name, SFS_MAXNAMELEN) == 0) {
	    return (EEXIST);
	}
    }

    /*
     * Inode の作成
     */
    if ((inode_index = alloc_inode(dev->channel, sb)) == 0) {
	return (ENOMEM);
    }
    bzero(newinode, sizeof(struct sfs_inode));
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
    struct sfs_inode dirbuf;
    int i;
    int err;

    if (strcmp(path, "/") == 0) {
	bcopy(cwd, ip, sizeof(struct sfs_inode));
	return (0);
    }
    if (*path == '/') {
	path++;
    }
    pdirp = cwd;
    dirp = &dirbuf;

    while (*path) {
	if (*path == '/') {
	    path++;
	}
	for (i = 0;; i++) {
	    if (i > SFS_MAXNAMELEN) {
		return (ENAMETOOLONG);
	    }
	    if ((*path == '/') || (*path == '\0')) {
		break;
	    }
	    name[i] = *path++;
	}
	if (i == 0)
	    break;

	name[i] = '\0';

#ifdef DEBUG
	fprintf (stderr, "local lookup = %s\n", name);
#endif
	err = locallookup_file(dev, sb, pdirp, dirp, name);
	if (err) {
	    return (err);
	}
	pdirp = dirp;
	dirp = pdirp;
    }

    bcopy(pdirp, ip, sizeof(struct sfs_inode));
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
    for (i = 0; i < nentry; i++) {
	if (strcmp(name, (char *) (dirp[i].d_name)) == 0) {
	    read_inode(dev, sb, dirp[i].d_index, ip);
	    return (0);
	}
    }
    return (ENOENT);
}


static int truncate_file(block_device_t *dev,
		  struct sfs_superblock *sb,
		  struct sfs_inode *ip,
		  int newsize)
{
    int nblock, blockno, inblock, offset;
    int i;

    nblock = ROUNDUP(newsize, sb->blksize);
    if (nblock < ROUNDUP(ip->i_size, sb->blksize)) {
	/* 余分なブロックを解放する
	 */
	for (blockno = i = nblock / sb->blksize;
	     i < ROUNDUP(ip->i_size, sb->blksize) / sb->blksize;
	     i++) {
	    free_block(dev, sb, get_block_num(dev, sb, ip, i));
	}

	/* 間接ブロックの block の開放 */
	inblock = blockno;
	offset = inblock % SFS_INDIRECT_BLOCK;
	inblock = inblock / SFS_INDIRECT_BLOCK;
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

    if (offset != 0) {
	struct sfs_indirect inbuf;

	dev->read(dev, (B *) & inbuf, ip->i_indirect[inblock]);

	for (i = offset; i < SFS_INDIRECT_BLOCK; ++i)
	    inbuf.in_block[i] = 0;

	dev->write(dev, (B *) & inbuf, ip->i_indirect[inblock]);

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
    int startoffset;
    int i, s;
    char *buf;

    if (sb->freeblock <= 0) {
	fprintf(stderr, "cannot allocate block\n");
	exit(-1);
    }

    startoffset = ((1 + 1) * sb->blksize);
    lseek(dev->channel, startoffset, 0);
    buf = alloca(sb->blksize);
    s = (sb->bsearch - 1) / (8 * sb->blksize);

    for (i = s; i < sb->bitmapsize; i++) {
	int j;

	lseek(dev->channel, (i * sb->blksize) + startoffset, 0);

	if (read(dev->channel, buf, sb->blksize) < 0)
	    return (-1);

	for (j = (i == s)? (((sb->bsearch - 1) / 8) % sb->blksize):0;
		j < sb->blksize; j++)
	    if (buf[j] & 0xff) {
		unsigned char mask = 1;
		int k;

		for (k = 0; k < 8; k++) {
		    if (mask & buf[j]) {
			int free_block = (i * sb->blksize * 8)
			    + (j * 8)
			    + k;

			buf[j] = buf[j] & ~mask;
			lseek(dev->channel, (i * sb->blksize) + startoffset, 0);

			if (write(dev->channel, buf, sb->blksize) < 0) {
			    fprintf(stderr, "write fail\n");
			    exit(-1);
			}

			sb->freeblock--;
			sb->bsearch = free_block;
			lseek(dev->channel, 1 * sb->blksize, 0);
			write(dev->channel, sb, sizeof(struct sfs_superblock));

			return (free_block);
		    }

		    mask = mask << 1;
		}
	    }
    }

    return (-1);
}


static int free_block(block_device_t *dev, struct sfs_superblock *sb, int blockno)
{
    unsigned char block;
    int startoffset;
    int mask;

    startoffset = ((1 + 1) * sb->blksize) + (blockno / 8);
    lseek(dev->channel, startoffset, 0);
    read(dev->channel, &block, 1);

    mask = 0x01;
    mask = mask << (blockno % 8);
    block = block | (mask & 0xff);
    lseek(dev->channel, startoffset, 0);
    write(dev->channel, &block, 1);

    sb->freeblock++;

    if (sb->bsearch >= blockno && blockno > 0)
	sb->bsearch = blockno - 1;

    lseek(dev->channel, 1 * sb->blksize, 0);
    write(dev->channel, sb, sizeof(struct sfs_superblock));

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

    dev->read(dev, (B *) & inbuf, ip->i_indirect[inblock]);

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
	bzero((B *) & inbuf, sizeof(inbuf));

    } else
	dev->read(dev, (B *) & inbuf, ip->i_indirect[inblock]);

    inbuf.in_block[inblock_offset] = newblock;
    dev->write(dev, (B *) & inbuf, ip->i_indirect[inblock]);
    write_inode(dev, sb, ip);

    return (inbuf.in_block[inblock_offset]);
}
