/*
   tty.c 
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
#include <unistd.h>
#include "../../../servers/keyboard/keyboard.h"

#define FCTL_WC_WINSIZ (WD_WINSIZ << 16)
#define FCTL_ENA_EOF (KEYBOARD_ENA_EOF << 16)
#define FCTL_DIS_EOF (KEYBOARD_DIS_EOF << 16)
#define FCTL_SET_KEY (KEYBOARD_CHANGEMODE << 16)
#define FCTL_GET_KEY (KEYBOARD_GETMODE << 16)

int ena_eof(int fd)
{
  int error;

  error =  fcntl(fd, FCTL_ENA_EOF, 0);
  return error;
}

int dis_eof(int fd)
{
  int error;

  error =  fcntl(fd, FCTL_DIS_EOF, 0);
  return error;
}

int set_keymode(int fd, int mode)
{
  int error;

  error =  fcntl(fd, FCTL_SET_KEY, (void*)mode);
  return error;
}

int get_keymode(int fd, int *mode)
{
  int error;

  error = fcntl(fd, FCTL_GET_KEY, mode);
  return error;
}
