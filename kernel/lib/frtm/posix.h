/*
   FoRTH Modoki
   posix.h ver 1.4.8
   Copyright (C) 1999-2002 Tomohide Naniwa. All rights reserved.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Last modified: Feb. 14, 2002.
*/

extern void f_cat(void);
extern void f_dir(void);
extern void f_open(void);
extern void f_close(void);
extern void f_write(void);
extern void f_link(void);
extern void f_unlink(void);
extern void f_mkdir(void);
extern void f_rmdir(void);
extern void f_chdir(void);
extern void f_chmod(void);
extern void f_pwd(void);
extern void f_exec(void);
extern void f_daemon(void);
extern void f_load(void);
#if 0
extern void f_multi(void);
extern void f_single(void);
#endif
extern void f_wdload(void);
extern void f_mount(void);
extern void f_umount(void);
extern void f_ps(void);
extern void f_df(void);
extern void f_sleep(void);
extern void f_usleep(void);
