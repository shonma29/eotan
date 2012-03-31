/*
   FoRTh Modoki
   debug.c ver 1.1.0
   Copyright (C) 1999, 2000 Tomohide Naniwa. All rights reserved.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Last modified: Jul. 26, 2000.
*/

#include "stdlib.h"
#include "frtm.h"

void f_tasklist (void)
{
  misc (-6, 0, NULL);
}

void f_pmemstat(void)
{
  misc (-7, 0, NULL);
}

void f_prtflist(void)
{
  /* malloc の空きリストの表示 */
  misc (4, 0, NULL);
}

void f_purge(void)
{
  /* purge disk cacue */
  misc(5, 0, NULL);
}


/* test for brk */
void f_brk(void)
{
  void *endds;
  int err;

  endds = (void *) 0x10002000;
  printf("call brk(0x%x)\n", endds);
  err = brk(endds);
  if (err) {
    printf("brk error %d\n", err);
  }
  endds = (void *) 0x10000000;
  printf("call brk(0x%x)\n", endds);
  err = brk(endds);
  if (err) {
    printf("brk error %d\n", err);
  }
}
