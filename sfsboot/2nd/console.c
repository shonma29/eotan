#include "console.h"
#include "vram.h"
#include "keycode.h"
#include "asm.h"
#include "types.h"
#include "errno.h"
#include "macros.h"

static int cur_x; 
static int cur_y;
static int attrib;
static short* text_vram;

static void write_cr ();
static void set_cursor(int sx, int sy);
static void get_cursor(int *gx,int *gy);
static void scroll_up(void);
static void write_vram(int x, int y, int ch, int attr);
static void console_clear (void);

/************************************************************************
 * テキスト画面の初期化
 */     

int
init_console (void)
{
  text_vram = (short*)TEXT_VRAM_ADDR;
  attrib = 0x07;

  console_clear ();

  cur_x = 0;
  cur_y = 0;
  set_cursor(cur_x, cur_y);

  return E_OK;
}


/***********************************************************************
 * 改行
 */

static void
write_cr ()
{
  cur_x = 0;
  cur_y = cur_y + 1;

  if (cur_y == MAX_HEIGHT) {
    scroll_up ();
    cur_y = MAX_HEIGHT - 1;
  }

  set_cursor(cur_x, cur_y);
}

/***********************************************************************
 * カーソルの位置に、一文字表示する
 */

int
putchar (int ch)
{
  ch = ch & 0xff;
  if (ch == '\n') {
    write_cr ();
    return E_OK;
  }
  else {
    write_vram (cur_x, cur_y, ch, attrib);
  }

  cur_x = cur_x + 1;
  
  if (cur_x == MAX_WIDTH) {
    cur_y = cur_y + 1;
    cur_x = 0;
    if (cur_y == MAX_HEIGHT) {
      scroll_up ();
      cur_y = MAX_HEIGHT - 1;
    }
  }
  set_cursor(cur_x, cur_y);

  return E_OK;
}

/************************************************************
* テキストVRAM等、ハードウェアへの操作
*/

static void 
set_cursor(int sx, int sy)
{
  int addr;

  dis_int();
  addr = MAX_WIDTH * sy + sx;
  outb (GDC_ADDR, 0x0e);
  outb (GDC_DATA, (addr >> 8));
  outb (GDC_ADDR, 0x0f);
  outb (GDC_DATA, addr);
  ena_int();    
}
/***********************************************************
 * 現在のカーソル位置を返す
 */

static void 
get_cursor(int *gx,int *gy)
{
  *gx = cur_x;
  *gy = cur_y;
}

/***********************************************************
 * 一行スクロールアップ
 */

static void
scroll_up ()
{
  int i;
  
  for (i = 0; i < (MAX_WIDTH * (MAX_HEIGHT - 1)); i++) 
      text_vram[i] = text_vram[i + MAX_WIDTH];
  
  for (; i < (MAX_WIDTH * MAX_HEIGHT); i++) 
      text_vram[i] = 0x0720;
}

/***********************************************************
 * 指定されたtext_vramの位置に、文字と属性を書き込む
 */

static void
write_vram (int x, int y, int ch, int attr)
{
  ch = (attr << 8) | (ch & 0x00ff);
  text_vram[MAX_WIDTH * y + x] = ch;
}

/***********************************************************
 * 画面消去
 */

static void
console_clear (void)
{
  int i;
  for(i = 0; i < (MAX_HEIGHT * MAX_WIDTH); i++)
    text_vram[i] = 0x0720;
}
