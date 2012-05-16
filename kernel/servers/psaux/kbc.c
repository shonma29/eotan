/*
   kbc.c

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "psaux.h"
#include "kbc.h"
#include "../../../include/mpu/io.h"

extern W driver_mode;
extern ID waitflag;

/*********************************************************************
 * リングバッファ
 *
 */

struct {
  B buffer[BUFF_SIZE];   /* バッファ */
  W read_pos;            /* 読み出しポインター */
  W write_pos;           /* 書き込みポインター */
  W count;               /* カウンター */
} ring_buffer;

void
init_buffer()
{
  ring_buffer.read_pos = 0;
  ring_buffer.write_pos = 0;
  ring_buffer.count = 0;
}

/*********************************************************************
 * キーボードコントローラへの入出力関数
 * 
 */

W
kbc_wr_command(B command)
{
  int i;
  for(i = 0; i < MAX_RETRY; i++)
    {
      if((inb(KBC_STAT) & (KBC_IBF | KBC_OBF)) == 0)
	{
	  outb(KBC_COM, command);
	  return TRUE;
	}
    }
  return FALSE;
}

W
kbc_wr_data(B data)
{
  int i;
  for(i = 0; i < MAX_RETRY; i++)
    {
      if((inb(KBC_STAT) & (KBC_IBF | KBC_OBF)) == 0)
	{
	  outb(KBC_DATA, data);
	  return TRUE;
	}
    }
  return FALSE;
}


B
kbc_rd_data(void)
{
  while((inb(KBC_STAT) & KBC_OBF) == 0);
  return inb(KBC_DATA);
}

/*********************************************************************
 * psaux割り込みハンドラ
 * 
 */

void
psaux_interrupt()
{
  B data;

  /* 割り込み禁止 */
  dis_int();         

  /* KBCからのデータの読み込み */
  data = kbc_rd_data();

  /* リングバッファへの書き込み */
  ring_buffer.buffer[ring_buffer.write_pos] = data;
  ring_buffer.write_pos++;
  ring_buffer.count++;

  if(ring_buffer.write_pos == BUFF_SIZE)
    ring_buffer.write_pos = 0;

  if(ring_buffer.count > BUFF_SIZE) 
    {
      ring_buffer.count = BUFF_SIZE;
      ring_buffer.read_pos = ring_buffer.write_pos;
    }

  /* 割り込み許可 */
  ena_int();

  /* イベントフラグ */
  set_flg(waitflag, 1);    

}

/*********************************************************************
 * psaux割り込みハンドラの登録
 *
 */

void
init_interrupt()
{
  T_DINT	pk_dint;
  ER		errno;

  pk_dint.intatr = ATR_INTR;
  pk_dint.inthdr = (FP)psaux_interrupt;
  errno = def_int(PSAUX_IRQ + 32, &pk_dint);
  if (errno != E_OK)
    {
      dbg_printf("psaux: cannnot define interrupt handler.\n");
      slp_tsk();
    }
}


/*********************************************************************
 * psauxからのdata入力 （実際にはリングバッファからの読み出し）
 *
 */

W
psaux_data_in(B* val)
{
  UINT rflag;
  B data;

  if((driver_mode == NOWAITMODE) && (ring_buffer.count == 0))
    return FALSE;

 retry:
  
  if(ring_buffer.count != 0)
    {
      dis_int();
      data = ring_buffer.buffer[ring_buffer.read_pos];
      ring_buffer.count--;
      ring_buffer.read_pos++;
      
      if(ring_buffer.read_pos == BUFF_SIZE)
	ring_buffer.read_pos = 0;

      ena_int();
      
      *val = data;
      return TRUE;
    }
  
  wai_flg(&rflag, waitflag, 1, TWF_ORW | TWF_CLR);
  
  goto retry;

}

/*********************************************************************
 * psauxへのdata出力
 *
 */

W
psaux_data_out(B data)
{
  dis_int();

  if(kbc_wr_command(0xd4) == TRUE)
    {
      if(kbc_wr_data(data) == TRUE)
	{
	  ena_int();
	  return TRUE;
	}
    }

  ena_int();
  return FALSE;
}



  
