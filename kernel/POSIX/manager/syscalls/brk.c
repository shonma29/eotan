/*

B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001, Tomohide Naniwa

*/
/*
 * $Log$
 *
 */

#include "posix.h"

W psc_brk_f(struct posix_request *req)
{
    struct proc *myprocp;
    W err, mypid, i;
    T_REGION reg;
    VP start;
    UW size;

    mypid = req->procid;
    err = proc_get_procp(mypid, &myprocp);
    if (err)
	return err;

    err = vsts_reg(req->caller, 3, (VP) & reg);
#ifdef DEBUG
    printk("[PM] err = %d sa %x, min %x, max %x, ea %x\n",
	   err, reg.start_addr, reg.min_size, reg.max_size,
	   req->param.par_brk.end_adr);
#endif

    err = EP_OK;
    start = reg.start_addr + reg.min_size;
    if (start > req->param.par_brk.end_adr) {
	/* region ��̾� */
	size = start - req->param.par_brk.end_adr;
	for (i = 1; i <= (size >> PAGE_SHIFT); ++i) {
	    err = shorten_vm(myprocp, (UW) start - (i << PAGE_SHIFT));
	    if (err)
		break;
	}
    } else if (start < req->param.par_brk.end_adr) {
	/* region ����� */
	size = req->param.par_brk.end_adr - start;
	for (i = 0; i < PAGES(size); ++i) {
	    err = grow_vm(myprocp, (UW) start + (i << PAGE_SHIFT),
			  VM_READ | VM_WRITE | VM_USER);
	    if (err)
		break;
	}
    }

    if (err) {
	put_response(req, err, -1, 0, 0);
	return (FAIL);
    }

    /* for debug */
#ifdef DEBUG
    vsts_reg(req->caller, 3, (VP) & reg);
    printk("[PM] after brk sa %x, min %x, max %x\n",
	   reg.start_addr, reg.min_size, reg.max_size);
#endif

    put_response(req, EP_OK, 0, 0, 0);
    return (SUCCESS);
}
