	/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/* sfs_dir.c - SFS のディレクトリに関係する処理を行う。
 *
 * $Log: sfs_dir.c,v $
 * Revision 1.11  2000/06/01 08:47:24  naniwa
 * to implement getdents
 *
 * Revision 1.10  1999/05/30 04:05:32  naniwa
 * modified to creat file correctly
 *
 * Revision 1.9  1999/05/28 15:48:34  naniwa
 * sfs ver 1.1
 *
 * Revision 1.8  1999/05/10 15:46:16  night
 * ディレクトリ内容の書き込みをする処理で、sfs_i_read () を呼びだしていた
 * のを sfs_i_write () を呼び出すように変更。
 *
 * Revision 1.7  1999/04/13 04:15:26  monaka
 * MAJOR FIXcvs commit -m 'MAJOR FIX!!! There are so many changes, modifys, fixes. Sorry but I can't remember all of those. For example, all the manager and driver programmer have got power to access all ITRON systemcall. (My works is just making access route to ITRON. I don't know what happens in the nuclus.'! There are so many changes, modifys, fixes. Sorry but I can't remember all of those. For example, all the manager and driver programmer have got power to access all ITRON systemcall. (My works is just making access route to ITRON. I don't know what happens in the nuclus.
 *
 * Revision 1.6  1999/03/24 04:52:16  monaka
 * Source file cleaning for avoid warnings.
 *
 * Revision 1.5  1997/07/04 15:07:41  night
 * ・スペシャルファイル - デバイスドライバポートの対応表の関連処理の追加。
 * ・ファイルの読み込み処理の改訂。
 *
 * Revision 1.4  1997/07/03 14:24:35  night
 * mountroot/open 処理のバグを修正。
 *
 * Revision 1.3  1996/11/27 15:42:08  night
 * sfs_write_dir() を追加
 *
 * Revision 1.2  1996/11/18 13:46:10  night
 * ファイルの中身を作成。
 *
 * Revision 1.1  1996/11/17  14:52:57  night
 * 最初の登録
 *
 *
 */

#include <fcntl.h>
#include <string.h>
#include <fs/sfs.h>
#include <nerve/kcall.h>
#include <sys/dirent.h>
#include "../fs.h"
#include "func.h"


/* ディレクトリに関係する処理
 *
 * sfs_read_dir()
 * sfs_write_dir()
 *
 */


/* sfsread_dir -
 *
 */
W
sfs_read_dir (struct inode *parentp,
	      W nentry,
	      struct sfs_dir *dirp)
{
  W	size;
  W	error_no;
  W	rsize;

#ifdef FMDEBUG
  dbg_printf ("sfs: sfs_read_dir: start. parent = 0x%x, nentry = %d\n", parentp, nentry);
#endif

  if ((nentry <= 0) || (dirp == NULL))
    {
      return (parentp->i_size / sizeof (struct sfs_dir));
    }
  
  size = ((nentry * sizeof (struct sfs_dir)) <= parentp->i_size) ? 
          nentry * sizeof (struct sfs_dir) :
	  parentp->i_size;

  error_no = sfs_i_read (parentp, 0, (char *)dirp, size, &rsize);
  if (error_no)
    {
      return (error_no);
    }
  return (0);
}


/* sfs_write_dir - ディレクトリに新しい要素を追加
 *
 */
W
sfs_write_dir (struct inode *parentp,
	      W nentry,
	      struct sfs_dir *dirp)
{
  W	error_no;
  W	rsize;

  /* 親ディレクトリの netnry 目から後に dirp の内容を追加 */
  error_no = sfs_i_write (parentp, nentry*sizeof(struct sfs_dir), (char *)dirp,
		       sizeof(struct sfs_dir), &rsize);
  if (error_no)
    {
      return (error_no);
    }
  return (0);
}

/*
 * sfs_getdetns()
 */
