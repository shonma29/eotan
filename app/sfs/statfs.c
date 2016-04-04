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
#include "sfs_utils.h"

static int mount_fs(char *, struct sfs_superblock *, struct sfs_inode *, int);
static int lookup_file(int, struct sfs_superblock *, struct sfs_inode *, char *, struct sfs_inode *);
static int create_file(int, struct sfs_superblock *, struct sfs_inode *, const char *, int, struct sfs_inode *);
static int read_file(int, struct sfs_superblock *, struct sfs_inode *, int, int, char *);
static int write_inode(int, struct sfs_superblock *, struct sfs_inode *);
static int truncate_file(int, struct sfs_superblock *, struct sfs_inode *, int);

static int read_block(int, int, int, B *);
static int write_block(int, int, int, B *);
static int alloc_block(int, struct sfs_superblock *);
static int free_block(int, struct sfs_superblock *, int);
static void free_indirect(int, struct sfs_superblock *, struct sfs_inode *, int, int);
static void free_dindirect(int, struct sfs_superblock *, struct sfs_inode *, int, int, int);
static void free_all_dindirect(int, struct sfs_superblock *, struct sfs_inode *, int);
static int get_block_num(int, struct sfs_superblock *, struct sfs_inode *, int);
static int get_indirect_block_num(int, struct sfs_superblock *, struct sfs_inode *, int);
static int set_block_num(int, struct sfs_superblock *, struct sfs_inode *, int, int);
static int set_indirect_block_num(int, struct sfs_superblock *, struct sfs_inode *, int, int);
static int set_dindirect_block_num(int, struct sfs_superblock *, struct sfs_inode *, int, int);
static int locallookup_file(int, struct sfs_superblock *, struct sfs_inode *, struct sfs_inode *, char *);
static int get_dindirect_block_num(int, struct sfs_superblock *, struct sfs_inode *, int);


static W read_dir(int, struct sfs_superblock *, struct sfs_inode *, int, struct sfs_dir *);

static W read_inode(int, struct sfs_superblock *, int, struct sfs_inode *);
static int write_file(int, struct sfs_superblock *, struct sfs_inode *,
		      int, int, char *);

static struct sfs_inode rootdir_buf;
static struct sfs_inode *rootdirp;

static int f_create_file(int, struct sfs_superblock *, char *);
static int f_write_file(int, struct sfs_superblock *, char *, char *);
static int f_dir(int, struct sfs_superblock *, char *);
static int f_mkdir(int, struct sfs_superblock *, char *);
static int f_chmod(int, struct sfs_superblock *, char *, char *);

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
    fd = mount_fs(av[1], &sb, &rootdir_buf, O_RDWR);
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
	    err = (*cmdtable[i].funcp) (fd, &sb, av[3], av[4], av[5]);
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
static int f_create_file(int fd, struct sfs_superblock *sb, char *path)
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

    err = lookup_file(fd, sb, rootdirp, pdirname, &parent_ip);
    if (err) {
	fprintf(stderr, "cannot lookup parent directory [%s]\n", pdirname);
	return (err);
    }
    err = create_file(fd, sb, &parent_ip, fname, 0666, &ip);
    if (err) {
	fprintf(stderr, "cannot create file [%s]\n", fname);
	return (err);
    }
    return (0);
}



/* f_write_file

 */
static int f_write_file(int fd, struct sfs_superblock *sb, char *path, char *src_file)
{
    struct sfs_inode ip;
    int err;
    int dfd;
    struct stat st;
    char *buf;

    dfd = open(src_file, O_RDONLY);
    if (dfd < 0) {
	fprintf(stderr, "cannot open src file [%s]\n", src_file);
	return (-1);
    }
    fstat(dfd, &st);
    buf = (char *) malloc(st.st_size);
    if (buf == NULL) {
	fprintf(stderr, "cannot allocate buffer\n");
	return (ENOMEM);
    }

    err = lookup_file(fd, sb, rootdirp, path, &ip);
    if (err) {
	if (err == ENOENT) {
	    err = f_create_file(fd, sb, path);
	    if (err) {
		return (err);
	    }
	    err = lookup_file(fd, sb, rootdirp, path, &ip);
	    if (err) {
		return (err);
	    }
	} else {
	    fprintf(stderr, "cannot open file [%s]\n", path);
	    free(buf);
	    return (err);
	}
    }
    read(dfd, buf, st.st_size);
    err = write_file(fd, sb, &ip, 0, st.st_size, buf);
    close(dfd);
    if (err) {
	fprintf(stderr, "cannot write to file.\n");
	free(buf);
	return (err);
    }
    free(buf);
    return (0);
}


