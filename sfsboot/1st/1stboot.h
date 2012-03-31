#ifndef __1STBOOT_H__
#define __1STBOOT_H__

#define MAX_FRAGMENT 8
#define ADDR_DEV_PARAM 0x1c0
#define ADDR_MAP 0x1c6 


struct fragment {
  unsigned short count;
  unsigned short cylinder;
  unsigned char head;
  unsigned char sector;
};

#endif
