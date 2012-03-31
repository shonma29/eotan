/*
   tty.h
   Copyright (C) 2002 Tomohide Naniwa. All rights reserved.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Last modified: Apr. 28, 2002.
*/

#ifndef __LIBC_TTY_H__
#define __LIBC_TTY_H__	1

/* keydevice mode */
#define WAITMODE	0x0001
#define ENAEOFMODE	0x0002
#define RAWMODE		0x0004

struct winsize {
  unsigned short int ws_row;
  unsigned short int ws_col;
  unsigned short int ws_xpixel;
  unsigned short int ws_ypixel;
} ws;

extern int get_winsz(int fd, struct winsize *ws);
extern int ena_eof(int fd);
extern int dis_eof(int fd);
extern int set_keymode(int fd, int mode);
extern int get_keymode(int fd, int *mode);

#endif
