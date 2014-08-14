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
extern void	psc_unlink_f (RDVNO rdvno, struct posix_request *);
extern void	psc_waitpid_f (RDVNO rdvno, struct posix_request *);
extern void	psc_write_f (RDVNO rdvno, struct posix_request *);
extern void	psc_mount_f (RDVNO rdvno, struct posix_request *);
extern void	psc_unmount_f (RDVNO rdvno, struct posix_request *);
extern void	psc_statvfs_f (RDVNO rdvno, struct posix_request *);
extern void	psc_getdents_f (RDVNO rdvno, struct posix_request *);
extern void psc_bind_device_f(RDVNO rdvno, struct posix_request *);

#ifdef DEBUG
#define s(q, v) q,v,
#else
#define s(q, v)
#endif


static struct posix_syscall	syscall_table[] =
{
  { s("chdir",		PSC_CHDIR)		psc_chdir_f },
  { s("chmod",		PSC_CHMOD)		psc_chmod_f },
  { s("close",		PSC_CLOSE)		psc_close_f },
  { s("dup",		PSC_DUP)		psc_dup_f },
  { s("exec",		PSC_EXEC)		psc_exec_f },
  { s("exit",		PSC_EXIT)		psc_exit_f },
  { s("fcntl",		PSC_FCNTL)		psc_fcntl_f },
  { s("fork",		PSC_FORK)		psc_fork_f },
  { s("fstat",		PSC_FSTAT)		psc_fstat_f },
  { s("link",		PSC_LINK)		psc_link_f },
  { s("lseek",		PSC_LSEEK)		psc_lseek_f },
  { s("mkdir",		PSC_MKDIR)		psc_mkdir_f },
  { s("open",		PSC_OPEN)		psc_open_f },
  { s("read",		PSC_READ)		psc_read_f },
  { s("rmdir",		PSC_RMDIR)		psc_rmdir_f },
  { s("unlink",		PSC_UNLINK)		psc_unlink_f },
  { s("waitpid",		PSC_WAITPID)		psc_waitpid_f },
  { s("write",		PSC_WRITE)		psc_write_f },
  { s("getdents",		PSC_GETDENTS)		psc_getdents_f },
  { s("mount",		PSC_MOUNT)		psc_mount_f },
  { s("statvfs",		PSC_STATVFS)		psc_statvfs_f },
  { s("unmount",		PSC_UNMOUNT)		psc_unmount_f },
  { s("kill",		PSC_KILL)		psc_kill_f },
  { s("dup2",		PSC_DUP2)		psc_dup2_f },
  { s("bind_device", PSC_BIND_DEVICE) psc_bind_device_f },
};

#endif /* #define __FS_API_H__ */
