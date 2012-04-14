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

#include "h/itron.h"
#include "h/errno.h"
#include "../../servers/port-manager.h"
#include "../../../servers/fs/posix.h"
#include "../console/console.h"

#define PSAUX_DRIVER "driver.psaux"

/* ioctl */
#define PSAUX_CLEAR       1
#define PSAUX_CHANGEMODE  2

/* driver mode */
#define WAITMODE        0x0000
#define NOWAITMODE      0x0001

#endif
