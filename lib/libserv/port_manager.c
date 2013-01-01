/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2003, Tomohide Naniwa

*/
/* @(#) $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/ITRON/kernlib/port_manager.c,v 1.1 1999/04/18 17:48:33 monaka Exp $ */

/*
 * $Log: port_manager.c,v $
 * Revision 1.1  1999/04/18 17:48:33  monaka
 * Port-manager and libkernel.a is moved to ITRON. I guess it is reasonable. At least they should not be in BTRON/.
 *
 * Revision 1.5  1999/03/30 13:38:52  monaka
 * Minor fixes.
 *
 * Revision 1.4  1999/03/15 00:23:48  monaka
 * Some casts for rcv_mbf added.
 *
 * Revision 1.3  1997/10/11 16:21:11  night
 * ポート名のコピーを修正。
 *
 * Revision 1.2  1997/09/23 13:52:20  night
 * デバッグ文の追加。
 * find_port() の第一引数 (name) の型を変更。
 *
 * Revision 1.1  1996/07/22  23:52:05  night
 * 最初の登録
 *
 * Revision 1.4  1995/12/05 15:10:22  night
 * unregist_port () の追加。
 *
 * Revision 1.3  1995/09/21  15:51:42  night
 * ソースファイルの先頭に Copyright notice 情報を追加。
 *
 * Revision 1.2  1995/06/28  14:14:25  night
 * print 文を #ifdef DEBUG 〜 #endif で囲った。
 *
 * Revision 1.1  1995/02/26  14:18:12  night
 * 最初の登録
 *
 *
 */
/*
 * ポートマネージャとの通信を簡単に行うためのライブラリ関数。
 *
 * regist_port (PORT_NAME name, ID port);   メッセージバッファ ID の登録
 * unregist_port (PORT_NAME name);	    メッセージバッファ ID の抹消
 * find_port (PORT_NAME name, ID &port);    メッセージバッファ ID の検索
 * alloc_port (void);                       ポートの生成
 */
#include <string.h>
#include <services.h>
#include <core.h>
#include <itron/rendezvous.h>
#include <port-manager/port-manager.h>

/*
 * ポートマネージャにメッセージバッファ情報を登録する。
 * この関数の中で一時的に受信用のメッセージポートを作成する。
 *
 */
PORT_MANAGER_ERROR
regist_port (port_name *name, ID port)
{
  union {
    struct recv_port_message_t	recv_msg;
    struct port_manager_msg_t  	send_msg;
  } buf;
  ER_UINT		     	rsize;

  /*
   * ポートマネージャへ送る要求メッセージを作成。
   */
  buf.send_msg.hdr.type  = REGIST_PORT;
  buf.send_msg.hdr.size  = sizeof (buf.send_msg);
  buf.send_msg.hdr.rport = 0;
  strcpy ((char*)&(buf.send_msg.body.regist.name), (char*)name);
  buf.send_msg.body.regist.port = port;

#ifdef DEBUG
  dbg_printf ("regist_port: name = <%s>\n", &(buf.send_msg.body.regist.name));
#endif /* DEBUG */

  /*
   * ポートマネージャに対して登録要求メッセージを送信する。
   */
  rsize = cal_por (PORT_NAME, 0xffffffff, &buf, sizeof (buf.send_msg));
  if (rsize < 0)
    {
      return (E_PORT_SYSTEM);
    }

#ifdef DEBUG
  dbg_printf ("port_manager:cal_por: ok.\n");
  dbg_printf ("port_manager: errno = %d\n", buf.recv_msg.error);
#endif /* DEBUG */
  return (buf.recv_msg.error);
}

