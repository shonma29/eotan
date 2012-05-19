/*********************************************************************
 * mkmap.c
 *  このプログラムは、statfs.c を元に作成されており、
 *  sfs上でのファイルのマップを標準出力に出力します。
 * 
 * 書式 :
 *       mkmap device filename 
 *
 * 出力 :  
 *       ブロックサイズ
 *       連続ブロック数 開始ブロック 
 *               :         
 *               :
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include "../include/itron/types.h"
#include "../servers/fs/sfs/sfs_fs.h"

#define BLOCKSIZE 512
#define RDONLY			1
#define RDWR			2

#define ROUNDUP(x,align)	(((((int)x) + ((align) - 1))/(align))*(align))
#define MIN(x,y)		((x > y) ? y : x)

struct sfs_inode	rootdir_buf;
struct sfs_inode	*rootdirp;

int write_flagment(int mapfd, int startblock, short int count);

/*********************************************************************
 * main
 *
 *
 */

int
main(int argc, char** argv) 
{

  int fd;
  int errno;
  struct sfs_superblock sb;
  struct sfs_inode ip;
  
  int block;
  int blocknum;
  int startblock;
  int count;


  if(argc != 3) {
    fprintf(stderr, "Usage : mkmap device filename \n");
    exit(1);
  }
  
  /* mount device */
  fd = mount_fs(argv[1], &sb, &rootdir_buf, RDONLY);
  if(fd == -1) {
    fprintf(stderr, "cannot mount device\n");
    exit(1);
  }
  rootdirp = &rootdir_buf;

  /* lookup file */
  errno = lookup_file (fd, &sb, rootdirp, argv[2], &ip);
  if (errno) {
      fprintf (stderr, "cannot open file.\n");
      exit(1);
  }
  
  printf("%d\n", sb.sfs_blocksize);                 /* blocksize 現在は512byte */                  
  
  startblock = get_block_num(fd, &sb, &ip, 0);      /* 連続するブロックをカウントする */
  count = 0;
  for(block = 0; block < ip.sfs_i_size_blk; block++) {
    blocknum = get_block_num(fd, &sb, &ip, block);
    
    if((startblock + count) != blocknum) {
      printf("%d %d\n", startblock, count);
      startblock = blocknum;
      count = 1;
    }
    else {
      count = count + 1;
    }
  }
  printf("%d %d\n", startblock, count);

  exit(0);

}

/*********************************************************************
 * ファイルシステムに関係する処理
 *
 *
 */ 

int
mount_fs (char *path, struct sfs_superblock *sb, struct sfs_inode *root, int mode)
{
  int	fd;

  if (mode == RDONLY)
    {
      fd = open (path, O_RDONLY);
    }
  else
    {
      fd = open (path, O_RDWR);
    }

  if (fd < 0)
    {
      fprintf (stderr, "Cannot open file.\n");
      return (-1);
    }

  if (lseek (fd, BLOCKSIZE, 0) < 0)
    {
      return (-1);
    }

  if (read (fd, sb, sizeof (struct sfs_superblock)) != sizeof (struct sfs_superblock))
    {
      return (-1);
    }

  read_inode (fd, sb, 1, root);
  return (fd);  
}


/*********************************************************************
 * i-node に関係している処理
 *
 *
 */


int
get_inode_offset (struct sfs_superblock *sb, int ino)
{
  int	offset;
  int	nblock;
  int	blocksize;

  nblock = sb->sfs_nblock;
  blocksize = sb->sfs_blocksize;
  offset = 1 + 1 + (ROUNDUP (nblock / 8, blocksize) / blocksize);
  offset *= blocksize;
  return (offset + ((ino - 1) * sizeof (struct sfs_inode)));
}
  
int
read_inode (int fd, struct sfs_superblock *sb, int ino, struct sfs_inode *ip)
{
  int	offset;
  offset = get_inode_offset (sb, ino);
  lseek (fd, offset, 0);
  read (fd, ip, sizeof (struct sfs_inode));
  return (0);
}

