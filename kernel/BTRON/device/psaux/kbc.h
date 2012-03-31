/*
   kbc.h

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __KBC_H__
#define __KBC_H__	


#define PSAUX_IRQ       12      /* psaux��IRQ */

#define KBC_COM		0x64    /* KBC�ؤΥ��ޥ�� */
#define KBC_DATA	0x60    /* ���ޥ�ɤ��Ф���ѥ�᡼�� */
#define KBC_STAT	0x64    /* KBC�Υ��ơ����� */

#define KBC_OBF         0x01    /* 1:���ϥХåե��˥ǡ���ͭ�� */
#define KBC_IBF         0x02    /* 1:���ϥХåե��˥ǡ���ͭ�� */ 

#define BUFF_SIZE       2048    /* ��󥰥Хåե��Υ����� */

#define MAX_RETRY       100000  /* retry��� */ 

#endif 


