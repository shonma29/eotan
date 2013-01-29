/*
   psaux.h

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __DUMMY_H__
#define __DUMMY_H__

#include <core.h>
#include "../port-manager/port-manager.h"
#include "../fs/fs.h"
#include "../console/console.h"

#define PSAUX_DRIVER "driver.psaux"

/* ioctl */
#define PSAUX_CLEAR       1
#define PSAUX_CHANGEMODE  2

/* driver mode */
#define WAITMODE        0x0000
#define NOWAITMODE      0x0001

/* kbc.c */
extern void init_buffer();
extern W kbc_wr_command(B command);
extern W kbc_wr_data(B data);
extern B kbc_rd_data(void);
extern void psaux_interrupt();
extern void init_interrupt();
extern W psaux_data_in(B* val);
extern W psaux_data_out(B data);

#endif
