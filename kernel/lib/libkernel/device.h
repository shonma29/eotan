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

#include "message.h"

/*
 * ドライバへ渡すコマンド一覧
 */
enum driver_command
{
  DEV_OPN,
  DEV_CLS,
  DEV_REA,
  DEV_WRI,
  DEV_CTL,
  DEV_CHG,
  DEV_PRD,
  DEV_RLY
};

/* =========================================================================
 デバイスドライバ関連のメッセージを定義する。

 メッセージは1つの要求(処理)につき、4種類定義する。

 すなわち、

  LOWLIB <-> デバイスドライバマネージャ 間の送受信
 デバイスドライバマネージャ <-> デバイスドライバ 間の送受信

 である。

 ここでは、以下の型を定義する。


                          要求(送信元)                 返信(送信元)  
     処理           LOWLIB         devman          driver       devman
 -------------------------------------------------------------------------
 デバイスの登録    DEV_REG_REQ       ---           ----      DEV_REG_RES
 デバイスの再登録  DEV_RREG_REQ      ---           ----      DEV_RREG_RES
 デバイス登録解除  DEV_UNREG_REQ     ---           ----      DEV_UNREG_RES 
 デバイスオープン  DEV_OPN_REQ  DDEV_OPN_REQ   DDEV_OPN_RES  DEV_OPN_RES
 デバイスクローズ  DEV_CLS_REQ  DDEV_CLS_REQ   DDEV_CLS_RES  DEV_CLS_RES
 読み込み          DEV_REA_REQ  DDEV_REA_REQ   DDEV_REA_RES  DEV_REA_RES
 書き込み          DEV_WRI_REQ  DDEV_WRI_REQ   DDEV_WRI_RES  DEV_WRI_RES
 デバイス制御      DEV_CTL_REQ  DDEV_CTL_REQ   DDEV_CTL_RES  DEV_CTL_RES
 モード変更        DEV_CHG_REQ      ---           ---        DEV_CHG_RES
 管理情報取り出し  DEV_STS_REQ      ---           ---        DEV_STS_RES
 論理デバイス名    DEV_GET_REQ      ---           ---        DEV_GET_RES
 デバイス取り出し  DEV_LST_REQ      ---           ---        DEV_LST_RES
 -------------------------------------------------------------------------

*/

typedef struct DEV_INFO
{
  W	type;
} DEV_INFO;

typedef struct DEV_STAT
{
  W	type;
} DEV_STAT;


/* デバイスの登録 
 *
 * ・デバイス名の指定は「種別」のみとする。
 * ・周辺核（ファイルマネージャ等）から要求を出す場合は、プロセスＩＤとして特
 *   殊な値を指定することにします。（全てのシステムコールに共通）
 */
typedef struct {
  ID		pid;     		/* システムコールを発行したプロセス */
  DEV_INFO	info;	/* */
  DEV_STAT	stat;	/* */
} DEV_REG_REQ;


/* デバイスの登録（の応答）
 * ・正常終了した場合にはerrcdにユニット番号が返却される
 *   ”ｈｄ”という種別でデバイス登録が要求されて、０が返却されたら、そのデバ
 *   イスは”ｈｄａ”となる。
 *   ＯＳのブート毎にデバイス名が変わる可能性があるが、デバイス（ユニット）名
 *   を一意にするための工夫はデバイスドライバマネージャの外部で行われることを
 *   前提としている。
 * ・ユニット番号は英字１文字で表現するので同じ種別のデバイスは最大２６ユニッ
 *   トまでしか登録できない。（暫定）
 */
typedef struct {
  W	errcd;
  W	devno;	/* デバイス番号 */
} DEV_REG_RES;