/*********************************************************************
 * directry に関係する処理
 *
 *
 */

int
read_dir (int fd,
	  struct sfs_superblock *sb,
	  struct sfs_inode *ip,
	  int nentry,
	  struct sfs_dir *dirp)
{
  int	size;

  if ((nentry <= 0) || (dirp == NULL))
    {
      return (ip->sfs_i_size / sizeof (struct sfs_dir));
    }
  size = (nentry * sizeof (struct sfs_dir) <= ip->sfs_i_size) ? 
          nentry * sizeof (struct sfs_dir) :
	  ip->sfs_i_size;

  read_file (fd, sb, ip, 0, size, (B *)dirp);	/* エラーチェックが必要! */
  return (0);
}


/*********************************************************************
 * file に関係している処理
 *
 *
 */
int
read_file (int fd,
	   struct sfs_superblock *sb,
	   struct sfs_inode *ip,
	   int start,
	   int size,
	   B *buf)
{
  B	*blockbuf;
  int	copysize;
  int	offset;
  int	retsize;
  char	*bufp;

  if (start + size > ip->sfs_i_size)
    {
      size = ip->sfs_i_size - start;
    }

  retsize = size;

  bufp = buf;

/*  fprintf (stderr, "read_file: offset = %d, size = %d\n", start, size); */
  blockbuf = (B *)alloca (sb->sfs_blocksize);
  while (size > 0)
    {
      read_block (fd, 
		  get_block_num (fd, sb, ip, start / sb->sfs_blocksize),
		  sb->sfs_blocksize,
		  blockbuf);
      offset = start % sb->sfs_blocksize;
      copysize = MIN (sb->sfs_blocksize - offset, size);
      bcopy (&blockbuf[offset], buf, copysize);

      buf += copysize;
      start += copysize;
      size -= copysize;
    }
  return (0);
}

    
int
lookup_file (int fd,
	     struct sfs_superblock *sb, 
	     struct sfs_inode *cwd,
	     char	*path,
	     struct sfs_inode *ip)
{
  char name[SFS_MAXNAMELEN + 1];
  struct sfs_inode	*dirp;
  struct sfs_inode	*pdirp;
  struct sfs_inode	dirbuf;
  int	i;

  if (strcmp (path, "/") == 0)
    {
      bcopy (cwd, ip, sizeof (struct sfs_inode));
      return (0);
    }

  if (*path == '/')
    {
      path++;
    }

  pdirp = cwd;
  dirp = &dirbuf;

  while (*path)
    {
      if (*path == '/')
	{
	  path++;
	}

      for (i = 0; ; i++)
	{
	  if (i > SFS_MAXNAMELEN)
	    {
	      return (ENAMETOOLONG);
	    }
	  if ((*path == '/') || (*path == '\0'))
	    {
	      break;
	    }
	  name[i] = *path++;
	}
      if (i == 0)
	break;

      name[i] = '\0';

/*      fprintf (stderr, "local lookup = %s\n", name); */
      errno = locallookup_file (fd, sb, pdirp, dirp, name);
      if (errno)
	{
	  return (errno);
	}

      pdirp = dirp;
      dirp = pdirp;
    }

  bcopy (pdirp, ip, sizeof (struct sfs_inode));
  return (0);
}


int
locallookup_file (int fd,
		   struct sfs_superblock *sb, 
		   struct sfs_inode *parent,
		   struct sfs_inode *ip,
		   char *name)
{
  int	nentry;
  struct sfs_dir *dirp;
  int	i;
  int	errno;

  nentry = read_dir (fd, sb, parent, 0, NULL);
  dirp = alloca (sizeof (struct sfs_dir) * nentry);
  read_dir (fd, sb, parent, nentry, dirp);
  for (i = 0; i < nentry; i++)
    {
      if (strcmp (name, dirp[i].sfs_d_name) == 0)
	{
	  errno = read_inode (fd, sb, dirp[i].sfs_d_index, ip);
	  if (errno)
	    {
	      return (errno);
	    }
	  return (0);
	}
    }
  return (ENOENT);      
}

