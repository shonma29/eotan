/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/jmp_user.S,v 1.2 1997/08/31 13:13:43 night Exp $ */

/*
 * POSIX 環境 on B-Free 
 */


.globl jmp_user_entry

/*
 * ユーザモードに遷移しつつ、指定したエントリアドレスへジャンプする
 */
jmp_user_entry:
	
	ret