/* デバイスの再登録（今の所、実装の予定はない）
 *  ・新しいデバイス名の指定は「種別」のみとする。
 *  ・古いデバイス名の指定は「論理デバイス名orユニット名」である。
 *  ・デバイスを排他モードでオープンしたときと同じ検査を行い、成功した場合にの
 *    み再登録が可能。（どこからもオープンされていない状態）
 *  ・デバイスドライバの登録情報はスタック状にネストして行われるので、再登録し
 *    たデバイスドライバを削除すれば、再登録以前の（古い）デバイスドライバが使
 *    用されることになる。
 *  ・古いデバイス名にユニット名を指定した場合は、デバイス全体が新しいデバイス
 *    に置き変わるが、古いデバイス名に論理デバイス名を指定した場合は、古いデバ
 *    イス内の１つのサブユニットのみが新しく定義される。したがって、残りのサブ
 *    ユニットは、再登録以前と変わらずにアクセスすることが可能である。
 *  ・古いデバイスドライバには、「イベント通知先変更」のメッセージが送られます
 *
 *  例）ＰＣＭＣＩＡスロットにＳＣＳＩカードが接続され、そこにＨＤが接続される
 *      場合は、以下の手順でデバイスドライバを登録する
 *      （１） ＰＣＭＣＩＡドライバを登録する。
 *      （２） ＰＣＭＣＩＡの１つのサブユニットに対してＳＣＳＩドライバを再登
 *             録する。
 *      （３） ＳＣＳＩの１つのサブユニット（ＩＤ）にたいしてＨＤドライバを再
 *             登録する。
 *
 *      上記の状態で、ＳＣＳＩの未使用サブユニットに（例えば）プリンタドライバ
 *      をさらに再登録することが可能である。
 *      また、ＰＣＭＣＩＡの未使用サブユニットにモデム（ＳＩＯ）ドライバを再登
 *      録することも可能である。（ＰＣＭＣＩＡドライバのサブユニット数は、使用
 *      される可能性のあるカードの種類分登録される事を前提としている）
 *
 *     この方法の欠点は、任意順序でデバイスドライバを登録できないことです。
 */
typedef struct {
  ID pid;         /* システムコールを発行したプロセス */
  TC dev[8];   /* 置き換える(古い)デバイス名(論理デバイス名orユニット名) */
  DEV_INFO info;  /* 新しく定義するデバイスの情報 */
  DEV_STAT stat;  /* 新しく定義するデバイスの情報 */
} DEV_RREG_REQ;


/* デバイスの再登録（の応答）
 *　・正常終了した場合にはerrcdにユニット番号が返却される
 *  ・古いデバイスドライバへメッセージを送るかどうかは、新しいデバイスドライバ
 *    に依存する。（古いデバイスドライバを拡張する事を前提としてこの機能を用意
 *    するので、メッセージを送らないということは考えにくい）
 *  ・デバイスドライバ間のメッセージ通信は直接行われ、デバイスドライバマネージ
 *    ャは関与しない。
 */
typedef struct {
  W	errcd;
  W	devno;   /* デバイス番号 */
  ID	mbfid;   /* 古いデバドラへメッセージを送信するためのメッセージバッファ */
} DEV_RREG_RES;



/* デバイス登録解除
 *  ・ユニット名で指定されたデバイスの登録情報を削除する。
 *  ・削除した時点でデバイスの処理待ちになっているプロセスには、エラーが返却さ
 *    れる。
 *  ・削除した時点でデバイスをオープンしているプロセスが有った場合、強制的にク
 *    ローズされる。ただしプロセスに対しては何の通知も行われず、そのプロセスが
 *    デバイスドライバマネージャになんらかの要求を発行した時点でエラー(E_DD)が
 *    返却されることになる。
 *  ・削除されたデバイスのユニット番号は欠番となり、次にデバイスが登録されても
 *    同じユニット番号が使用されることは無い。
 */
typedef struct {
  ID	pid;		/* システムコールを発行したプロセス */
  TC	dev[8];		/* デバイス名（ユニット名） */
} DEV_UNREG_REQ;


/* デバイスオープン(opn_dev)
 */
typedef struct {
  ID	pid;      /* システムコールを発行したプロセス	*/
  TC	dev[8];   /* デバイス名（論理デバイス名）	*/
  UW	o_mode;
} DEV_OPN_REQ;


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
  W errcd;
  W errinfo;    /* エラー詳細情報 */
} DDEV_OPN_RES;


/* デバイスオープン（ＬｏｗＬｉｂへの応答）
 * 　・正常終了した場合にはerrcdにデバイスディスクリプタが返却される
 */
typedef struct {
  W errcd;
  W errinfo;   /* エラー詳細情報 */
} DEV_OPN_RES;


/* デバイスクローズ(cls_dev)
 *
 */
typedef struct {
  ID	pid;	/* システムコールを発行したプロセス	*/
  W	dd;
  W	eject;	/* イジェクト指定 */
} DEV_CLS_REQ;

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
  W errcd;
  W errinfo;   /* エラー詳細情報	*/
} DDEV_CLS_RES;

/* デバイスクローズ（ＬｏｗＬｉｂへの応答）
 */
typedef struct {
  W	errcd;
  W	errinfo;   /* エラー詳細情報	*/
} DEV_CLS_RES;



/* デバイスからの読み込み(rea_dev) 
 *   ・ＬｏｗＬｉｂは、自分の用意しているメッセージバッファのサイズ以上の要求を
 *     発行してはいけない。（ＬｏｗＬｉｂ内で分割して発行する）
 */