int sfs_getdents(struct inode *ip, ID caller, W offset,
	       VP buf, UW length, W *rsize, W *fsize)
{
  W nentry, i, s, error_no, len;
  struct dirent dent;

  *rsize = 0; *fsize = 0;
  dent.d_ino = ip->i_index;
  nentry = sfs_read_dir (ip, 0, NULL);
  s = sizeof(struct sfs_dir);
  if (offset >= nentry*s) return(EOK);
  {
    struct sfs_dir dirp[nentry]; /* GCC の拡張機能を使っている */
    error_no = sfs_read_dir (ip, nentry, dirp);
    if (error_no) return(error_no);
    for (i = offset/s; i < nentry; i++) {
      len = sizeof(struct dirent)+strlen(dirp[i].d_name);
      if ((*rsize) + len >= length) return(EOK);
      dent.d_reclen = len;
      dent.d_off = i*s;
      strncpy(dent.d_name, dirp[i].d_name, MAX_NAME_LEN);
      dent.d_name[MAX_NAME_LEN] = '\0';
      error_no = kcall->region_put(caller, buf+(*rsize), len, &dent);
      if (error_no) return(error_no);
      *rsize += len;
      *fsize += s;
    }
  }
  return(EOK);
}


/* sfs_i_lookup - ファイルの探索
 *
 * 引数 parent (親ディレクトリ) の指すディレクトリからファイルを
 * 探索する。
 *
 */
int
sfs_i_lookup(struct inode *parent,
	     char *fname,
	     W oflag,
	     W mode, struct permission *acc, struct inode **retip)
{
    W error_no;
    W nentry;
    W i;
#ifdef FMDEBUG
    dbg_printf("sfs: sfs_i_lookup: start. fname = %s\n", fname);	/* */
#endif
    if (strcmp(fname, ".") == 0) {
	*retip = parent;
	(*retip)->i_refcount++;
	return (EOK);
    }

    nentry = sfs_read_dir(parent, 0, NULL);
    {
	struct sfs_dir dirp[nentry];

	error_no = sfs_read_dir(parent, nentry, dirp);
	for (i = 0; i < nentry; i++) {
	    /* 表示文字長を SFS_MAXNAMELEN にするため．後に pad があるので大丈夫 */
	    if ((W) strncmp(fname, dirp[i].d_name, SFS_MAXNAMELEN + 1)
		== 0) {
		break;
	    }
	}
#ifdef FMDEBUG
	dbg_printf
	    ("sfs: sfs_i_lookup: called sfs_read_dir(). i = %d, nentry = %d\n",
	     i, nentry);
#endif
	if (i >= nentry) {
	    return (ENOENT);
	}

	*retip = fs_get_inode(parent->i_fs, dirp[i].d_index);
	if (*retip) {
	    /* すでにオープンしていたファイルだった
	     */
	    if ((*retip)->coverfile) {
		*retip = (*retip)->coverfile;
	    }
	    (*retip)->i_refcount++;
	    return (EOK);
	}

	*retip = alloc_inode(parent->i_fs);
	if (*retip == NULL) {
	    return (ENOMEM);
	}

	error_no = sfs_read_inode(parent->i_fs, dirp[i].d_index, *retip);
	if (error_no) {
	    return (error_no);
	}
	fs_register_inode(*retip);
    }

    return (EOK);
}


int sfs_i_close()
{
    return (EOK);
}


int sfs_i_link(struct inode * parent, char *fname, struct inode * srcip,
	     struct permission * acc)
{
    W error_no;
    struct sfs_dir dirent;
    W dirnentry;
    struct inode *ip;

    /* リンク先にファイルが存在していたらエラー */
    error_no = fs_lookup(parent, fname, O_RDONLY, 0, acc, &ip);
    if (error_no == EOK) {
	fs_close_file(ip);
	return (EEXIST);
    }

    /* ディレクトリのエントリを作成 */
    dirent.d_index = srcip->i_index;
    /* 表示文字長を SFS_MAXNAMELEN にするため．後に pad があるので大丈夫 */
    strncpy(dirent.d_name, fname, SFS_MAXNAMELEN);
    dirent.pad[0] = '\0';

    /* ディレクトリにエントリを追加 */
    dirnentry = sfs_read_dir(parent, 0, NULL);
    error_no = sfs_write_dir(parent, dirnentry, &dirent);
    if (error_no != EOK) {
	return (error_no);
    }

