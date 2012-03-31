/*

B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001, Tomohide Naniwa

*/
/*
 * $Log: link.c,v $
 * Revision 1.1  1999/03/21 14:01:51  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include "posix.h"

W psc_link_f(struct posix_request *req)
{
    B src[MAX_NAMELEN], dst[MAX_NAMELEN];
    struct access_info acc;
    W errno;

    errno = vget_reg(req->caller, req->param.par_link.src,
		     req->param.par_link.srclen + 1, src);
    if (errno) {
	/* �ѥ�̾�Υ��ԡ����顼 */
	if (errno == E_PAR)
	    put_response(req, EP_INVAL, -1, 0, 0);
	else
	    put_response(req, EP_FAULT, -1, 0, 0);

	return (FAIL);
    }
    errno = vget_reg(req->caller, req->param.par_link.dst,
		     req->param.par_link.dstlen + 1, dst);
    if (errno) {
	/* �ѥ�̾�Υ��ԡ����顼 */
	if (errno == E_PAR)
	    put_response(req, EP_INVAL, -1, 0, 0);
	else
	    put_response(req, EP_FAULT, -1, 0, 0);

	return (FAIL);
    }

    /* �ץ����Υ桼�� ID �ȥ��롼�� ID ��
     * ���Ф���
     * ���ξ���˴�Ť��ơ��ե���������Ǥ��뤫�ɤ�����
     * ���ꤹ�롣
     */
    errno = proc_get_euid(req->procid, &(acc.uid));
    if (errno) {
	put_response(req, errno, 0, 0, 0);
	return (FAIL);
    }
    errno = proc_get_egid(req->procid, &(acc.gid));
    if (errno) {
	put_response(req, errno, 0, 0, 0);
	return (FAIL);
    }

    errno = fs_link_file(req->procid, src, req->param.par_link.srclen,
			 dst, req->param.par_link.dstlen, &acc);
    if (errno) {
	put_response(req, errno, 0, 0, 0);
	return (FAIL);
    }
    put_response(req, EP_OK, 0, 0, 0);
    return (SUCCESS);
}
