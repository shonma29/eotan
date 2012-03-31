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
 * �ƥ����Ȳ��̤ν����
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
 * ����
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
 * ����
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
 * ��������ΰ��֤ˡ���ʸ��ɽ������
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
 * �����ܡ��ɤ����ʸ���ɤ߹���
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
 * �����ܡ��ɤ����ʸ���ɤ߹��ࡣ�������Хå�̵��
 */

int getch(void)
{
  int ch;
  ch = read_keyboard ();
  return ch;
}

/***************************************************************************
 * ʸ�����CR�����Ϥ����ޤǥ����ܡ��ɤ����ɤ�
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
* �ƥ�����VRAM�����ϡ��ɥ������ؤ����
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
 * ���ߤΥ���������֤��֤�
 */

void 
get_cursor(int *gx,int *gy)
{
  *gx = cur_x;
  *gy = cur_y;
}

/***********************************************************
 * ��ԥ������륢�å�
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
 * ���ꤵ�줿text_vram�ΰ��֤ˡ�ʸ����°����񤭹���
 */

void
write_vram (int x, int y, int ch, int attr)
{
  ch = (attr << 8) | (ch & 0x00ff);
  text_vram[MAX_WIDTH * y + x] = ch;
}

/***********************************************************
 * ���̾õ�
 */

void
console_clear (void)
{
  int i;
  for(i = 0; i < (MAX_HEIGHT * MAX_WIDTH); i++)
    text_vram[i] = 0x0720;
}






