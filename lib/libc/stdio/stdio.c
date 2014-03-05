/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/

/* $Id: stdlib.c,v 1.6 2000/06/04 04:10:15 naniwa Exp $ */

/*
 * $Log: stdlib.c,v $
 * Revision 1.6  2000/06/04 04:10:15  naniwa
 * to support octal
 *
 * Revision 1.5  2000/01/29 16:23:52  naniwa
 * to clean up
 *
 * Revision 1.4  2000/01/18 14:39:25  naniwa
 * to make independent of libkernel.a
 *
 * Revision 1.3  2000/01/15 15:25:16  naniwa
 * to use read/write systemcalls
 *
 * Revision 1.2  1999/11/10 10:44:37  naniwa
 * correction
 *
 * Revision 1.5  1999/05/10 02:57:17  naniwa
 * add fprintf() and modified to user vfprintf()
 *
 * Revision 1.4  1998/12/08 05:38:39  monaka
 * I init has been included the Mitten Scripting Language since this release.
 * But, it's just a first preview. So you can't use many convenience
 * features. Mitten works extension scripting environment like Tcl, Guile,
 * Windows Scripting Host, and so on. If you wished, you may connect your
 * function written in C to Mitten operator. If you wished, you may call
 * Mitten from C.
 *
 * Revision 1.3  1997/07/06 11:50:02  night
 * デバッグ文の引数指定がまちがっていたので、修正した。
 *
 * Revision 1.2  1997/04/24 15:38:06  night
 * printf() 関数の処理を kernlib の dbg_printf() と同様のものにした。
 *
 * Revision 1.1  1996/07/25  16:03:44  night
 * IBM PC 版への最初の登録
 *
 *
 */

#include <unistd.h>
#include <stdio.h>

static ER	vfprintf (FILE *port, B *fmt, VP arg0);
static W writechar (ID port, UB *buf, W length);



static W
print_digit (FILE *port, UW d, UW base)
{
  static B digit_table[] = "0123456789ABCDEF";
  W len = 0;

  if (d < base)
    {
      fputc (digit_table[d], port);
      len++;
    }
  else
    {
      len += print_digit (port, d / base, base);
      fputc (digit_table[d % base], port);
      len++;
    }

  return len;
}

#define INC(p,x)	(p = (VP)(((W)p) + sizeof (x *)))

/*
 *
 */
W
printf (B *fmt,...)
{
  VP *arg0;
  ER err;

  arg0 = (VP *)&fmt;
  INC (arg0, B *);
  err = vfprintf (stdout, fmt, (VP)arg0);
  fflush (stdout);
  return (err);
}

W
fprintf (FILE *port, B *fmt,...)
{
  VP *arg0;
  ER err;

  arg0 = (VP *)&fmt;
  INC (arg0, B *);
  err = vfprintf (port, fmt, (VP)arg0);
  return (err);
}

static ER
vfprintf (FILE *port, B *fmt, VP arg0)
{
  VP *ap;
  W len = 0;

  for (ap = (VP *)arg0; *fmt != '\0'; fmt++)
    {
      if (*fmt == '%')
	{
	  switch (*++fmt)
	    {
	    case 's':
	      len += fputs ((B*)(*ap), port);
	      INC (ap, B *);
	      break;

	    case 'd':
	      if ((W)*ap < 0)
		{
		  W *q = (W*)ap;

		  *q = 0 - ((W)*ap);
		  fputc ('-', port);
		  len++;
		}
	      len += print_digit (port, (W)*ap, 10);
	      INC (ap, W);
	      break;

	    case 'x':
	      len += print_digit (port, (W)*ap, 16);
	      INC (ap, W);
	      break;

	    case 'o':
	      len += print_digit (port, (W)*ap, 8);
	      INC (ap, W);
	      break;

	    default:
	      fputc ('%', port);
	      len++;
	      break;
	    }
	}
      else
	{
	  fputc (*fmt, port);
	  len++;
	}
    }

    return len;
}


W
fputc (W ch, FILE *port)
{
  port->buf[port->pos] = ch;
  port->pos++;

  if (ch == '\n')
    {
      writechar (port->fd, port->buf, port->pos);
      port->pos = 0;
    }
  else if (port->pos >= port->len)
    {
      writechar (port->fd, port->buf, port->pos);
      port->pos = 0;
    }

  return (ch);
}

W
fflush (FILE *port)
{
  writechar (port->fd, port->buf, port->pos);
  port->pos = 0;

  return 0;
}

W
fputs (B *line, FILE *port)
{
  W i;

  for (i = 0; line[i] != '\0'; i++)
    {
      fputc (line[i], port);
    }
  return (i);
}

W
puts (B *line)
{
  W len = fputs (line, stdout);

  fputc ('\n', stdout);
  fflush (stdout);
  return (len + 1);
}

static W
writechar (ID port, UB *buf, W length)
{
  write(port, buf, length);

  return length;
}

W
fgetc (FILE *port)
{
  B buf[1];

  read(port->fd, buf, 1);
  return (W)(buf[0]);
}

W
putchar (W ch)
{
  W result = fputc(ch, stdout);

  fflush(stdout);

  return result;
}
