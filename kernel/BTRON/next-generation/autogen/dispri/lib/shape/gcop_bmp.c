/*
B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gcop_bmp
** Description: -
*/

ER
gcop_bmp (W srcid,RPTR sr,W dstid,RPTR dr,GSCAN *mask,W mode)
{
	return call_btron(BSYS_GCOP_BMP, srcid, sr, dstid, dr, *mask, mode);
}
