/*
   FoRTh Modoki
   funcs.c ver 1.4.0
   Copyright (C) 1999-2001 Tomohide Naniwa. All rights reserved.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Last modified: Mar. 4, 2001.
*/

#include <stdlib.h>
#include "frtm.h"

/* 組み込みコマンド */

void f_quit()
{
  error_no = E_QUT;
}

void f_dot()
{
  int val;
  val = pop();
  EC;
  printf("%d", val);
}

void f_doth()
{
  int val;
  val = pop();
  EC;
  printf("%x", val);
}

void f_doto()
{
  int val;
  val = pop();
  EC;
  printf("%o", val);
}

void f_cr()
{
  printf("\n");
}

void f_sum()
{
  int val1, val2;
  val1 = pop(); EC;
  val2 = pop(); EC;
  push(val1 + val2); EC;
}

void f_sub()
{
  int val1, val2;
  val1 = pop(); EC;
  val2 = pop(); EC;
  push(val2 - val1); EC;
}

void f_mul()
{
  int val1, val2;
  val1 = pop(); EC;
  val2 = pop(); EC;
  push(val1 * val2); EC;
}

void f_div()
{
  int val1, val2;
  val1 = pop(); EC;
  val2 = pop(); EC;
  if (val1 == 0) {
    error_no = E_DV0;
    return;
  }
  push(val2 / val1); EC;
}

void f_mod()
{
  int val1, val2;
  val1 = pop(); EC;
  val2 = pop(); EC;
  if (val1 == 0) {
    error_no = E_DV0;
    return;
  }
  push(val2 % val1); EC;
}

void f_echo()
{
  char str[LINE];

  pop_string(str); EC;
  printf("%s", str);
}

void f_unpack()
{
  char str[LINE];
  int i;

  pop_string(str); EC;
  for (i = strlen(str); i >= 0; --i) {
    push(str[i]); EC;
  }
}

void f_pack()
{
  static char str[LINE];
  int i;

  i = 0;
  while (1) {
    str[i] = pop(); EC;
    if (str[i++] == 0) break;
  }
  push_cstring(str); EC;
}

void f_eq()
{
  int v1, v2;
  v1 = pop(); EC;
  v2 = pop(); EC;
  push(v1 == v2); EC;
}

void f_lt()
{
  int v1, v2;
  v1 = pop(); EC;
  v2 = pop(); EC;
  push(v2 < v1); EC;
}

void f_gt()
{
  int v1, v2;
  v1 = pop(); EC;
  v2 = pop(); EC;
  push(v2 > v1); EC;
}

void f_le()
{
  int v1, v2;
  v1 = pop(); EC;
  v2 = pop(); EC;
  push(v2 <= v1); EC;
}

void f_ge()
{
  int v1, v2;
  v1 = pop(); EC;
  v2 = pop(); EC;
  push(v2 >= v1); EC;
}

void f_not()
{
  int val;
  val = pop(); EC;
  push(! val); EC;
}

void f_dup()
{
  int val;
  val = pop(); EC;
  push(val); EC; 
  push(val); EC;
}

void f_swap()
{
  int v1, v2;
  v1 = pop(); EC;
  v2 = pop(); EC;
  push(v1); EC;
  push(v2); EC;
}

void f_drop()
{
  pop(); EC;
}

void f_sp()
{
  printf(" ");
}

void f_spaces()
{
  int val, i;
  val = pop(); EC;
  for(i = 0; i < val; ++i) {
    printf(" ");
  }
}

void f_esc()
{
  printf("\033");
}

void f_minus()
{
  int val;
  val = pop(); EC;
  val *= -1;
  push(val); EC;
}

void f_abs()
{
  int val;
  val = pop(); EC;
  if (val < 0) {
    push(-val); EC;
  }
  else {
    push(val);
  }
}

void f_max()
{
  int v1, v2;
  v1 = pop(); EC;
  v2 = pop(); EC;
  push((v1 > v2)? v1: v2); EC;
}

void f_min()
{
  int v1, v2;
  v1 = pop(); EC;
  v2 = pop(); EC;
  push((v1 < v2)? v1: v2); EC;
}

void f_inc()
{
  int val;
  val = pop(); EC;
  push(val + 1); EC;
}

void f_dec()
{
  int val;
  val = pop(); EC;
  push(val - 1); EC;
}

void f_eq0()
{
  int val;
  val = pop(); EC;
  push(val == 0); EC;
}

void f_over()
{
  int v1, v2;
  v1 = pop(); EC;
  v2 = pop(); EC;
  push(v2); EC;
  push(v1); EC;
  push(v2); EC;
}

void f_rot()
{
  int v1, v2, v3;
  v1 = pop(); EC;
  v2 = pop(); EC;
  v3 = pop(); EC;
  push(v2); EC;
  push(v1); EC;
  push(v3); EC;
}

void f_rotr()
{
  int v1, v2, v3;
  v1 = pop(); EC;
  v2 = pop(); EC;
  v3 = pop(); EC;
  push(v1); EC;
  push(v3); EC;
  push(v2); EC;
}
