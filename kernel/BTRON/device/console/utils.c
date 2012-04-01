/*
   VGA console driver
   utils.c
   Copyright (C) 1999-2001 Tomohide Naniwa. All rights reserved.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Last modified: Dec. 28, 2001.
*/

void vram_write(unsigned char *gvram, int color)
{
  unsigned char dummy;

  dummy = *gvram;	/* ラッチに VRAM の内容を読み込む */
  *gvram = color;
}
