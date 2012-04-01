/*********************************************************************
 * mkboot.c
 *  このプログラムは、1stbootに、 ディスクパラメータ、マップ情報を書き込みます。
 *  1.44M FDにしか対応していません。マップ情報は標準入力より入力します。
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "./1st/1stboot.h"

struct  dev_param {
  unsigned char bootdev;
  unsigned short n_cylinder;
  unsigned char n_head;
  unsigned char n_sector;
  unsigned int  start;
};

int
main(int argc, char** argv)
{
  int nfrg, i, count;
  int start_block, blocksize;
  int cylinder, head, sector;
  struct dev_param dp;
  struct fragment frg[MAX_FRAGMENT];
  unsigned char buff[512];
  int size;
  int fd;

  if(argc != 2) {
    fprintf(stderr, "Usage : mkboot filename\n");
    exit(1);
  }

  /* ブートディスクのパラメータ */
  dp.bootdev = 0;          /* 1.44M FDD */
  dp.n_cylinder = 80;
  dp.n_head = 2;
  dp.n_sector = 18;

  /* ブロックサイズ */
  if(scanf("%d", &blocksize) != 1) {
    fprintf(stderr, "cannot read block size.\n");
    exit(1);
  }

  /* マップ */
  printf("    C   H   S    n\n");
  printf("------------------\n");

  for(nfrg=0; nfrg < MAX_FRAGMENT + 1; nfrg++) {
    if(scanf("%d %d", &start_block, &count) != 2)
      break;

    /* ブロックナンバーから、CHSに変換する */
    frg[nfrg].count = count;
    frg[nfrg].cylinder = start_block / (dp.n_sector * dp.n_head);
    frg[nfrg].head = (start_block % (dp.n_sector * dp.n_head)) / dp.n_sector;
    frg[nfrg].sector = (start_block % (dp.n_sector * dp.n_head)) % dp.n_sector + 1;
   
    printf("%5d,%3d,%3d,%4d\n", frg[nfrg].cylinder, frg[nfrg].head, frg[nfrg].sector, count);
  }

  if(nfrg == 0) {
    fprintf(stderr, "cannot read fragment.\n");
    exit(1);
  }

  if(nfrg == MAX_FRAGMENT + 1) {
    fprintf(stderr, "too many fragment.\n");
    exit(1);
  }
 

  fd = open(argv[1], O_RDWR);
  if(fd == -1) {
    fprintf(stderr, "cannot open 1stboot file.\n");
    exit(1);
  }
  
  size = read(fd, buff, 512);
  if(size != 512) {
    fprintf(stderr, "cannot make 1stboot.\n");
    exit(1);
  }

  /* ブートデバイス パラメータ */
  buff[ADDR_DEV_PARAM] = dp.bootdev;
  buff[ADDR_DEV_PARAM + 1] = dp.n_cylinder & 0x0ff;
  buff[ADDR_DEV_PARAM + 2] = dp.n_cylinder >> 8;
  buff[ADDR_DEV_PARAM + 3] = dp.n_head;
  buff[ADDR_DEV_PARAM + 4] = dp.n_sector;

  /* マップ */

  for(i = 0; i < MAX_FRAGMENT * 6; i++)     /* マップのクリア  */
    buff[ADDR_MAP + i] = 0x00;

  for(i =  0; i < nfrg; i++) {
    buff[ADDR_MAP + i * 6] = frg[i].count & 0xff;
    buff[ADDR_MAP + i * 6 + 1] = frg[i].count >> 8;
    buff[ADDR_MAP + i * 6 + 2] = frg[i].cylinder & 0xff;
    buff[ADDR_MAP + i * 6 + 3] = frg[i].cylinder >> 8;
    buff[ADDR_MAP + i * 6 + 4] = frg[i].head;
    buff[ADDR_MAP + i * 6 + 5] = frg[i].sector;
  }

  if(lseek(fd, 0, SEEK_SET) == -1) {
    fprintf(stderr,"cannot make 1stboot.\n");
    exit(1);
  }
  if(write(fd, buff, 512) != 512) {
    fprintf(stderr,"cannot make 1stboot.\n");
    exit(1);
  }

  exit(0);
}

 
      
    
