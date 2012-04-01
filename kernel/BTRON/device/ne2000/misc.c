/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/device/ne2000/misc.c,v 1.4 1997/10/29 22:39:27 night Exp $ */

static char rcsid[] = "@(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/device/ne2000/misc.c,v 1.4 1997/10/29 22:39:27 night Exp $";


/*
 * $Log: misc.c,v $
 * Revision 1.4  1997/10/29 22:39:27  night
 * NE2000 アダプタの SA_PROM の読み取り処理の追加。
 *
 * Revision 1.3  1997/10/28 12:44:42  night
 * ne2000_internal.h のインクルード文の追加。
 *
 * Revision 1.2  1997/05/17 10:41:15  night
 * ファイルの文字コードを SJIS から EUC に変更。
 *
 * Revision 1.1  1997/05/06 12:43:31  night
 * 最初の登録
 *
 *
 */


#include "ne2000.h"
#include "ne2000_internal.h"





static ID	log_port;
static ID	dev_recv;


static ER	vprintf (B *fmt, VP arg0);




/* init_log - ログ出力機構を初期化する
 *
 *
 */
void
init_log (void)
{
  if (find_port (CONSOLE_DRIVER, &log_port) != E_PORT_OK)
    {
      dbg_printf ("POSIX: Cannot open console device.\n");
      slp_tsk ();
      /* DO NOT REACHED */
    }

  dev_recv = get_port (sizeof (DDEV_RES), sizeof (DDEV_RES));
  if (dev_recv <= 0)
    {
      dbg_printf ("POSIX: Cannot allocate port\n");
      slp_tsk ();
      /* DO NOT REACHED */
    }
}


void
print_digit (UW d, UW base)
{
  static B digit_table[] = "0123456789ABCDEF";

  if (d < base)
    {
      putc ((W)(digit_table[d]), log_port);
    }
  else
    {
      print_digit (d / base, base);
      putc ((W)(digit_table[d % base]), log_port);
    }
}

#define INC(p,x)	(((W)p) = (((W)p) + sizeof (x)))

/*
 *
 */
W
printf (B *fmt,...)
{
  VP arg0;
  ER err;

  arg0 = (VP)&fmt;
  INC (arg0, B *);
  err = vprintf (fmt, (VP)arg0);
  return (err);
}

static ER
vprintf (B *fmt, VP arg0)
{
  VP *ap;

  for (ap = (VP *)arg0; *fmt != '\0'; fmt++)
    {
      if ((*fmt) == '%')
	{
	  ++fmt;
	  switch (*fmt)
	    {
	    case 's':
	      put_string (*(B **)ap, log_port);
	      INC (ap, B *);
	      break;

	    case 'd':
	      if ((W)*ap < 0)
		{
		  ((W)*ap) = -((W)*ap);
		  putc ('-', log_port);
		}
	      print_digit ((W)*ap, 10);
	      INC (ap, W);
	      break;

	    case 'x':
	      print_digit ((UW)*ap, 16);
	      INC (ap, W);
	      break;

	    default:
	      putc ('%', log_port);
	      break;
	    }
	}
      else
	{
	  putc (*fmt, log_port);
	}
    }
}



W
put_string (B *line, ID port)
{
  W i;

  for (i = 0; line[i] != '\0'; i++)
    {
      putc (line[i], port);
    }
  return (i);
}


W 
putc (int ch, ID port)
{
  DDEV_REQ		req;		/* 要求パケット */
  DDEV_RES		res;		/* 返答パケット */
  W			rsize;
  ER			error;
  W			i;
  
  
  req.header.mbfid = dev_recv;
  req.header.msgtyp = DEV_WRI;
  req.body.wri_req.dd = 0xAA;
  req.body.wri_req.size = 1;
  req.body.wri_req.dt[0] = (char)(ch & 0xff);
  error = snd_mbf (port, sizeof (req), &req);
  if (error != E_OK)
    {
      dbg_printf ("cannot send packet. %d\n", error);
      return;
    }
  rsize = sizeof (res);
  error = rcv_mbf (&res, (INT *)&rsize, dev_recv);
  if (res.body.wri_res.errcd != E_OK)
    {
      dbg_printf ("%d\n", res.body.wri_res.errcd);
      return (0);
    }      
  return (1);
}



/* _assert - ASSERT マクロによって呼び出される関数
 *
 * メッセージを出力し、プログラムを終了する。
 *
 */ 
void
_assert (B *msg)
{
  printf ("ASSERT: ");
  printf ("%s\n", msg);
  for (;;)
    {
      slp_tsk ();
    }
}



void
busywait (W count)
{
  W	i;
  W	dummy;

  while (count-- > 0)
    {
      for (i = 0; i < 10000; i++)
	{
	  dummy++;
	}
    }
}
