#include "stdlib.h"
#if 0
#include "fcntl.h"
#endif

int fd;

char 
read_psaux()
{
  char c;

  while(read(fd, &c, 1) == 0);
  return c;
}

main()
{
  int  i, x ,y, l, r;
  unsigned char cmd, d1, d2, d3;

  x = y = 0;
  r = l = 0;

  fd = open("/dev/psaux", O_RDWR);

  if(fd == -1)
    {
      printf("cannot open PSAUX.\n");
      _exit(1);
    }
  
  /* マウスをリセットする */
  cmd = 0xff;
  write(fd, &cmd, 1);
  d1 = read_psaux();
  d2 = read_psaux();
  d3 = read_psaux();
  if(d2 != 0xaa)
    {
      printf("mouse device error.\n");
      goto psauxtst_exit;
    }
  

  /* デフォルト設定 */
  cmd = 0xf6;
  write(fd, &cmd, 1);
  d1 = read_psaux();

  /* マウスをイネーブルにする */
  cmd = 0xf4;
  write(fd, &cmd, 1);
  d1 = read_psaux();

  printf("X:Y:L:R = %d:%d:%d:%d\n", x, y, l, r); 

  while(1) 
    {
      d1 = read_psaux();
      d2 = read_psaux();
      d3 = read_psaux();

      if((d1 & 0x10) == 0)
	x = x + (signed char)d2;
      else
	x = x + ((signed char)d2 & 0xffffffff);

      if((d1 & 0x20) == 0)
	y = y + (signed char)d3;
      else
	y = y + ((signed char)d3 & 0xffffffff);

      l = d1 & 0x1;
      r = (d1 & 0x2) >> 1;

      printf("X:Y:L:R = %d:%d:%d:%d\n", x, y, l, r);      

      if(l && r)
	break;

    }

  /* マウスをディスエイブルにする */
  cmd = 0xf5;
  write(fd, &cmd, 1);
  d1 = read_psaux();

 psauxtst_exit:
  close(fd);
}