    /* inode 情報の更新 */
    struct sfs_inode *sfs_inode = srcip->i_private;
    sfs_inode->i_nlink += 1;
    time_get(&(sfs_inode->i_ctime));
    srcip->i_dirty = 1;

    /* 本来は inode の deallocate のところで行う処理のはず */
    error_no = sfs_i_sync(srcip);
    if (error_no != EOK) {
	return (error_no);
    }
    return (EOK);
}


int
sfs_i_unlink(struct inode * parent, char *fname, struct permission * acc)
{
    int nentry;
    int i;
    struct inode *ip;
    W rsize, error_no;

    error_no = fs_lookup(parent, fname, O_RDWR, 0, acc, &ip);
    if (error_no) {
	return (error_no);
    }
    if ((ip->i_mode & S_IFMT) == S_IFDIR) {
	fs_close_file(ip);
	return (EISDIR);
    }

    /* ファイルの名前の最後の１つで，使用中なら削除しない */
    struct sfs_inode *sfs_inode = ip->i_private;
    if ((sfs_inode->i_nlink == 1) && (ip->i_refcount >= 2)) {
	fs_close_file(ip);
	return (EBUSY);
    }

    nentry = sfs_read_dir(parent, 0, NULL);
    if (nentry <= 0) {
	fs_close_file(ip);
	return (ENOENT);
    } {
	struct sfs_dir buf[nentry];	/* GCC の拡張機能を使っている */
	if (sfs_read_dir(parent, nentry, buf) != 0) {
	    fs_close_file(ip);
	    return (EIO);
	}

	for (i = 0; i < nentry; i++) {
	    /* 表示文字長を SFS_MAXNAMELEN にするため．後に pad があるので大丈夫 */
	    if (strncmp(fname, buf[i].d_name, SFS_MAXNAMELEN + 1) == 0) {
		break;
	    }
	}
	if (i >= nentry) {
	    fs_close_file(ip);
	    return (ENOENT);
	}

	while (i < nentry) {
	    buf[i].d_index = buf[i + 1].d_index;
	    /* 表示文字長を SFS_MAXNAMELEN にするため．後に pad があるので大丈夫 */
	    strncpy(buf[i].d_name, buf[i + 1].d_name,
		    SFS_MAXNAMELEN);
	    buf[i].pad[0] = '\0';
	    i++;
	}
	i = parent->i_size - sizeof(struct sfs_dir);
	sfs_i_write(parent, 0, (B *) buf, i, &rsize);
	parent->i_dirty = 1;
	sfs_i_truncate(parent, i);

	sfs_inode->i_nlink--;
	time_get(&(sfs_inode->i_ctime));
	ip->i_dirty = 1;
	if (sfs_inode->i_nlink <= 0) {
	    sfs_i_truncate(ip, 0);
	    sfs_free_inode(ip->i_fs, ip);
	}
    }
    fs_close_file(ip);
    return (EOK);
}


/*
 * ディレクトリを作成する。
 *
 * 1) 新しい inode をアロケート。
 * 2) 親ディレクトリにアロケートした新しい inode の情報を追加。
 *
 */
