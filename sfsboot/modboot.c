/*
   modboot.c for rewrite hd_boot.bin
   Copyright (C) 2001 Tomohide Naniwa. All rights reserved.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Last modified: Dec. 10, 2001.
*/


#include <stdio.h>
#include <fcntl.h>
#include "1st/1stboot.h"

#define FD_CYLINDER 80
#define FD_HEAD 2
#define FD_SECTOR 18

struct  dev_param {
  unsigned char bootdev;
  unsigned short n_cylinder;
  unsigned char n_head;
  unsigned char n_sector;
  unsigned int  start;
};

main(argc, argv)
     int argc;
     char *argv[];
{
  int fd, size, i;
  struct dev_param dp;
  char buff[512];
  int count, start_block, cylinder, head, sector;
  
  if (argc < 6) {
    printf("useage: %s file head sector cylinder start\n", argv[0]);
    exit(0);
  }
  if ((fd = open(argv[1], O_RDWR)) < 0) {
    printf("can't open file %s\n", argv[1]);
    exit(0);
  }
  dp.bootdev = 0x80;          /* hda */
  dp.n_cylinder = atoi(argv[4]);
  dp.n_head = atoi(argv[2]);
  dp.n_sector = atoi(argv[3]);
  dp.start = atoi(argv[5]);
  printf("boot param head = %d, sector = %d, cylinder = %d, start = %d\n", 
	 dp.n_head, dp.n_sector, dp.n_cylinder, dp.start);

  size = read(fd, buff, 512);
  if(size != 512) {
    fprintf(stderr, "can't read from %s.\n", argv[1]);
    exit(1);
  }

  if (buff[ADDR_DEV_PARAM]) {
    fprintf(stderr, "%s is not rewritable.\n", argv[1], argv[0]);
    exit(1);
  }
  /* ブートデバイス パラメータ */
  buff[ADDR_DEV_PARAM] = dp.bootdev;
  buff[ADDR_DEV_PARAM + 1] = dp.n_cylinder & 0x0ff;
  buff[ADDR_DEV_PARAM + 2] = dp.n_cylinder >> 8;
  buff[ADDR_DEV_PARAM + 3] = dp.n_head;
  buff[ADDR_DEV_PARAM + 4] = dp.n_sector;

  for(i =  0; i < MAX_FRAGMENT; i++) {
    count = (buff[ADDR_MAP + i * 6] & 0xff);
    count += (buff[ADDR_MAP + i * 6 + 1] & 0xff) << 8;

    if (count == 0) break;
    
    cylinder = (buff[ADDR_MAP + i * 6 + 2] & 0xff);
    cylinder += (buff[ADDR_MAP + i * 6 + 3] & 0xff) << 8;
    head =  buff[ADDR_MAP + i * 6 + 4] & 0xff;
    sector = buff[ADDR_MAP + i * 6 + 5] & 0xff;

    start_block = (cylinder * (FD_SECTOR*FD_HEAD)) +
      (head * FD_SECTOR) + sector - 1 + dp.start;

    cylinder = start_block / (dp.n_sector * dp.n_head);
    head = (start_block % (dp.n_sector * dp.n_head)) / dp.n_sector;
    sector = (start_block % (dp.n_sector * dp.n_head)) % dp.n_sector + 1;

    buff[ADDR_MAP + i * 6] = count & 0xff;
    buff[ADDR_MAP + i * 6 + 1] = count >> 8;
    buff[ADDR_MAP + i * 6 + 2] = cylinder & 0xff;
    buff[ADDR_MAP + i * 6 + 3] = cylinder >> 8;
    buff[ADDR_MAP + i * 6 + 4] = head;
    buff[ADDR_MAP + i * 6 + 5] = sector;
  }

  if(lseek(fd, ADDR_DEV_PARAM, 0) == -1) {
    fprintf(stderr,"seek error.\n");
    exit(1);
  }
  size = 512 - ADDR_DEV_PARAM;
  if(write(fd, &buff[ADDR_DEV_PARAM], size) != size) {
    fprintf(stderr,"can't rewrite %s.\n", argv[1]);
    exit(1);
  }
  close(fd);
}
