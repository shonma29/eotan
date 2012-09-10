/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* console.c --- コンソールへの入出力モジュール
 *
 *
 * コンソールへの入力および出力を行う。
 *
 * なお、ここで定義しているコンソール入出力関数群は、ITRON レベルのため、
 * 上位層では、別のデバイスドライバが入出力を行う。また、ITRON からの出力も、
 * 上位層で定義するデバイスドライバを介して行う。ここの関数群を使用するのは、
 * 立ち上げ直後だけである。
 *
 *
 */

#include "core.h"
#include "misc.h"
#include "func.h"
#include "../../include/mpu/io.h"
#include "../../include/device.h"

/*	アトリビュートの設定
 */
#define NORM_ATTR	0x1e		/* 通常文字 			*/
#define MAX_WIDTH	79
#define MAX_HEIGHT	24

#define TEXT_VRAM_ADDR	0x800B8000
#define TEXT_VRAM_SIZE	2000

#define GDC_ADDR	0x03d4
#define GDC_DATA	0x03d5

#define CURSOR_POS(x,y)		(x + y * 80)


/***********************************************************************
 * position --- カーソル位置情報
 *
 */
struct position
{
  W	x;
  W	y;
};

static	struct position	cursor;


static void	inc_cursor (W count);
static void write_cr (void);
static void write_tab ();
static void	scroll_up (void);
static void console_clear (void);
static void	write_vram (W x, W y, W ch, W attr);
static void	set_cursor_pos (W x, W y);

/*************************************************************************
 * simple_init_console 
 *
 * 引数：
 *
 * 返値：
 *
 * 処理：
 *
 */
void
simple_init_console (void)
{
  console_clear ();
}

/*************************************************************************
 * putchar --- １文字出力; カーネル内のサービスルーチン
 *
 *
 * 引数：	ch	出力する文字コード (TRON コード)
 *
 * 返値：	なし
 *
 * 処理：	write_vram を呼び出し、VRAM に１文字出力する。
 *		この関数は、カーネルからメッセージを出力する時にも使用する。
 *		そのため、この関数は GLOBAL として定義している。
 *
 *
 * 注意：	この関数は、出力できる文字かどうかをチェックしていない。
 *		出力できない文字が指定された場合でも、VRAM に書き込もうとする。
 *
 */
void
putchar (TC ch)
{
  switch (ch)
    {
    default:
      write_vram (cursor.x, cursor.y, ch, NORM_ATTR);
      inc_cursor (1);
      break;

    case '\n':
      write_cr ();
      break;

    case '\t':
      write_tab ();
      break;
    }
}

/*************************************************************************
 * inc_cursor --- カーソルを進める。
 *
 * 引数：	count	カーソルの移動量
 *
 * 返値：	なし
 *
 * 処理：	指定した量だけ、カーソルを進める(count は、せいぜい2である)。
 *		もし、カーソルが画面をはみだした場合は、カーソルを次の行に
 *		進める。
 *		さらに一番下の行にいる場合には、これ以上下の行にいけないので、
 *		スクロールアップする。
 *
 */
static void
inc_cursor (W count)
{
  cursor.x += count;
  if (cursor.x >= MAX_WIDTH)
    {
      cursor.y++;
      if (cursor.y >= MAX_HEIGHT)
	{
	  scroll_up ();
	  cursor.y = MAX_HEIGHT;
	}
      cursor.x = 0;
    }
  set_cursor_pos (cursor.x, cursor.y);
}

/*************************************************************************
 * write_cr --- 改行する
 *
 * 引数：	なし
 *
 * 返値：	なし
 *
 * 処理：	カーソルを次の行の先頭に移動させる。
 *		もし、すでに行が一番最後ならば、画面全体をスクロールする。
 *
 */
static void write_cr (void)
{
  cursor.x = 0;
  if (cursor.y >= MAX_HEIGHT)
    {
      scroll_up ();
    }
  else
    {
      cursor.y++;
    }
  set_cursor_pos (cursor.x, cursor.y);
}

/*************************************************************************
 * write_tab --- タブを出力する。
 *
 * 引数：	なし
 *
 * 返値：	なし
 *
 * 処理：	タブを出力する。
 *
 */
static void write_tab ()
{
  W	tmp;

  if (cursor.x < MAX_WIDTH)
    {
      tmp = ROUNDUP (cursor.x + 1, 8) - 1;
      while (cursor.x < tmp)
	{
	  write_vram (cursor.x, cursor.y, ' ', 0xe1);
	  cursor.x++;
	}
    }
}

/*************************************************************************
 * scroll_up --- 画面全体をスクロールアップする
 *
 * 引数：	なし
 *
 * 返値：	なし
 *
 * 処理：	画面全体を一行スクロールする。
 *
 */
static void
scroll_up (void)  
{
  UH	*addr;
  W		i;
  
  addr = (unsigned short *)TEXT_VRAM_ADDR;
  for (i = 0; i < TEXT_VRAM_SIZE - 80; i++)
    {
      addr[i] = addr[i + 80];
    }
  for (i = 0; i <= MAX_WIDTH; i++)
    {
      write_vram (i, MAX_HEIGHT, ' ', 0xe1);
    }
}

/*************************************************************************
 * console_clear --- コンソールをクリアする。
 *
 * 引数：	なし
 *
 * 返値：	なし
 *
 * 処理：	コンソールのはじからはじまでを空白文字で埋める。
 *
 */
static void console_clear (void)
{
  W	x, y;
  
  for (y = 0; y <= MAX_HEIGHT; y++)
    for (x = 0; x <= MAX_WIDTH; x++)
      {
	write_vram (x, y, ' ', 0xE1);
      }
  cursor.x = cursor.y = 0;
}

/*************************************************************************
 * write_vram --- VRAM に文字を書き込む
 *
 * 引数：	x, y	座標
 *		ch	出力する文字
 *		attr	文字属性
 *
 * 返値：	なし
 *
 * 処理：	引数 x, y で指定した座標に文字を書き込む.
 *		この関数は、ASCII 文字用になっている。
 *
 *
 * 注意：	現在、attr は無視している。
 *
 */
static void
write_vram (W x, W y, W ch, W attr)
{
  short	*addr;
  
  addr = (short *)TEXT_VRAM_ADDR;
  ch = ch | (addr [CURSOR_POS (x, y)] & 0xff00);
  addr [CURSOR_POS (x, y)] = ch;
}

/*************************************************************************
 * set_cursor_pos --- カーソル位置の設定.
 *
 * 引数：	x, y	座標
 *
 * 返値：	なし
 *
 * 処理：	指定した座標にカーソルを移動させる。
 *
 */
static void
set_cursor_pos (W x, W y)
{
  W	addr;

  addr = CURSOR_POS (x, y);
  outb (GDC_ADDR, 0x0e);
  outb (GDC_DATA, (addr >> 8) & 0xff);
  outb (GDC_ADDR, 0x0f);
  outb (GDC_DATA, addr & 0xff);
}