int
sfs_i_mkdir(struct inode * parent,
	    char *fname,
	    W mode, struct permission * acc, struct inode ** retip)
{
    struct inode *newip;
    W error_no;
    struct sfs_dir dirent;
    W dirnentry;
    W i_index;
    W rsize;
    static struct sfs_dir dir[2] = {
	{0, "."},
	{0, ".."}
    };
    SYSTIM clock;

    /* 引数のチェック */
    newip = alloc_inode(parent->i_fs);
    if (newip == NULL) {
	return (ENOMEM);
    }
    *retip = newip;

    /* 新しい sfs_inode をアロケート */
    i_index = sfs_alloc_inode(parent->i_fs);
    if (i_index <= 0) {
	return (ENOMEM);
    }

    /* 設定 */
    struct sfs_inode *sfs_inode = newip->i_private;
    time_get(&clock);
    newip->i_fs = parent->i_fs;
    newip->i_refcount = 1;
    newip->i_dirty = 1;
    newip->i_mode = mode | S_IFDIR;
    sfs_inode->i_nlink = 2;
    newip->i_index = i_index;
    sfs_inode->i_uid = acc->uid;
    sfs_inode->i_gid = acc->gid;
    newip->i_dev = 0;
    newip->i_size = 0;
    sfs_inode->i_atime = clock;
    sfs_inode->i_ctime = clock;
    sfs_inode->i_mtime = clock;
    newip->i_nblock = 0;

    fs_register_inode(newip);

    dir[0].d_index = i_index;
    dir[1].d_index = parent->i_index;
    error_no = sfs_i_write(newip, 0, (B *) dir, sizeof(dir), &rsize);
    if (error_no) {
	return (error_no);
    }

    /* ディレクトリのエントリを作成 */
    dirent.d_index = newip->i_index;
    /* 表示文字長を SFS_MAXNAMELEN にするため．後に pad があるので大丈夫 */
    strncpy(dirent.d_name, fname, SFS_MAXNAMELEN);
    dirent.pad[0] = '\0';

    /* ディレクトリにエントリを追加 */
    sfs_inode = parent->i_private;
    sfs_inode->i_nlink += 1;
    dirnentry = sfs_read_dir(parent, 0, NULL);
    error_no = sfs_write_dir(parent, dirnentry, &dirent);
    if (error_no != EOK) {
	return (error_no);
    }

    return (EOK);
}

/*
 * ディレクトリを削除する。
 *
 */
int sfs_i_rmdir(struct inode * parent, char *fname, struct permission * acc)
{
    int nentry;
    int i;
    struct inode *ip;
    W rsize, error_no;
    SYSTIM clock;

    error_no = fs_lookup(parent, fname, O_RDWR, 0, acc, &ip);
    if (error_no) {
#ifdef FMDEBUG
        dbg_printf("sfs: sfs_i_rmdir: can't remove directory %s(%d)\n", fname,
	     error_no);
#endif
	return (error_no);
    }
    if ((ip->i_mode & S_IFMT) != S_IFDIR) {
	fs_close_file(ip);
	return (ENOTDIR);
    }
    if (ip->i_refcount >= 2) {
	fs_close_file(ip);
	return (EBUSY);
    }
    nentry = sfs_read_dir(ip, 0, NULL);
    if (nentry >= 3) {
	fs_close_file(ip);
	return (ENOTEMPTY);
    }

    nentry = sfs_read_dir(parent, 0, NULL);
    if (nentry <= 0) {
	fs_close_file(ip);
	return (ENOENT);
    } {
	struct sfs_dir buf[nentry];	/* GCC の拡張機能を使っている */
	if (sfs_read_dir(parent, nentry, buf) != 0) {
	    fs_close_file(ip);
	    return (EIO);
	}

	for (i = 0; i < nentry; i++) {
	    /* 表示文字長を SFS_MAXNAMELEN にするため．後に pad があるので大丈夫 */
	    if (strncmp(fname, buf[i].d_name, SFS_MAXNAMELEN + 1) == 0) {
		break;
	    }
	}
	if (i >= nentry) {
	    fs_close_file(ip);
	    return (ENOENT);
	}

	while (i < nentry) {
	    buf[i].d_index = buf[i + 1].d_index;
	    /* 表示文字長を SFS_MAXNAMELEN にするため．後に pad があるので大丈夫 */
	    strncpy(buf[i].d_name, buf[i + 1].d_name,
		    SFS_MAXNAMELEN);
	    buf[i].pad[0] = '\0';
	    i++;
	}
	i = parent->i_size - sizeof(struct sfs_dir);
	sfs_i_write(parent, 0, (B *) buf, i, &rsize);
	parent->i_dirty = 1;
	sfs_i_truncate(parent, i);

	struct sfs_inode *sfs_inode = ip->i_private;
	sfs_inode->i_nlink--;
	time_get(&clock);
	sfs_inode->i_ctime = clock;
	ip->i_dirty = 1;
	if (sfs_inode->i_nlink <= 1) {
	    sfs_i_truncate(ip, 0);
	    sfs_free_inode(ip->i_fs, ip);
	}
	sfs_inode = parent->i_private;
	sfs_inode->i_nlink -= 1;
	sfs_inode->i_ctime = clock;
	parent->i_dirty = 1;
    }
    fs_close_file(ip);
    return (EOK);
}