static int f_dir(int fd, struct sfs_superblock *sb, char *path)
{
    struct sfs_inode ip;
    int err;
    int nentry;
    struct sfs_dir *dirp;
    char *name;
    int i;

    err = lookup_file(fd, sb, rootdirp, path, &ip);
    if (err) {
	fprintf(stderr, "cannot open file [%s]\n", path);
	return (err);
    }
    if ((ip.i_mode & S_IFMT) != S_IFDIR) {
	fprintf(stderr, "%3u  %4.4o\t%4.4u\t%4.4u\t%-14s\t%u bytes\n",
		ip.i_nlink, ip.i_mode, ip.i_uid, ip.i_gid,
		path, ip.i_size);
	return (0);
    }
    nentry = read_dir(fd, sb, &ip, 0, NULL);
    dirp = alloca(nentry * sizeof(struct sfs_dir));
    if (read_dir(fd, sb, &ip, nentry, dirp) != 0) {
	fprintf(stderr, "cannot read directory\n");
	return (-1);
    }
    name = alloca(strlen(path) + SFS_MAXNAMELEN + 2);
    for (i = 0; i < nentry; i++) {
	struct sfs_inode ip;

	strcpy(name, path);
	strcat(name, "/");
	strcat(name, (char*)(dirp[i].d_name));
	err = lookup_file(fd, sb, rootdirp, name, &ip);
	if (err) {
	    fprintf(stderr, "err = %d\n", err);
	    return (err);
	}
	if ((ip.i_mode & S_IFCHR) != 0) {
	    fprintf(stderr, "%3u  %4.4o\t%4.4u\t%4.4u\t%-14s\t%08x\n",
	       ip.i_nlink, ip.i_mode, ip.i_uid, ip.i_gid,
		    dirp[i].d_name, ip.i_direct[0]);
	} else {
	    fprintf(stderr, "%3u  %4.4o\t%4.4u\t%4.4u\t%-14s\t%u bytes\n",
	       ip.i_nlink, ip.i_mode, ip.i_uid, ip.i_gid,
		    dirp[i].d_name, ip.i_size);
	}
    }
    return (0);
}


static int f_mkdir(int fd, struct sfs_superblock *sb, char *path)
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
    err = lookup_file(fd, sb, rootdirp, pdirname, &parent_ip);
    if (err) {
	fprintf(stderr, "cannot lookup parent directory [%s]\n", pdirname);
	return (err);
    }
    err = create_file(fd, sb, &parent_ip, fname,
	    S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH, &ip);
    if (err) {
	fprintf(stderr, "cannot create file [%s]\n", fname);
	return (err);
    }
    ip.i_nlink = 2;
    ip.i_mode = (ip.i_mode & ~S_IFMT) | (S_IFDIR);
    err = write_inode(fd, sb, &ip);
    if (err) {
	fprintf(stderr, "cannot write inode\n");
	return (err);
    }
    /* parent_ip のリンク数の更新は create_file では行わない． */
    parent_ip.i_nlink += 1;
    err = write_inode(fd, sb, &parent_ip);
    if (err) {
	fprintf(stderr, "cannot write inode\n");
	return (err);
    }
    dir[0].d_index = ip.i_index;
    dir[1].d_index = parent_ip.i_index;
    err = write_file(fd, sb, &ip, 0, sizeof(dir), (char *) dir);
    if (err) {
	fprintf(stderr, "cannot write to directory.\n");
	return (err);
    }
    return (0);
}


static int f_chmod(int fd, struct sfs_superblock *sb, char *num, char *path)
{
    struct sfs_inode ip;
    int err;
    int i;

    err = lookup_file(fd, sb, rootdirp, path, &ip);
    if (err) {
	fprintf(stderr, "cannot open file [%s]\n", path);
	return (err);
    }
    sscanf(num, "%o", &i);
    ip.i_mode = i | (ip.i_mode & S_IFMT);
    err = write_inode(fd, sb, &ip);
    if (err) {
	fprintf(stderr, "cannot write inode\n");
	return (err);
    }
    return (0);
}