PORT_MANAGER_ERROR
unregist_port (port_name *name)
{
  union {
    struct recv_port_message_t	recv_msg;
    struct port_manager_msg_t  	send_msg;
  } buf;
  ER_UINT		     	rsize;

  /*
   * ポートマネージャへ送る要求メッセージを作成。
   */
  buf.send_msg.hdr.type  = UNREGIST_PORT;
  buf.send_msg.hdr.size  = sizeof (buf.send_msg);
  buf.send_msg.hdr.rport = 0;
  strcpy ((char*)&(buf.send_msg.body.regist.name), (char*)name);

#ifdef DEBUG
  dbg_printf ("regist_port: name = <%s>\n", &(buf.send_msg.body.regist.name));
#endif /* DEBUG */

  /*
   * ポートマネージャに対して登録要求メッセージを送信する。
   */
  rsize = cal_por (PORT_NAME, 0xffffffff, &buf, sizeof (buf.send_msg));
  if (rsize < 0)
    {
      return (E_PORT_SYSTEM);
    }

#ifdef DEBUG
  dbg_printf ("port_manager:cal_por: ok.\n");
  dbg_printf ("port_manager: errno = %d\n", buf.recv_msg.error);
#endif /* DEBUG */
  return (buf.recv_msg.error);
}


/*
 * ポートマネージャからメッセージバッファ情報を検索する。
 * この関数の中で一時的に受信用のメッセージポートを作成する。
 *
 */
PORT_MANAGER_ERROR
find_port (B *name, ID *rport)
{
  union {
    struct recv_port_message_t	recv_msg;
    struct port_manager_msg_t  	send_msg;
  } buf;
  ER_UINT		     	rsize;

#ifdef DEBUG
  dbg_printf ("find_port, name = <%s>\n", name);		/* */
#endif

#ifdef DEBUG
  dbg_printf ("strcpy (0x%x, %s)\n",
	      &(buf.send_msg.body.find.name), name);
#endif /* DEBUG */
  /*
   * ポートマネージャへ送る要求メッセージを作成。
   */
  buf.send_msg.hdr.type  = FIND_PORT;
  buf.send_msg.hdr.size  = sizeof (buf.send_msg);
  buf.send_msg.hdr.rport = 0;
  strcpy (buf.send_msg.body.find.name, name);

#ifdef DEBUG
  dbg_printf ("find_port: name = <%s>\n", &(buf.send_msg.body.find.name));
#endif /* DEBUG */
  /*
   * ポートマネージャに対して検索要求メッセージを送信する。
   */
  rsize = cal_por (PORT_NAME, 0xffffffff, &buf, sizeof (buf.send_msg));
  if (rsize < 0)
    {
      return (E_PORT_SYSTEM);
    }

#ifdef DEBUG
  dbg_printf ("port_manager:cal_por: ok.\n");
#endif /* DEBUG */

  /*
   * ポートマネージャが検索したポート番号を返す。
   */
  *rport = buf.recv_msg.port;

#ifdef DEBUG
  dbg_printf ("port_manager: errno = %d\n", buf.recv_msg.error);
#endif /* DEBUG */
  return (buf.recv_msg.error);
}

/*
 * メッセージバッファを生成する。
 * メッセージバッファ ID は、自動的に空いているものを使用する。
 *
 */
ID
alloc_port (W size, W max_entry)
{
  ID		msg_port;
  T_CMBF	create_argument;

  /*
   * 要求受けつけのためのメッセージバッファを作成する。
   * メッセージバッファの ID は特に決まっていない。空いているメッセー
   * ジバッファを適当に選ぶ。
   */
  create_argument.bufsz  = size;
  create_argument.maxmsz = size * max_entry;
  create_argument.mbfatr = TA_TFIFO;
  for (msg_port = MIN_USERMBFID;
       msg_port <= MAX_USERMBFID;
       msg_port++)
    {
      if (cre_mbf (msg_port, &create_argument) == E_OK)
	{
	  /*
	   * メッセージバッファの取得に成功した。
	   */
	  return (msg_port);
	}
    }

  /*
   * メッセージバッファが取得できなかった。
   */
#ifdef DEBUG
  dbg_printf ("posix.process server: cannot allocate messege buffer\n");
#endif /* DEBUG */
  return (0);
}
