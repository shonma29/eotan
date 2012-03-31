/*********************************************************************
 * mkboot.c
 *  $B$3$N%W%m%0%i%`$O!"(B1stboot$B$K!"(B $B%G%#%9%/%Q%i%a!<%?!"%^%C%W>pJs$r=q$-9~$_$^$9!#(B
 *  $B8=:_(B,primary$B$N(Bmaster HD$B$+$i$N%V!<%H$K$7$+BP1~$7$F$$$^$;$s!#(B
 *  $B%^%C%W>pJs$OI8=`F~NO$h$jF~NO$7$^$9!#(B
 *
 *  $B=q<0(B : 
 *         mkboot filename device 
 *     
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/hdreg.h>

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
  struct hd_geometry geo;
  int bootfd; 

  if(argc != 3) {
    fprintf(stderr, "Usage : mkboot filename device\n");
    exit(1);
  }

  /* $B%V!<%H%G%#%9%/$N%Q%i%a!<%?(B */
  bootfd = open(argv[2], 0);
  if(ioctl(bootfd, HDIO_GETGEO, &geo) < 0) {
    perror("ioctl");
  }
  close(bootfd);
  
  dp.bootdev = 0x80;          /* hda */
  dp.n_cylinder = geo.cylinders;
  dp.n_head = geo.heads;
  dp.n_sector = geo.sectors;
  dp.start = geo.start;

  /* $B%V%m%C%/%5%$%:(B */
  if(scanf("%d", &blocksize) != 1) {
    fprintf(stderr, "cannot read block size.\n");
    exit(1);
  }

  /* $B%^%C%W(B */
  printf("    C   H   S    n\n");
  printf("------------------\n");

  for(nfrg=0; nfrg < MAX_FRAGMENT + 1; nfrg++) {
    if(scanf("%d %d", &start_block, &count) != 2)
      break;
    
    start_block = start_block + dp.start;

    /* $B%V%m%C%/%J%s%P!<$+$i!"(BCHS$B$KJQ49$9$k(B */
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

  /* $B%V!<%H%G%P%$%9(B $B%Q%i%a!<%?(B */
  buff[ADDR_DEV_PARAM] = dp.bootdev;
  buff[ADDR_DEV_PARAM + 1] = dp.n_cylinder & 0x0ff;
  buff[ADDR_DEV_PARAM + 2] = dp.n_cylinder >> 8;
  buff[ADDR_DEV_PARAM + 3] = dp.n_head;
  buff[ADDR_DEV_PARAM + 4] = dp.n_sector;

  /* $B%^%C%W(B */

  for(i = 0; i < MAX_FRAGMENT * 6; i++)     /* $B%^%C%W$N%/%j%"(B  */
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

 
      
    









