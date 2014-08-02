/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/* posix_syscall.h - POSIX 環境マネージャ用のヘッダファイル
 *		     (システムコール関連の定義)
 *
 * Note:
 *	PSC (and psc) = Posix System Call
 *
 */

#ifndef __FS_API_H__
#define __FS_API_H__	1

#include <core.h>
#include <sys/syscall.h>

struct posix_syscall
{
#ifdef DEBUG
  B	*name;
  W	callno;
#endif
  void	(*syscall)(RDVNO rdvno, struct posix_request *);
};


extern struct posix_syscall	syscall_table[];


extern void	psc_access_f (RDVNO rdvno, struct posix_request *);
extern void	psc_chdir_f (RDVNO rdvno, struct posix_request *);
extern void	psc_chmod_f (RDVNO rdvno, struct posix_request *);
extern void	psc_close_f (RDVNO rdvno, struct posix_request *);
extern void	psc_dup_f (RDVNO rdvno, struct posix_request *);
extern void	psc_dup2_f (RDVNO rdvno, struct posix_request *);
extern void	psc_exec_f (RDVNO rdvno, struct posix_request *);
extern void	psc_exit_f (RDVNO rdvno, struct posix_request *);
extern void	psc_fcntl_f (RDVNO rdvno, struct posix_request *);
extern void	psc_fork_f (RDVNO rdvno, struct posix_request *);
extern void	psc_kill_f (RDVNO rdvno, struct posix_request *);
extern void	psc_link_f (RDVNO rdvno, struct posix_request *);
extern void	psc_lseek_f (RDVNO rdvno, struct posix_request *);
extern void	psc_mkdir_f (RDVNO rdvno, struct posix_request *);
extern void	psc_open_f (RDVNO rdvno, struct posix_request *);
extern void	psc_read_f (RDVNO rdvno, struct posix_request *);
extern void	psc_rmdir_f (RDVNO rdvno, struct posix_request *);
extern void	psc_fstat_f (RDVNO rdvno, struct posix_request *);
extern void	psc_umask_f (RDVNO rdvno, struct posix_request *);
extern void	psc_unlink_f (RDVNO rdvno, struct posix_request *);
extern void	psc_waitpid_f (RDVNO rdvno, struct posix_request *);
extern void	psc_write_f (RDVNO rdvno, struct posix_request *);
extern void	psc_mount_f (RDVNO rdvno, struct posix_request *);
extern void	psc_umount_f (RDVNO rdvno, struct posix_request *);
extern void	psc_statfs_f (RDVNO rdvno, struct posix_request *);
extern void	psc_getdents_f (RDVNO rdvno, struct posix_request *);
extern void psc_bind_device_f(RDVNO rdvno, struct posix_request *);

#endif /* #define __FS_API_H__ */
