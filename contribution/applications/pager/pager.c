/*
   pager.c ver 1.0

   Contributed by Mr. ∫‰À‹°˜∞¶√Œ∏©
   Modified by Tomohide Naniwa.
            by NAKANISHI Shin.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Last modified: Jan 9, 2002.

 */

#include    <stdlib.h>
        
int read_ln(int fd, char *buf, int  buf_len);
void prompt();
int p_lines();

main(int argc, char **argv)
{
  int     fd;
  int     line;
  char    linebuf[1024];
  
  if ( argc < 2) {
    printf("Usage: %s file\n", argv[0]);
    _exit(-1);
  }

  if ((fd = open(argv[1], O_RDONLY)) < 0) {
    printf("%s\n", argv[1]);
    _exit(-1);
  }

  while(1) {
    line = 1;
    while(line < p_lines()) {
      if (read_ln(fd, linebuf, sizeof(linebuf)) <= 0) {
        close(fd);
	prompt();
	line = -1;
	break;
      }
      printf("%s\n", linebuf);
      line ++;
    }
    if (line < 0) break;
    prompt();
  }
}
    
void prompt()
{
  char    c;
    
  printf("Type any charcter for next page: ");
  read(0, &c, 1);
  switch (c) {
  case 'q':
    _exit(1);
  case 'Q':
    _exit(1);
  default:
    break;
  }
  write(1, "\n", 1);
}

int read_ln(int fd, char *buf, int  buf_len)
{
  int  len = 0;
  int  n;

  while (buf_len > len) {
    n = read(fd, buf, 1);
    if (n <= 0) {
      if (n < 0) len = n;
      break;
    }
    len ++;
    if(*buf == '\n') break;
    buf ++;
  }
  *buf=0;
  return(len);
}
#define MAX_LINES       40
#define MAX_COLS  120
#define DEFAULT_LINES   25
#define DEFAULT_COLS    80

int p_lines(void)
{
  char *s;
  struct winsize {
    unsigned short int ws_row;
    unsigned short int ws_col;
    unsigned short int ws_xpixel;
    unsigned short int ws_ypixel;
  } ws;


  int LINES, COLS, i;

  fcntl(1, (0x00000013 << 16), &ws);
  LINES = ws.ws_row;
  LINES = (LINES > MAX_LINES)? MAX_LINES:LINES;
  COLS = ws.ws_col;
  COLS = (COLS > MAX_COLS)? MAX_COLS:COLS;

  return (LINES);
}