typedef struct {
  ID pid;         /* システムコールを発行したプロセス	*/
  W dd;        /* デバイスディスクリプタ	*/
  W	start;     /* デバイスの先頭からの位置（物理ブロック単位）	*/
  W	size;      /* 読み込む量（物理ブロック単位）	*/
} DEV_REA_REQ;

/* デバイスからの読み込み（デバイスドライバへ送るメッセージ）
 */
typedef struct {
  W dd;		/* デバイスドライバマネージャ管理情報	*/
  		/* デバイスドライバはこの値を使用せず，応答メッセージの	*/
                /* 先頭に登録する	*/
  W	start;     /* デバイスの先頭からの位置（物理ブロック単位）	*/
  W	size;      /* 書き込み量（物理ブロック単位）	*/
} DDEV_REA_REQ;

typedef struct {
  W dd;
  B *buf;
  W start;
  W length;
} DDEV_PRD_REQ;

typedef struct {
  W	dd;	/* デバイスドライバマネージャ管理情報	*/
  W	start;		/* デバイスの先頭からの位置（物理ブロック単位）	*/
  W	size;		/* 書き込み量（物理ブロック単位）	*/
  UB	dt[1024];	/* 書き込むデータ(最大)	*/
} DDEV_RLY_REQ;

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
  W errcd;
  W errinfo;   /* エラー詳細情報	*/
  W split;     /* 分割情報	*/
  W	a_size; /* 実際に読み込んだブロック数	*/
  UB dt[1024];     /* 読み込んだデータ	*/
} DDEV_REA_RES;


/* デバイスからの読み込み（ＬｏｗＬｉｂへの応答）
 */
typedef struct {
  W	errcd;
  W	errinfo;   /* エラー詳細情報	*/
  W	split;     /* 分割情報	*/
  W	a_size;     /* 実際に読み込んだブロック数	*/
  UB	dt[1024];     /* 読み込んだデータ	*/
} DEV_REA_RES;

/* デバイスへの書き込み(wri_dev) 
 *
 *   ・ＬｏｗＬｉｂは、デバイスドライバマネージャの用意しているメッセージバッフ
 *     ァのサイズ以上の要求を発行してはいけない。（不可能）（ＬｏｗＬｉｂ内で分
 *     割して発行する）
 */
typedef struct {
  ID	pid;			/* システムコールを発行したプロセス	*/
  W	dd;			/* デバイスディスクリプタ	*/
  W	start;			/* デバイスの先頭からの位置（物理ブロック単位）	*/
  W	size;			/* 書き込み量（物理ブロック単位）	*/
  UB	dt[1024];		/* 書き込むデータ	*/
} DEV_WRI_REQ;


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
  W	start;		/* デバイスの先頭からの位置（物理ブロック単位）	*/
  W	size;		/* 書き込み量（物理ブロック単位）	*/
  UB	dt[1024];	/* 書き込むデータ(最大)	*/
} DDEV_WRI_REQ;


/* デバイスへの書き込み（デバイスドライバマネージャへの応答）
 */
typedef struct {
  W dd;		/* デバイスドライバマネージャ管理情報	*/
  W errcd;
  W errinfo;	/* エラー詳細情報	*/
  W	a_size;	/* 実際に書き込んだブロック数	*/
} DDEV_WRI_RES;


/* デバイスへの書き込み（ＬｏｗＬｉｂへの応答）
 */
typedef struct {
  W errcd;
  W errinfo;	/* エラー詳細情報	*/
  W	a_size;	/* 実際に書き込んだブロック数	*/
}DEV_WRI_RES;



/* デバイス制御(ctl_dev)
 */
typedef struct {
  ID pid;		/* システムコールを発行したプロセス	*/
  W dd;			/* デバイスディスクリプタ	*/
  UW cmd;		/* 制御コード（デバイスドライバ依存）	*/
  W len;		/* パラメタのバイトサイズ	*/
  UB param[0];		/* 	*/
} DEV_CTL_REQ;

/* デバイス制御（デバイスドライバへ送るメッセージ）
 */
typedef struct {
  W dd;			/* デバイスドライバマネージャ管理情報	*/
  			/* デバイスドライバはこの値を使用せず， */
  			/* 応答メッセージの	*/
			/* 先頭に登録する	*/
  UW cmd;		/* 制御コード（デバイスドライバ依存）	*/
  W len;		/* パラメタのバイトサイズ	*/
  UB param[0];		/* 	*/
} DDEV_CTL_REQ;

/* デバイス制御（デバイスドライバマネージャへの応答）
 */
typedef struct {
  W dd;			/* デバイスドライバマネージャ管理情報	*/
  W errcd;
  W errinfo;		/* エラー詳細情報	*/
  W res[0];
} DDEV_CTL_RES;

