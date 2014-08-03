/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/ITRON/kernlib/device.h,v 1.4 2000/04/03 14:37:34 naniwa Exp $ */

/*
 * $Log: device.h,v $
 * Revision 1.4  2000/04/03 14:37:34  naniwa
 * to call timer handler in task
 *
 * Revision 1.3  2000/02/27 15:31:45  naniwa
 * to work as multi task OS
 *
 * Revision 1.2  1999/12/29 17:09:35  monaka
 * In BTRON3, Variable Type TCODE is replaced to TC.
 *
 * Revision 1.1  1999/04/18 17:48:33  monaka
 * Port-manager and libkernel.a is moved to ITRON. I guess it is reasonable. At least they should not be in BTRON/.
 *
 * Revision 1.4  1999/04/13 04:14:51  monaka
 * MAJOR FIXcvs commit -m 'MAJOR FIX!!! There are so many changes, modifys, fixes. Sorry but I can't remember all of those. For example, all the manager and driver programmer have got power to access all ITRON systemcall. (My works is just making access route to ITRON. I don't know what happens in the nuclus.'! There are so many changes, modifys, fixes. Sorry but I can't remember all of those. For example, all the manager and driver programmer have got power to access all ITRON systemcall. (My works is just making access route to ITRON. I don't know what happens in the nuclus.
 *
 * Revision 1.3  1998/05/23 15:31:32  night
 * enum 宣言の最後の要素についていた ',' を削除した。
 * (ANSI の文法では、',' は最後の要素にはつけられない)
 *
 * Revision 1.2  1997/05/06 12:57:46  night
 * デバイスからの返答メッセージ (DDEV_CTL_RES と DEV_CTL_RES) に
 * 余分なデータ領域を追加。
 *
 * Revision 1.1  1996/07/22  23:52:05  night
 * 最初の登録
 *
 * Revision 1.4  1995/12/05 15:00:51  night
 * 註釈の変更
 *
 * Revision 1.3  1995/09/21  15:51:40  night
 * ソースファイルの先頭に Copyright notice 情報を追加。
 *
 * Revision 1.2  1995/09/17  17:05:00  night
 * デバイスドライバインタフェースを全面変更。
 *
 * Revision 1.1  1995/09/14  04:38:10  night
 * 最初の登録
 *
 * Revision 1.1  1995/09/12  18:18:14  night
 * 最初の登録
 *
 *
 */

/* device.h --- デバイスドライバのための構造体の定義
 *
 */


#ifndef __KERNLIB_DEVICE_H__
#define __KERNLIB_DEVICE_H__	1

#include <core/types.h>

#define DEV_BUF_SIZE (1024)

/*
 * メッセージ
 */
typedef struct {
  W	msgtyp;		/* メッセージ識別番号				*/
} SVC_REQ;

/*
 * ドライバへ渡すコマンド一覧
 */
enum driver_command
{
  DEV_OPN,
  DEV_CLS,
  DEV_REA,
  DEV_WRI,
  DEV_CTL
};

/* =========================================================================
 デバイスドライバ関連のメッセージを定義する。

 メッセージは1つの要求(処理)につき、2種類定義する。

 すなわち、

 デバイスドライバマネージャ <-> デバイスドライバ 間の送受信

 である。

 ここでは、以下の型を定義する。


                   要求(送信元) 返信(送信元)  
     処理              user        driver    
 ------------------------------------------------------------
 デバイスオープン  DDEV_OPN_REQ DDEV_OPN_RES  
 デバイスクローズ  DDEV_CLS_REQ DDEV_CLS_RES  
 読み込み          DDEV_REA_REQ DDEV_REA_RES  
 書き込み          DDEV_WRI_REQ DDEV_WRI_RES  
 ------------------------------------------------------------

*/

/* デバイスオープン（デバイスドライバへ送るメッセージ）
 *
 *    ・デバイスドライバは必要が無ければ何の処理を行う必要もない。
 *
 */
typedef struct {
  W	dd;	/* デバイスドライバマネージャ管理情報 */
  		/* デバイスドライバはこの値を使用せず，応答メッセージの */
                /* 先頭に登録する */
} DDEV_OPN_REQ;

/* デバイスオープン（デバイスドライバマネージャへの応答）
 *
 */
typedef struct {
  W dd;		/* デバイスドライバマネージャ管理情報 */
  UW size;
  ER errcd;
  W errinfo;    /* エラー詳細情報 */
} DDEV_OPN_RES;

/* デバイスクローズ（デバイスドライバへ送るメッセージ）
 *
 */
typedef struct {
  W dd;       /* デバイスドライバマネージャ管理情報 */
              /* デバイスドライバはこの値を使用せず，応答メッセージの */
              /* 先頭に登録する */
  W eject;    /* イジェクト指定	*/
} DDEV_CLS_REQ;