/*********************************************************************
 * block に関係している処理
 *
 *
 */

int
read_block (int fd, int blockno, int blocksize, B *buf)
{
  if (lseek (fd, blockno * blocksize, 0) < 0)
    {
      return (0);
    }

  if (read (fd, buf, blocksize) < blocksize)
    {
      return (0);
    }

  return (blocksize);
}

int
get_block_num (int fd,
	       struct sfs_superblock *sb,
	       struct sfs_inode *ip,
	       int blockno)
{
  if (blockno < SFS_DIRECT_BLOCK_ENTRY)
    {
      /* 直接ブロックの範囲内
       */
      return (ip->sfs_i_direct[blockno]);
    }
  else if (blockno < (SFS_DIRECT_BLOCK_ENTRY 
  		      + (SFS_INDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK)))
    {
      /* 一重間接ブロックの範囲内
       */
      return (get_indirect_block_num (fd, sb, ip, blockno));
    }
  else if (blockno < (SFS_DIRECT_BLOCK_ENTRY 
  		      + (SFS_INDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK)
		      + (SFS_DINDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK * SFS_INDIRECT_BLOCK)))
    {
      /* 二重間接ブロックの範囲内
       */
      return (get_dindirect_block_num (fd, sb, ip, blockno));
    }

  return (-1);
}


int
get_indirect_block_num (int fd, struct sfs_superblock *sb, struct sfs_inode *ip, int blockno)
{
  int	inblock;
  int	inblock_offset;
  struct sfs_indirect	inbuf;

  inblock = (blockno - SFS_DIRECT_BLOCK_ENTRY);
  inblock_offset = inblock % SFS_INDIRECT_BLOCK;
  inblock = inblock / SFS_INDIRECT_BLOCK;
  if (ip->sfs_i_indirect[inblock] <= 0)
    {
      return (0);
    }

  read_block (fd, ip->sfs_i_indirect[inblock], sb->sfs_blocksize, (B*)&inbuf);
#ifdef notdef
  fprintf (stderr, "get_ind: inblock = %d, offset = %d, blocknum = %d\n",
	  inblock, inblock_offset, inbuf.sfs_in_block[inblock_offset]);
#endif
  return (inbuf.sfs_in_block[inblock_offset]);
}

int
get_dindirect_block_num (int fd, struct sfs_superblock *sb, struct sfs_inode *ip, int blockno)
{
  int	dinblock;
  int	dinblock_offset;
  int	inblock;
  struct sfs_indirect	inbuf;

  blockno = blockno - (SFS_DIRECT_BLOCK_ENTRY + SFS_INDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK);

  inblock = blockno / (SFS_DINDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK);
  dinblock = (blockno % (SFS_DINDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK)) / SFS_INDIRECT_BLOCK;
  dinblock_offset = blockno % SFS_INDIRECT_BLOCK;

#ifdef notdef
  fprintf (stderr, "GET: blockno = %d, inblock = %d, dinblock = %d, dinblock_offset = %d\n",
	   blockno, inblock, dinblock, dinblock_offset);
#endif
  if (ip->sfs_i_dindirect[inblock] <= 0)
    {
      return (0);
    }

  read_block (fd, ip->sfs_i_dindirect[inblock], sb->sfs_blocksize, (B*)&inbuf);
  if (inbuf.sfs_in_block[dinblock] <= 0)
    {
      return (0);
    }

  read_block (fd, inbuf.sfs_in_block[dinblock], sb->sfs_blocksize, (B*)&inbuf);

#ifdef notdef
  fprintf (stderr, "get_ind: inblock = %d, dinblock = %d, offset = %d, blocknum = %d\n",
	  inblock, dinblock, dinblock_offset, inbuf.sfs_in_block[dinblock_offset]);
#endif
  return (inbuf.sfs_in_block[dinblock_offset]);
}