/* デバイス制御（ＬｏｗＬｉｂへの応答）
 */
typedef struct {
  W errcd;
  W errinfo;		/* エラー詳細情報	*/
  W res[0];
} DEV_CTL_RES;



/* デバイスのアクセスモード変更(chg_dmd)
 */
typedef struct {
  ID pid;		/* システムコールを発行したプロセス	*/
  TC dev[8];		/* デバイス名（ユニット名）	*/
  UW mode;		/* 新しいアクセスモード	*/
} DEV_CHG_REQ;

/* デバイスのアクセスモード変更（ＬｏｗＬｉｂへの応答）
 */
typedef struct {
  W errcd;
} DDEV_CHG_RES;



/* デバイスの管理情報の取り出し(dev_sts)
 */
typedef struct {
  ID pid;		/* システムコールを発行したプロセス	*/
  TC dev[8];		/* デバイス名（ユニット名）	*/
} DDEV_STS_RES;

/* デバイスの管理情報の取り出し（ＬｏｗＬｉｂへの応答）
 */
typedef struct {
  W errcd;
  DEV_STAT stat;
} DEV_STS_RES;



/* 論理デバイス名の取り出し(get_dev)
 */
typedef struct {
  ID pid;		/* システムコールを発行したプロセス	*/
  W num;		/* デバイス番号	*/
} DEV_GET_REQ;

/* 論理デバイス名の取り出し（ＬｏｗＬｉｂへの応答）
 * ・正常終了した場合にはerrcdにサブユニット数が返却される
 */
typedef struct {
  W errcd;
  TC dev[9];		/* ユニット名	*/
} DEV_GET_RES;


/* 登録済みデバイスの取り出し(lst_dev)
 */
typedef struct {
  ID pid;		/* システムコールを発行したプロセス	*/
  UW ndev;		/* 返却するデバイス情報数	*/
} DEV_LST_REQ;

/* 登録済みデバイスの取り出し（ＬｏｗＬｉｂへの応答）
 * ・正常終了した場合にはerrcdにデバイス情報数が返却される
 */
typedef struct {
  W errcd;
  DEV_INFO info[0];
} DEV_LST_RES;



/* イベント通知先の変更（今の所実装の予定なし）
 *  ・デバイスドライバはデフォルトでデバイスドライバマネージャにイベントを送り
 *    ます。しかし、デバイスが再登録されるとイベント通知先変更のメッセージが送
 *    られますので、それ以降は、このメッセージで指定されたメッセージバッファに
 *    イベントを送ってください。（ここで指定されるメッセージバッファは、再登録
 *    したデバイスドライバのメッセージバッファです。従って、再登録するデバイス
 *    ドライバでは、デバイスイベントを処理できなければいけません）
 *
 */
typedef struct {
  W mbfid;		/* イベント通知先のメッセージバッファ	*/
} DEV_CHG_EVT_REQ;


/* イベントの通知（デバイスドライバマネージャへの通知）
 * ・デバイス番号は、デバイス登録時に返却された値を指定してください。
 */
typedef struct {
  W kind;		/* デバイスイベント種別（詳細はデバイスドライバに依存）	*/
  W devno;		/* デバイス番号	*/
} DDEV_EVT_REQ;


/* イベントの通知（ユーザープロセスへの通知）
 *  ・デバイス番号で指定されたデバイスをオープンしているプロセスがいなければ、
 *    発生したイベントは無視されてしまいます。
 *  ・デバイスが複数のプロセス（タスク）からオープンされている場合は、全てのプ
 *    ロセス（タスク）にイベントが通知されます。
 *  ・キーボードイベント、ポインティングデバイスイベント、ＦＤ挿入イベント等を
 *    処理するためには、デーモンプロセスが動作している必要があります。
 */
typedef struct {
  W	kind;		/* デバイスイベント種別（詳細はデバイスドライバに依存）	*/
  W	devno;		/* デバイス番号	*/
  W	time;		/* イベント発生時刻	*/
} DEV_EVT_REQ;



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
      DDEV_CTL_REQ	ctl_req;
      DDEV_PRD_REQ	prd_req;
      DDEV_RLY_REQ	rly_req;
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
      DDEV_CTL_RES	ctl_res;
    } body;
} DDEV_RES;



#define DEVID(id)	(id >> 16)
#define UNIT(id)	((id >> 8) & 0xff)
#define SUBUNIT(id)	(id & 0xff)
#define MAKEDEVID(dev,unit,subunit)	((dev << 16) | (unit << 8) | subunit)

#endif /* __KERNLIB_DEVICE_H__ */
