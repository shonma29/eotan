/* locore.s --- アセンブラで書かなければならない関数および
 *              変数群の定義.
 *
 *
 */


.text

.globl		get_cr2, get_cr3, load_task_register
.globl		resume

/*************************************************************************
 * get_cr2 --- コントロールレジスタ 2 の内容を返す。
 *
 * 引数：	なし
 *
 * 返値：	コントロールレジスタ 2 の内容。
 *
 * 処理：	コントロールレジスタ 2 の内容を返す。
 *
 */
get_cr2:
		mov	%cr2, %eax
		ret

/*************************************************************************
 * get_cr3 --- コントロールレジスタ 3 の内容を返す。
 *
 * 引数：	なし
 *
 * 返値：	コントロールレジスタ 3 の内容。
 *
 * 処理：	コントロールレジスタ 3 の内容を返す。
 *
 */
get_cr3:
		mov	%cr3, %eax
		ret



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
#if 0
string1:
		.ascii "load_task_register: arg = 0x%x\n"
#endif
		.align 2
load_task_register:
		pushl	%ebp
		movl	%esp,%ebp
#if 0
		pushl	8(%ebp)
		pushl	$string1
		call	printf
#endif
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
#if 0
LC0:
		.ascii "resume: arg = 0x%x\n"
#endif
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
