#include "types.h"
#include "config.h"
#include "boot.h"
#include "lib.h"

UWORD clock;

#define BCD_TO_BIN(x) ((((x) & 0xF0) >> 4)*10 + ((x) & 0x0F))
void init_time()
{
  char *bcd_century, *bcd_year, *bcd_month, *bcd_day;
  char *bcd_hour, *bcd_minute, *bcd_second;
  unsigned int year, month, day, hour, minute, second;

  bcd_century = (char *) 0x8ff7;
  bcd_year = (char *)0x8ff6;
  bcd_month = (char *)0x8ff5;
  bcd_day = (char *) 0x8ff4;
  bcd_hour = (char *) 0x8ff3;
  bcd_minute = (char *) 0x8ff2;
  bcd_second = (char *) 0x8ff1;

  year = BCD_TO_BIN(*bcd_century)*100 + BCD_TO_BIN(*bcd_year);
  month = BCD_TO_BIN(*bcd_month);
  day = BCD_TO_BIN(*bcd_day);
  hour = BCD_TO_BIN(*bcd_hour);
  minute = BCD_TO_BIN(*bcd_minute);
  second = BCD_TO_BIN(*bcd_second);

  boot_printf ("boot date = %d/%d/%d %d:%d:%d ",
	       (int) year, (int) month, (int) day,
	       (int) hour, (int) minute, (int) second);

  if (0 >= (int) ((month) -= 2)) {	/* 1..12 -> 11,12,1..10 */
    (month) += 12;	/* Puts Feb last since it has leap day */
    (year) -= 1;
  }

  clock =
    ((((UWORD)(year/4 - year/100 + year/400 + 367*month/12 + day) +
       year*365 - 719499
       )*24 + hour
      )*60 + minute
     )*60 + second;
  clock -= 32400; /* 9h, JST to GMT */
  clock -= BTRON_OFFSET;
  boot_printf ("BTRON SECONDS = %d(%d)\n", clock, clock+BTRON_OFFSET);
}
