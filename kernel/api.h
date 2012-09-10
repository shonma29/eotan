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
#define	SYS_CHG_PRI	(7)
#define	SYS_REL_WAI	(8)
#define	SYS_GET_TID	(9)

#define	SYS_SUS_TSK	(10)
#define	SYS_RSM_TSK	(11)

#define SYS_ACRE_FLG	(12)
#define SYS_DEL_FLG   	(13)
#define SYS_SET_FLG   	(14)
#define SYS_CLR_FLG  	(15)
#define SYS_WAI_FLG   	(16)

#define SYS_CRE_MBF	(17)
#define SYS_ACRE_MBF	(18)
#define SYS_DEL_MBF	(19)
#define SYS_SND_MBF	(20)
#define SYS_PSND_MBF	(21)
#define SYS_RCV_MBF	(22)

#define SYS_SET_TIM	(23)
#define SYS_GET_TIM	(24)
#define SYS_DLY_TSK	(25)

#define SYS_DEF_ALM	(26)

#define SYS_DEF_INT	(27)

#define SYS_VSYS_INF	(28)
#define SYS_DBG_PUTS	(29)

#define SYS_VCRE_REG	(30)
#define SYS_VDEL_REG	(31)
#define SYS_VMAP_REG	(32)
#define SYS_VUNM_REG	(33)
#define SYS_VDUP_REG	(34)
#define SYS_VPUT_REG	(35)
#define SYS_VGET_REG	(36)
#define SYS_VSTS_REG	(37)

#define SYS_VGET_PHS	(38)

#define SYS_VSYS_MSC	(39)
#define SYS_VCPY_STK	(40)
#define SYS_VSET_CTX	(41)
#define SYS_VUSE_FPU	(42)

#define SYS_CRE_POR	(43)
#define SYS_ACRE_POR	(44)
#define SYS_DEL_POR	(45)
#define SYS_CAL_POR	(46)
#define SYS_ACP_POR	(47)
#define SYS_RPL_RDV	(48)

#endif /* __CORE_API_H__ */
