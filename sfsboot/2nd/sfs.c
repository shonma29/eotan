#include "lib.h"
#include "sfs.h"

int device_id;
int (*device_read_func)();

struct sfs_superblock super_block;
char block_buff[SFS_BLOCK_SIZE];
struct sfs_indirect indirect_block_buff;

extern int fd_read();
 

/*********************************************************************
 * read_dev
 * ブートデバイスから、ブロック単位でデータを読み出す
 *   start  : 開始ブロック
 *   blockn : ブロック数
 *   buff   : 読みだし先
 */

int
read_dev(int start, int blockn, char* buff)
{
  int device;
  int drive;
  int partition;

  device = (device_id >> 16) & 0xff;
  drive = (device_id >> 8) & 0xff;
  partition = device_id & 0xff;

  if(device == 0)
    device_read_func = fd_read;

  if((*device_read_func)(drive, partition, start, buff, blockn) == -1)
    return -1;

  return 0;
}

/*********************************************************************
 * mount_sfs
 * ブートデバイスをマウントする
 *   dev  :  デバイス番号
 */

int
mount_sfs(int dev)
{
  device_id = dev;

  if(read_dev(1, 1, block_buff) == -1)
    return -1;

  memcpy((char*)&super_block, (char*)block_buff, sizeof(struct sfs_superblock));

  if(super_block.sfs_magic != SFS_MAGIC)           /* マジックナンバーのチェック */
    return -1;

  indirect_block_buff.current = 0;                 /* 間接ブロックバッファの初期化 */

  return 0;
}

/*********************************************************************
 * read_inode
 * i-nodeを取り出す
 *   inodeno : i-node番号
 *   inode   : コピー先
 */

int
read_inode(int inodeno, struct sfs_inode* inode)
{
  int blockno;
  int offset;
 
  offset = (inodeno -1) * sizeof(struct sfs_inode); /* i-nodeブロックの先頭からのオフセット */
  blockno = offset / SFS_BLOCK_SIZE + 2 
    + super_block.sfs_bitmapsize;                   /* i-nodeが含まれているブロック番号 */

  if(read_dev(blockno, 1, block_buff) == -1)
    return -1;

  offset = offset % SFS_BLOCK_SIZE;                 /* ブロック内でのi-nodeのオフセット */ 
  memcpy((char*)inode, block_buff + offset, sizeof(struct sfs_inode));
  return 0;
}

/*********************************************************************
 * read_file_block
 * ブロック単位でファイルを読み出す
 *   inode   : ファイルのi-node
 *   blockno : ブロック番号
 *   buff    : 読みだし先
 */

int
read_file_block(struct sfs_inode* inode, int blockno, char* buff)
{
  int size; 
  int indirect_block;

  if(inode->sfs_i_size_blk <= blockno)
    return -1;

  if(blockno < SFS_DIRECT_BLOCK_ENTRY)                                       /* 直接ブロック */
    return(read_dev(inode->sfs_i_direct[blockno], 1, buff));
  
  blockno = blockno - SFS_DIRECT_BLOCK_ENTRY;

  if(blockno < SFS_INDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK) {              /* 間接ブロック */
    indirect_block = inode->sfs_i_indirect[blockno / SFS_INDIRECT_BLOCK]; 

    if(indirect_block_buff.current != indirect_block) {
      if(read_dev(indirect_block, 1, (char*)indirect_block_buff.sfs_in_block) == -1)
	return -1;
      indirect_block_buff.current = indirect_block;
    }

    size = read_dev(indirect_block_buff.sfs_in_block[blockno % SFS_INDIRECT_BLOCK],
		    1, buff);
    return size;
  }

  return -1;                                                                  /* ２重間接ブロック */
}

/*********************************************************************
 * read_file
 * バイト単位でファイルを読み出す
 *   inode : ファイルのi-node
 *   start : 読みだし位置
 *   size  : サイズ
 *   buff  : 読みだし先    
 */

int
read_file(struct sfs_inode* inode, int start , int size, char* buff)
{
  int copysize;
  int offset;
  int retsize;

  if(start + size > inode->sfs_i_size)
    size = inode->sfs_i_size - start;

  retsize = size;

  while(size > 0) {
    if(read_file_block(inode, start / SFS_BLOCK_SIZE, block_buff) == -1)
      return -1;
    offset = start % SFS_BLOCK_SIZE;
    copysize = (((SFS_BLOCK_SIZE - offset) > size) ? size : (SFS_BLOCK_SIZE - offset));
    memcpy(buff, &block_buff[offset], copysize);

    buff = buff + copysize;
    start = start + copysize;
    size = size - copysize;
  }

  return 0;
}

/*********************************************************************
 * lookup_file_local
 * lookup_fileから呼ばれる。ディレクトリー内のファイルのi-nodeを返す
 *   filename     : ファイル名
 *   parent_inode : parent directryのi-node 
 *   inode        : filenameのi-nodeのコピー先
 */

int
lookup_file_local(char* filename, struct sfs_inode* parent_inode, struct sfs_inode* inode)
{
  int bn;
  int i;
  struct sfs_dir* dirp;

  for(bn = 0; bn < parent_inode->sfs_i_size_blk; bn++) {
    if(read_file_block(parent_inode, bn, block_buff) == -1) 
      return -1;
    
    dirp = (struct sfs_dir*)block_buff;

    for(i = 0; i < (SFS_BLOCK_SIZE / sizeof(struct sfs_dir)); i++) {
      if(strcmp(filename, dirp[i].sfs_d_name) == 0) {
	if(read_inode(dirp[i].sfs_d_index, inode) == -1)
	  return -1;
	return 0;
      }
    }
  }
  return -1;
}

/*********************************************************************
 * lookup_file
 * ファイルのi-nodeを返す
 *   path  : ファイルパス
 *   inode : i-nodeのコピー先
 */

int
lookup_file(char* path, struct sfs_inode* inode)
{
  int i;
  char name[SFS_MAXNAMELEN + 1];
  struct sfs_inode parent_inode;

  if(read_inode(1, &parent_inode) == -1)                            /* '/'のi-nodeを得る */
    return -1;


  if(strcmp(path, "/") == 0) {
    memcpy((char*)inode, (char*)&parent_inode, sizeof(struct sfs_inode));
    return 0;
  }

  while(*path != '\0') {

    if(*path == '/')
      path = path + 1;
 
    for(i = 0; ; i++) {
      if(i > SFS_MAXNAMELEN)
	return -1;
      if((*path == '/') || (*path == '\0'))
	break;
      name[i] = *path;
      path = path + 1;
    }

    if(i == 0)
      break;

    name[i] = '\0';

    if(lookup_file_local(name, &parent_inode, inode) == -1) 
      return -1; 
    
    memcpy((char*)&parent_inode, (char*)inode, sizeof(struct sfs_inode));
  }
  
  return 0;
}
    

