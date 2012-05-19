/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/*************************************************************************
 *
 *		2nd BOOT types definition.
 *
 * $Header: /usr/local/src/master/B-Free/Program/btron-pc/boot/2nd/types.h,v 1.2 1996/11/13 16:29:05 night Exp $
 *
 * $Log: types.h,v $
 * Revision 1.2  1996/11/13 16:29:05  night
 * TCODE の型を unsigned short から unsigned char に変更した。
 *
 * Revision 1.1  1996/05/11  10:45:07  night
 * 2nd boot (IBM-PC 版 B-FREE OS) のソース。
 *
 * Revision 1.3  1995/09/21  15:50:44  night
 * ソースファイルの先頭に Copyright notice 情報を追加。
 *
 * Revision 1.2  1994/07/30  17:37:20  night
 * ファイル中の日本語文字をすべて EUC コードに変更。
 *
 * Revision 1.1  1993/10/11  21:30:02  btron
 * btron/386
 *
 * Revision 1.1.1.1  93/01/14  12:30:28  btron
 * BTRON SYSTEM 1.0
 * 
 * Revision 1.1.1.1  93/01/13  16:50:28  btron
 * BTRON SYSTEM 1.0
 * 
 */

#ifndef	__TYPES_H__
#define	__TYPES_H__	1

typedef	char		SBYTE;
typedef unsigned char	BYTE;
typedef	short		WORD16;
typedef	unsigned short	UWORD16;
typedef long		WORD32;
typedef unsigned long	UWORD32;
typedef long		WORD;
typedef unsigned long	UWORD;
typedef long		LONG;
typedef unsigned long	ULONG;
typedef float		FLOAT;
typedef double		DOUBLE;
typedef void		VOID;

typedef unsigned char	SCODE;			/* ASCII code */
typedef unsigned char	TCODE;			/* TRON code */

typedef char		*SBPTR;
typedef unsigned char	*BPTR;
typedef short		*WPTR;
typedef unsigned short	*UWPTR;
typedef long		*LPTR;
typedef unsigned long	*ULPTR;

typedef TCODE		*TPTR;
typedef SCODE		*SPTR;
typedef int		(*FUNCP)();

#ifndef SNULL
#define SNULL		(BYTE)0
#endif
#ifndef TNULL
#define TNULL		(WORD)0
#endif
#ifndef NULL
#define NULL		(LONG)0
#endif

#define LOCAL		static
#define GLOBAL		/* 何も定義しない */
#define IMPORT		extern

#define MAXINT		(0x7fffffff)

#endif /* __TYPES_H__ */

