#include "console.h"
#include "vram.h"
#include "keyboard.h"
#include "keycode.h"
#include "asm.h"
#include "types.h"
#include "errno.h"
#include "macros.h"

static int cur_x; 
static int cur_y;
static int attrib;
static short* text_vram;

static void write_vram(int x, int y, int ch, int attr);
static void scroll_up(void);
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

void
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
 * タブ
 */

void
write_tab ()
{
  int	tmp;
  if (cur_x < MAX_WIDTH) {
    tmp = ROUNDUP (cur_x + 1, 8) - 1;
    while (cur_x < tmp) {
      write_vram (cur_x, cur_y, ' ', 0x07);
      cur_x++;
    }
    set_cursor(cur_x, cur_y);
  }
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

/***************************************************************************
 * キーボードから一文字読み込む
 */

int
getchar (void)
{
  int   ch;
  
  ch = read_keyboard ();
  putchar (ch);
  return (ch);
}

/***************************************************************************
 * キーボードから一文字読み込む。エコーバック無し
 */

int getch(void)
{
  int ch;
  ch = read_keyboard ();
  return ch;
}

/***************************************************************************
 * 文字列を、CRが入力されるまでキーボードから読む
 */

char *
gets (char *line)
{
  int   ch;
  char  *p;
  int   len;
  
  p = line;
  len = 0;
  ch = getch ();
  while (ch != '\n')
    {
      if(ch == DEL){
        if(len>0){
	  cur_x = cur_x - 1;
          write_vram(cur_x, cur_y, ' ', 0x07);
          set_cursor(cur_x, cur_y);
          p--;
          len--;
        }
      }else{
        *p++ = ch;
        len++;
        putchar(ch);
      }
      ch = getch ();
    }
  *p = '\0';
  putchar('\n');
  return (line);
}


/************************************************************
* テキストVRAM等、ハードウェアへの操作
*/

void 
set_cursor(int sx, int sy)
{
  int addr;

  clear_int();
  addr = MAX_WIDTH * sy + sx;
  outb (GDC_ADDR, 0x0e);
  outb (GDC_DATA, (addr >> 8));
  outb (GDC_ADDR, 0x0f);
  outb (GDC_DATA, addr);
  set_int();    
}
/***********************************************************
 * 現在のカーソル位置を返す
 */

void 
get_cursor(int *gx,int *gy)
{
  *gx = cur_x;
  *gy = cur_y;
}

/***********************************************************
 * 一行スクロールアップ
 */

void
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

void
write_vram (int x, int y, int ch, int attr)
{
  ch = (attr << 8) | (ch & 0x00ff);
  text_vram[MAX_WIDTH * y + x] = ch;
}

/***********************************************************
 * 画面消去
 */

void
console_clear (void)
{
  int i;
  for(i = 0; i < (MAX_HEIGHT * MAX_WIDTH); i++)
    text_vram[i] = 0x0720;
}