/* デバイスクローズ（デバイスドライバマネージャへの応答）
 */
typedef struct {
  W dd;        /* デバイスドライバマネージャ管理情報	*/
  ER errcd;
  W errinfo;   /* エラー詳細情報	*/
} DDEV_CLS_RES;

/* デバイスからの読み込み（デバイスドライバへ送るメッセージ）
 */
typedef struct {
  W dd;		/* デバイスドライバマネージャ管理情報	*/
  		/* デバイスドライバはこの値を使用せず，応答メッセージの	*/
                /* 先頭に登録する	*/
  UW	start;     /* デバイスの先頭からの位置(byte)	*/
  UW	size;      /* 書き込み量(byte) */
} DDEV_REA_REQ;

/* デバイスからの読み込み（デバイスドライバマネージャへの応答）
 *
 *    ・デバイスドライバは指定されたサイズのデータを一度に読み込んでも良いし、ブ
 *      ロック単位に分割して読み込んでもよい。分割して読み込む場合は分割情報（先
 *      頭ブロックフラグと最終ブロックフラグ）を最初のブロックと最後のブロックに
 *      登録する。一度に読み込む場合は、先頭ブロックフラグと最終ブロックフラグの
 *      両方を登録する。
 */
typedef struct {
  W dd;        /* デバイスドライバマネージャ管理情報	*/
  ER errcd;
  W errinfo;   /* エラー詳細情報	*/
  W split;     /* 分割情報	*/
  UW	a_size; /* 実際に読み込んだbyte数	*/
  UB dt[DEV_BUF_SIZE];     /* 読み込んだデータ	*/
} DDEV_REA_RES;

/* デバイスへの書き込み（デバイスドライバへ送るメッセージ）
 *
 * ・デバイスドライバマネージャは、メッセージバッファの空き容量を検査し、必要
 *   が有れば、データを分割して要求を送る。（前回のミーティングで、ブロック単
 *   位に分割して要求を発行するというように発言したが、それは取り消します。デ
 *   バイスドライバが１ブロック単位での書き込み要求しか受け取りたくない場合に
 *   は、メッセージバッファのサイズを調整することによりデバイスドライバ主導で
 *   実現可能となります）
 */
typedef struct {
  W	dd;	/* デバイスドライバマネージャ管理情報	*/
		/* デバイスドライバはこの値を使用せず，応答メッセージの	*/
		/* 先頭に登録する	*/
  UW	start;		/* デバイスの先頭からの位置（byte）	*/
  UW	size;		/* 書き込み量（byte）	*/
  UB	dt[DEV_BUF_SIZE];	/* 書き込むデータ(最大)	*/
} DDEV_WRI_REQ;

/* デバイスへの書き込み（デバイスドライバマネージャへの応答）
 */
typedef struct {
  W dd;		/* デバイスドライバマネージャ管理情報	*/
  ER errcd;
  W errinfo;	/* エラー詳細情報	*/
  UW	a_size;	/* 実際に書き込んだbyte数	*/
} DDEV_WRI_RES;

/*
 * ドライバマネージャとデバイスドライバとのやりとりに使用するメッセージパケット
 * kernlib/message.h で定義している SVC_REQ 形式が頭につく。
 * 要求する時に使用する。
 */
typedef struct DDEV_REQ
{
  SVC_REQ		header;		/* 汎用メッセージ */

  union 
    {
      DDEV_OPN_REQ	opn_req;
      DDEV_CLS_REQ	cls_req;
      DDEV_REA_REQ	rea_req;	
      DDEV_WRI_REQ	wri_req;
    } body;
} DDEV_REQ;

/* 
 * ドライバマネージャとデバイスドライバとのやりとりに使用するメッセージパケット
 * kernlib/message.h で定義している SVC_REQ 形式が頭につく。
 * ドライバが返答する時に使用する。
 */
typedef struct DDEV_RES
{
  SVC_REQ		header;		/* 汎用メッセージ */

  union 
    {
      DDEV_OPN_RES	opn_res;
      DDEV_CLS_RES	cls_res;
      DDEV_REA_RES	rea_res;	
      DDEV_WRI_RES	wri_res;
    } body;
} DDEV_RES;

typedef union _devmsg_t {
    DDEV_REQ req;
    DDEV_RES res;
} devmsg_t;


#define DEVID(id)	(id >> 16)
#define UNIT(id)	((id >> 8) & 0xff)
#define SUBUNIT(id)	(id & 0xff)
#define MAKEDEVID(dev,unit,subunit)	((dev << 16) | (unit << 8) | subunit)

#endif /* __KERNLIB_DEVICE_H__ */