/* ファイルシステム全体に関係する処理

 * mount_fs()
 *
 */


static int mount_fs(char *path, struct sfs_superblock *sb, struct sfs_inode *root, int mode)
{
    int fd;

    if (mode == O_RDONLY) {
	fd = open(path, O_RDONLY);
    } else {
	fd = open(path, O_RDWR);
    }

    if (fd < 0) {
	fprintf(stderr, "Cannot open file.\n");
	return (-1);
    }
    if (lseek(fd, SFS_BLOCK_SIZE, 0) < 0) {
	return (-1);
    }
    if (read(fd, sb, sizeof(struct sfs_superblock)) != sizeof(struct sfs_superblock)) {
	return (-1);
    }
    read_inode(fd, sb, 1, root);
    return (fd);
}


static W
read_dir(int fd,
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

    read_file(fd, sb, ip, 0, size, (char *) dirp);		/* エラーチェックが必要! */
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
read_inode(int fd, struct sfs_superblock *sb, int ino, struct sfs_inode *ip)
{
    int offset;
    offset = get_inode_offset(sb, ino);
    lseek(fd, offset, 0);
    read(fd, ip, sizeof(struct sfs_inode));
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


/* free_inode - 


 */
int free_inode(int fd, struct sfs_superblock *sb, int inode_index)
{
    struct sfs_inode ipbuf;

    if (lseek(fd, get_inode_offset(sb, 1), 0) < 0) {
	perror("lseek");
	exit(1);
    }
    bzero(&ipbuf, sizeof(ipbuf));
    if (lseek(fd, get_inode_offset(sb, inode_index), 0) < 0) {
	perror("lseek");
	exit(1);
    }
    write(fd, &ipbuf, sizeof(ipbuf));

    sb->freeinode++;
    if (sb->isearch >= inode_index)
	sb->isearch = inode_index - 1;
    lseek(fd, 1 * sb->blksize, 0);
    write(fd, sb, sizeof(struct sfs_superblock));

    return (0);
}


static int write_inode(int fd, struct sfs_superblock *sb, struct sfs_inode *ip)
{
    lseek(fd, get_inode_offset(sb, ip->i_index), 0);
    if (write(fd, ip, sizeof(struct sfs_inode)) < sizeof(struct sfs_inode)) {
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
static int write_file(int fd,
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
	if (get_block_num(fd, sb, ip, start / sb->blksize) <= 0) {
	    /* ファイルサイズを越えて書き込む場合には、新しくブロックをアロケートする
	     */
	    set_block_num(fd, sb, ip, start / sb->blksize, alloc_block(fd, sb));
/*
 *   ip->i_direct[start / sb->blksize] = alloc_block (fd, sb);
 */
	    bzero(blockbuf, sb->blksize);
	} else {
	    read_block(fd,
		    get_block_num(fd, sb, ip, start / sb->blksize),
		       sb->blksize,
		       blockbuf);
	}

	/* 読み込んだブロックの内容を更新する
	 */
	offset = start % sb->blksize;
	copysize = MIN(sb->blksize - offset, size);
	bcopy(buf, &blockbuf[offset], copysize);

	/* 更新したブロックを書き込む
	 */
	write_block(fd,
		    get_block_num(fd, sb, ip, start / sb->blksize),
		    sb->blksize,
		    blockbuf);

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
	write_inode(fd, sb, ip);
    } else {
	truncate_file(fd, sb, ip, filesize);
    }
    return (0);
}


static int read_file(int fd,
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
	read_block(fd,
		   get_block_num(fd, sb, ip, start / sb->blksize),
		   sb->blksize,
		   blockbuf);
	offset = start % sb->blksize;
	copysize = MIN(sb->blksize - offset, size);
	bcopy(&blockbuf[offset], buf, copysize);

	buf += copysize;
	start += copysize;
	size -= copysize;
    }
    return (0);
}



static int create_file(int fd,
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

    nentry = read_dir(fd, sb, parent_dir, 0, NULL);
    dirp = alloca(nentry * sizeof(struct sfs_dir));
    if (read_dir(fd, sb, parent_dir, nentry, dirp) != 0) {
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
    if ((inode_index = alloc_inode(fd, sb)) == 0) {
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
    write_inode(fd, sb, newinode);

    /* parent_dir のリンクカウントは増やさない．
       parent_dir->i_nlink += 1;
       write_inode (fd, sb, parent_dir);
     */

    /* 親ディレクトリの更新
     */
    newdir_entry.d_index = newinode->i_index;
    strcpy((char *) (newdir_entry.d_name), name);
    write_file(fd,
	       sb,
	       parent_dir,
	       sizeof(struct sfs_dir) * dir_index,
	       sizeof(struct sfs_dir),
	        (char *) &newdir_entry);
    return (0);
}


static int lookup_file(int fd,
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
	err = locallookup_file(fd, sb, pdirp, dirp, name);
	if (err) {
	    return (err);
	}
	pdirp = dirp;
	dirp = pdirp;
    }

    bcopy(pdirp, ip, sizeof(struct sfs_inode));
    return (0);
}


static int locallookup_file(int fd,
		     struct sfs_superblock *sb,
		     struct sfs_inode *parent,
		     struct sfs_inode *ip,
		     char *name)
{
    int nentry;
    struct sfs_dir *dirp;
    int i;

    nentry = read_dir(fd, sb, parent, 0, NULL);
    dirp = alloca(sizeof(struct sfs_dir) * nentry);
    read_dir(fd, sb, parent, nentry, dirp);
    for (i = 0; i < nentry; i++) {
	if (strcmp(name, (char *) (dirp[i].d_name)) == 0) {
	    read_inode(fd, sb, dirp[i].d_index, ip);
	    return (0);
	}
    }
    return (ENOENT);
}


static int truncate_file(int fd,
		  struct sfs_superblock *sb,
		  struct sfs_inode *ip,
		  int newsize)
{
    int nblock, blockno, inblock, offset, dinblock;
    int i;

    nblock = ROUNDUP(newsize, sb->blksize);
    if (nblock < ROUNDUP(ip->i_size, sb->blksize)) {
	/* 余分なブロックを解放する
	 */
	for (blockno = i = nblock / sb->blksize;
	     i < ROUNDUP(ip->i_size, sb->blksize) / sb->blksize;
	     i++) {
	    free_block(fd, sb, get_block_num(fd, sb, ip, i));
	}

	/* 間接ブロックの block の開放 */
	if (blockno < SFS_DIRECT_BLOCK_ENTRY) {
	    /* 直接ブロックの範囲内 */
	    for (i = blockno; i < SFS_DIRECT_BLOCK_ENTRY; ++i) {
		ip->i_direct[i] = 0;
	    }
	    free_indirect(fd, sb, ip, 0, 0);
	    free_all_dindirect(fd, sb, ip, 0);
	} else if (blockno < (SFS_DIRECT_BLOCK_ENTRY
		    + (SFS_INDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK))) {
	    /* 一重間接ブロックの範囲内 */
	    inblock = (blockno - SFS_DIRECT_BLOCK_ENTRY);
	    offset = inblock % SFS_INDIRECT_BLOCK;
	    inblock = inblock / SFS_INDIRECT_BLOCK;
	    free_indirect(fd, sb, ip, offset, inblock);
	    free_all_dindirect(fd, sb, ip, 0);
	} else if (blockno < (SFS_DIRECT_BLOCK_ENTRY
			+ (SFS_INDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK)
			      + (SFS_DINDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK * SFS_INDIRECT_BLOCK))) {
	    /* 二重間接ブロックの範囲内 */
	    blockno = blockno -
		(SFS_DIRECT_BLOCK_ENTRY + SFS_INDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK);

	    inblock = blockno / (SFS_DINDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK);
	    dinblock = (blockno % (SFS_DINDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK))
		/ SFS_INDIRECT_BLOCK;
	    offset = blockno % SFS_INDIRECT_BLOCK;
	    free_dindirect(fd, sb, ip, offset, dinblock, inblock);
	    free_all_dindirect(fd, sb, ip, inblock + 1);
	}
    }
    ip->i_size = newsize;
    ip->i_nblock = nblock;
    return (write_inode(fd, sb, ip));
}

static void free_indirect(int fd, struct sfs_superblock *sb, struct sfs_inode *ip,
		   int offset, int inblock)
{
    int i;
    struct sfs_indirect inbuf;

    if (offset != 0) {
	read_block(fd, ip->i_indirect[inblock], sb->blksize,
		   (B *) & inbuf);
	for (i = offset; i < SFS_INDIRECT_BLOCK; ++i) {
	    inbuf.in_block[i] = 0;
	}
	write_block(fd, ip->i_indirect[inblock], sb->blksize,
		    (B *) & inbuf);

	++inblock;
    }
    for (i = inblock; i < SFS_INDIRECT_BLOCK_ENTRY; ++i) {
	if (ip->i_indirect[i] > 0) {
	    free_block(fd, sb, ip->i_indirect[i]);
	    ip->i_indirect[i] = 0;
	}
    }
}

static void free_dindirect(int fd, struct sfs_superblock *sb, struct sfs_inode *ip,
		    int offset, int dinblock, int inblock)
{
    int i;
    struct sfs_indirect inbuf, inbuf2;

    if (ip->i_dindirect[inblock] <= 0) {
	return;
    }
    read_block(fd, ip->i_dindirect[inblock], sb->blksize, (B *) & inbuf);
    if (offset != 0) {
	read_block(fd, inbuf.in_block[dinblock], sb->blksize,
		   (B *) & inbuf2);
	for (i = offset; i < SFS_INDIRECT_BLOCK; ++i) {
	    inbuf2.in_block[i] = 0;
	}
	write_block(fd, inbuf.in_block[dinblock], sb->blksize,
		    (B *) & inbuf2);
	++dinblock;
    }
    for (i = dinblock; i < SFS_INDIRECT_BLOCK; ++i) {
	if (inbuf.in_block[i] > 0) {
	    free_block(fd, sb, inbuf.in_block[i]);
	    inbuf.in_block[i] = 0;
	}
    }
    if (dinblock > 0) {
	write_block(fd, ip->i_dindirect[inblock],
		    sb->blksize, (B *) & inbuf);
    }
}

static void free_all_dindirect(int fd, struct sfs_superblock *sb,
			struct sfs_inode *ip, int inblock)
{
    int i;

    for (i = inblock; i < SFS_DINDIRECT_BLOCK_ENTRY; ++i) {
	if (ip->i_dindirect[i] > 0) {
	    free_dindirect(fd, sb, ip, 0, 0, i);
	    free_block(fd, sb, ip->i_dindirect[i]);
	    ip->i_dindirect[i] = 0;
	}
    }
}


/* ブロックに関係している処理

 * read_block()
 * write_block()
 * alloc_block()
 * get_block_num()
 * set_block_num()
 *
 */
static int read_block(int fd, int blockno, int blocksize, B * buf)
{
    if (lseek(fd, blockno * blocksize, 0) < 0) {
	return (0);
    }
    if (read(fd, buf, blocksize) < blocksize) {
	return (0);
    }
    return (blocksize);
}


static int write_block(int fd, int blockno, int blocksize, B * buf)
{
    if (lseek(fd, blockno * blocksize, 0) < 0) {
	return (0);
    }
    if (write(fd, buf, blocksize) < blocksize) {
	return (0);
    }
    return (blocksize);
}

static int alloc_block(int fd, struct sfs_superblock *sb)
{
    int startoffset;
    int i, j, k, s;
    char *buf;
    int free_block;
    unsigned char mask;


    if (sb->freeblock <= 0) {
	fprintf(stderr, "cannot allocate block\n");
	return (-1);
    }
    startoffset = ((1 + 1) * sb->blksize);
    lseek(fd, startoffset, 0);
    buf = alloca(sb->blksize);
    s = (sb->bsearch - 1) / (8 * sb->blksize);
    for (i = s; i < sb->bitmapsize; i++) {
	lseek(fd, (i * sb->blksize) + startoffset, 0);
	if (read(fd, buf, sb->blksize) < 0) {
	    return (-1);
	}
	if (i == s)
	    j = ((sb->bsearch - 1) / 8) % sb->blksize;
	else
	    j = 0;
	for (; j < sb->blksize; j++) {
	    if ((buf[j] & 0xff) != 0xff) {
		mask = 1;
		for (k = 0; k < 8; k++) {
		    if ((mask & buf[j]) != mask) {
			free_block = (i * sb->blksize * 8)
			    + (j * 8)
			    + k;
			buf[j] = buf[j] | mask;
			lseek(fd, (i * sb->blksize) + startoffset, 0);
			if (write(fd, buf, sb->blksize) < 0) {
			    fprintf(stderr, "write fail\n");
			    return (-1);
			}
			sb->freeblock--;
			sb->bsearch = free_block;
			lseek(fd, 1 * sb->blksize, 0);
			write(fd, sb, sizeof(struct sfs_superblock));
			return (free_block);
		    }
		    mask = mask << 1;
		}
	    }
	}
    }
    return (-1);
}


static int free_block(int fd, struct sfs_superblock *sb, int blockno)
{
    unsigned char block;
    int startoffset;
    int mask;

    startoffset = ((1 + 1) * sb->blksize) + (blockno / 8);
    lseek(fd, startoffset, 0);
    read(fd, &block, 1);
    mask = 0x01;
    mask = mask << (blockno % 8);
    block = block & ((~mask) & 0xff);
    lseek(fd, startoffset, 0);
    write(fd, &block, 1);

    sb->freeblock++;
    if (sb->bsearch >= blockno && blockno > 0)
	sb->bsearch = blockno - 1;
    lseek(fd, 1 * sb->blksize, 0);
    write(fd, sb, sizeof(struct sfs_superblock));
    return (0);
}


static int get_block_num(int fd,
		  struct sfs_superblock *sb,
		  struct sfs_inode *ip,
		  int blockno)
{
    if (blockno < SFS_DIRECT_BLOCK_ENTRY) {
	/* 直接ブロックの範囲内
	 */
	return (ip->i_direct[blockno]);
    } else if (blockno < (SFS_DIRECT_BLOCK_ENTRY
		    + (SFS_INDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK))) {
	/* 一重間接ブロックの範囲内
	 */
	return (get_indirect_block_num(fd, sb, ip, blockno));
    } else if (blockno < (SFS_DIRECT_BLOCK_ENTRY
			+ (SFS_INDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK)
			  + (SFS_DINDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK * SFS_INDIRECT_BLOCK))) {
	/* 二重間接ブロックの範囲内
	 */
	return (get_dindirect_block_num(fd, sb, ip, blockno));
    }
    return (-1);
}


static int get_indirect_block_num(int fd, struct sfs_superblock *sb, struct sfs_inode *ip, int blockno)
{
    int inblock;
    int inblock_offset;
    struct sfs_indirect inbuf;

    inblock = (blockno - SFS_DIRECT_BLOCK_ENTRY);
    inblock_offset = inblock % SFS_INDIRECT_BLOCK;
    inblock = inblock / SFS_INDIRECT_BLOCK;
    if (ip->i_indirect[inblock] <= 0) {
	return (0);
    }
    read_block(fd, ip->i_indirect[inblock], sb->blksize, (B *) & inbuf);
    return (inbuf.in_block[inblock_offset]);
}

static int get_dindirect_block_num(int fd, struct sfs_superblock *sb, struct sfs_inode *ip, int blockno)
{
    int dinblock;
    int dinblock_offset;
    int inblock;
    struct sfs_indirect inbuf;

    blockno = blockno - (SFS_DIRECT_BLOCK_ENTRY + SFS_INDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK);

    inblock = blockno / (SFS_DINDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK);
    dinblock = (blockno % (SFS_DINDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK)) / SFS_INDIRECT_BLOCK;
    dinblock_offset = blockno % SFS_INDIRECT_BLOCK;

    if (ip->i_dindirect[inblock] <= 0) {
	return (0);
    }
    read_block(fd, ip->i_dindirect[inblock], sb->blksize, (B *) & inbuf);
    if (inbuf.in_block[dinblock] <= 0) {
	return (0);
    }
    read_block(fd, inbuf.in_block[dinblock], sb->blksize, (B *) & inbuf);

    return (inbuf.in_block[dinblock_offset]);
}


static int set_block_num(int fd,
		  struct sfs_superblock *sb,
		  struct sfs_inode *ip,
		  int blockno,
		  int newblock)
{
    if (newblock < 0) {
	return (-1);
    }
    if (blockno < (SFS_DIRECT_BLOCK_ENTRY)) {
	/* 直接ブロックの範囲内
	 */
	ip->i_direct[blockno] = newblock;
	return (ip->i_direct[blockno]);
    } else if (blockno < (SFS_DIRECT_BLOCK_ENTRY
		    + (SFS_INDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK))) {
	/* 一重間接ブロックの範囲内
	 */
	return (set_indirect_block_num(fd, sb, ip, blockno, newblock));
    } else if (blockno < (SFS_DIRECT_BLOCK_ENTRY
			+ (SFS_INDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK)
			  + (SFS_DINDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK * SFS_INDIRECT_BLOCK))) {
	/* 二重間接ブロックの範囲内
	 */
	return (set_dindirect_block_num(fd, sb, ip, blockno, newblock));
    }
    return (-1);
}



static int set_indirect_block_num(int fd,
			   struct sfs_superblock *sb,
			   struct sfs_inode *ip,
			   int blockno,
			   int newblock)
{
    int inblock;
    int inblock_offset;
    struct sfs_indirect inbuf;
    int newinblock;

    inblock = (blockno - SFS_DIRECT_BLOCK_ENTRY);
    inblock_offset = inblock % SFS_INDIRECT_BLOCK;
    inblock = inblock / SFS_INDIRECT_BLOCK;
    if (ip->i_indirect[inblock] <= 0) {
	newinblock = alloc_block(fd, sb);
	ip->i_indirect[inblock] = newinblock;
	bzero((B *) & inbuf, sizeof(inbuf));
    } else {
	read_block(fd, ip->i_indirect[inblock], sb->blksize, (B *) & inbuf);
    }

    inbuf.in_block[inblock_offset] = newblock;
    write_block(fd, ip->i_indirect[inblock], sb->blksize, (B *) & inbuf);
    write_inode(fd, sb, ip);

    return (inbuf.in_block[inblock_offset]);
}


static int set_dindirect_block_num(int fd,
			    struct sfs_superblock *sb,
			    struct sfs_inode *ip,
			    int blockno, int newblock)
{
    int dinblock;
    int dinblock_offset;
    int inblock;
    struct sfs_indirect inbuf;	/* 一番目の間接ブロックの情報 */
    struct sfs_indirect dinbuf;	/* 二番目の間接ブロックの情報 */
    int newinblock;
    int newdinblock;

    blockno = blockno - (SFS_DIRECT_BLOCK_ENTRY + (SFS_INDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK));

    inblock = blockno / (SFS_DINDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK);
    dinblock = (blockno % (SFS_DINDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK)) / SFS_INDIRECT_BLOCK;
    dinblock_offset = blockno % SFS_INDIRECT_BLOCK;

    bzero(&inbuf, sizeof(inbuf));
    bzero(&dinbuf, sizeof(dinbuf));
    if (ip->i_dindirect[inblock] <= 0) {
	/* 一重目の間接ブロックの更新(アロケート)
	 */
	newinblock = alloc_block(fd, sb);
	ip->i_dindirect[inblock] = newinblock;
	bzero((B *) & inbuf, sizeof(inbuf));
    } else {
	read_block(fd, ip->i_dindirect[inblock], sb->blksize, (B *) & inbuf);
    }

    if (inbuf.in_block[dinblock] <= 0) {
	/* 二番目の間接ブロックの更新
	 * (アロケート)
	 */
	newdinblock = alloc_block(fd, sb);
	inbuf.in_block[dinblock] = newdinblock;
	bzero((B *) & dinbuf, sizeof(dinbuf));
    } else {
	read_block(fd, inbuf.in_block[dinblock], sb->blksize, (B *) & dinbuf);
    }

    dinbuf.in_block[dinblock_offset] = newblock;

    write_block(fd, ip->i_dindirect[inblock], sb->blksize, (B *) & inbuf);
    write_block(fd, inbuf.in_block[dinblock], sb->blksize, (B *) & dinbuf);
    write_inode(fd, sb, ip);

    return (newblock);
}
