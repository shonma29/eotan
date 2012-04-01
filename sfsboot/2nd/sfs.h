#ifndef __SFS_H__
#define __SFS_H__	1

#define SFS_BLOCK_SIZE	512

#define SFS_MAGIC			0x12345678
#define SFS_MAXNAMELEN			14
#define SFS_VERSION_HI			1
#define SFS_VERSION_LO			1

#define SFS_DIRECT_BLOCK_ENTRY		70
#define SFS_INDIRECT_BLOCK_ENTRY	20
#define SFS_DINDIRECT_BLOCK_ENTRY 	26
#define SFS_TINDIRECT_BLOCK_ENTRY 	1
#define SFS_INDIRECT_BLOCK		128

struct sfs_superblock
{
  unsigned long	sfs_magic;
  short		sfs_version_hi;
  short		sfs_version_lo;
  unsigned long	sfs_mountcount;
  short		sfs_blocksize;		/* ブロックサイズ */

  unsigned long	sfs_nblock;
  unsigned long	sfs_freeblock;

  unsigned long	sfs_bitmapsize;

  unsigned long	sfs_ninode;
  unsigned long	sfs_freeinode;

  unsigned long	sfs_isearch;		/* この番号以下の inode は使用中 */
  unsigned long	sfs_bsearch;		/* この番号以下の block は使用中 */

  unsigned long	sfs_datablock;		/* データ領域の開始位置。
				 * inode は、スーパーブ
				 * ロックの直後になるので、
				 * 特に記録しない。*/
};

struct sfs_inode
{
  unsigned long	sfs_i_index;	/* SFS の場合、inode は 1 からはじまる */
  unsigned long	sfs_i_nlink;
  unsigned long	sfs_i_size;
  unsigned long	sfs_i_size_blk;
  
  unsigned long	sfs_i_perm;
  unsigned long	sfs_i_uid;
  unsigned long	sfs_i_gid;
  unsigned long	sfs_i_dev;	/* not used */

  unsigned long	sfs_i_atime;
  unsigned long	sfs_i_ctime;
  unsigned long	sfs_i_mtime;

  unsigned long	sfs_i_direct[SFS_DIRECT_BLOCK_ENTRY];
  unsigned long	sfs_i_indirect[SFS_INDIRECT_BLOCK_ENTRY];
  unsigned long	sfs_i_dindirect[SFS_DINDIRECT_BLOCK_ENTRY];
  unsigned long	sfs_i_tindirect[SFS_TINDIRECT_BLOCK_ENTRY]; /* reservation only */
};


struct sfs_indirect
{
  int current;
  unsigned long	sfs_in_block[SFS_INDIRECT_BLOCK];
};


struct sfs_dir
{
  unsigned long	sfs_d_index;		/* inode 番号 */
  unsigned char	sfs_d_name[SFS_MAXNAMELEN];
  unsigned char	pad[2];			/* padding */
};

int mount_sfs(int dev);
int read_file(struct sfs_inode* inode, int start , int size, char* buff);
int lookup_file(char* path, struct sfs_inode* inode);

#endif /* __SFS_H__ */
