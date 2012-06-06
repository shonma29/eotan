/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/*
 *
 * $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/ITRON/h/syscall.h,v 1.10 1999/12/29 17:03:15 monaka Exp $
 */

#ifndef __CORE_API_H__
#define __CORE_API_H__	1


/* システムコール番号の定義 
 */
#define SYS_CRE_TSK	(1)
#define	SYS_DEL_TSK	(2)
#define	SYS_STA_TSK	(3)
#define	SYS_EXT_TSK	(4)
#define	SYS_EXD_TSK	(5)
#define	SYS_TER_TSK	(6)
#define	SYS_DIS_DSP	(7)
#define	SYS_ENA_DSP	(8)
#define	SYS_CHG_PRI	(9)
#define	SYS_ROT_RDQ	(10)
#define	SYS_REL_WAI	(11)
#define	SYS_GET_TID	(12)

#define	SYS_SUS_TSK	(14)
#define	SYS_RSM_TSK	(15)

#define	SYS_SLP_TSK	(17)
#define	SYS_TSLP_TSK	(18)
#define	SYS_WUP_TSK	(19)
#define SYS_CAN_WUP	(20)

#define SYS_CRE_FLG   	(28)
#define SYS_DEL_FLG   	(29)
#define SYS_SET_FLG   	(30)
#define SYS_CLR_FLG  	(31)
#define SYS_WAI_FLG   	(32)

#define SYS_CRE_MBF	(36)
#define SYS_DEL_MBF	(37)
#define SYS_SND_MBF	(38)
#define SYS_PSND_MBF	(39)

#define SYS_RCV_MBF	(41)

#define SYS_DIS_INT   	(44)
#define SYS_ENA_INT   	(45)

#define SYS_SET_TIM	(58)
#define SYS_GET_TIM	(59)
#define SYS_DLY_TSK	(60)

#define SYS_DEF_ALM	(64)

#define SYS_DEF_INT	(67)

#define SYS_VSYS_INF	(99)
#define SYS_DBG_PUTS	(100)

#define SYS_VCRE_REG	(101)
#define SYS_VDEL_REG	(102)
#define SYS_VMAP_REG	(103)
#define SYS_VUNM_REG	(104)
#define SYS_VDUP_REG	(105)
#define SYS_VPRT_REG	(106)
#define SYS_VSHR_REG	(107)
#define SYS_VPUT_REG	(108)
#define SYS_VGET_REG	(109)
#define SYS_VSTS_REG	(110)

#define SYS_VGET_PHS	(111)

#define SYS_VSYS_MSC	(122)
#define SYS_VCPY_STK	(123)
#define SYS_VSET_CTX	(124)
#define SYS_VUSE_FPU	(125)

#endif /* __CORE_API_H__ */
