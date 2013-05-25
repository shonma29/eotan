/* locore.s --- アセンブラで書かなければならない関数および
 *              変数群の定義.
 *
 *
 */


.text

.globl		load_task_register
.globl		resume

/*************************************************************************
 * load_task_register --- タスクセグメントレジスタのロード
 *
 * 引数：	ローディングするタスクセグメントレジスタへのセレクタ
 *
 * 返値：	なし
 *
 * 処理：	ltr を実行する。
 *
 */
		.align 2
load_task_register:
		pushl	%ebp
		movl	%esp,%ebp
		movl	8(%ebp),%eax
		ltr	%ax
		leave
		ret


/*************************************************************************
 * resume --- 引数で指定したプロセスに切り換える。
 *
 * 引数：	切り換えるプロセスの TSS をさすセレクタ
 *
 * 返値：	なし
 *
 * 処理：	引数で渡されたセレクタに TSS jump する。
 *		その結果プロセスを切り換える。
 *
 */
		.align 2
offset:		.long	0x0000
selector:	.word	0x0000

resume:
		cli
		pushl	%ebp
		movl	%esp, %ebp
		movl	8(%ebp), %eax		/* original is 8(%ebp) */
		movw	%ax, (selector)
		movl	%cr3, %eax	/* TLB キャッシュをフラッシュする */
		movl	%eax, %cr3
		lea	offset, %eax		/* */

/*
 *		プロセス切り換え
 *		ljmp	far (%ax)
 */
		.byte	0xff
		.byte	0x28
		clts   /* clear TS bit */
		leave
		sti
		ret
