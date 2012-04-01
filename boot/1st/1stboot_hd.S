!
!
! <1stboot.s> --- ファーストブート.一番最初にロードされる.
!
! $Header: /usr/local/src/master/B-Free/Program/btron-pc/boot/1st/1stboot_hd.S,v 1.2 1999/12/23 17:54:18 kishida0 Exp $
!
! $Log: 1stboot_hd.S,v $
! Revision 1.2  1999/12/23 17:54:18  kishida0
! loader $B$N0LCV$r(B 1st loader $B$K$&$a$`MM$K%(%s%H%j$rDI2C$7$?(B
!
! Revision 1.1  1999/03/15 02:10:54  monaka
! renamed some asm files included cpp macros. *.s was obsolute.
!
! Revision 1.5  1998/06/11 14:37:24  night
! 10ビットあるシリンダ番号の上位 2 ビットを使えるように変更した。
! (今までは 8 ビットしか有効でなかった)
! この変更の結果、シリンダ数が 256 以上の HD に対してもブートでき
! るようになった。
!
! なお、ここで書いているシリンダ数は BIOS にて変換された後の値。
!
! Revision 1.4  1998/02/23 15:34:23  night
! Dynabook の HD に合わせて設定を変更した。
!
! Revision 1.3  1997/08/31 14:23:22  night
! テスト用環境の変更に伴い、HD の設定情報を変更した。
!
! Revision 1.2  1997/06/29 13:12:16  night
! HD のジオメトリ情報および 2nd boot の入っているパーティション情報の変更。
!
# Revision 1.1  1996/08/11  15:09:47  night
# 最初の登録
#
!
! Discription
!	本プログラムは、BTRON/386 のカーネルロードプログラムである。
!
!	このプログラムは、ＨＤ (IDE) からブートすることを前提としている。
!	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
!
!	このプログラムがＩＰＬからロードされた時、レジスタ類の状態は次の
!	ようになっている。
!
!	AX,BX,CX,DX			不定
!
!	ロードアドレス	セグメント	0x07C0
!			オフセット	0x0000
!	ロードサイズ			512 バイト
!
! 処理内容
!
!	自分自身をコピー (0x07C0 -> 0x70000)
!	2nd BOOT 情報をロード
!	2nd BOOT 本体をロード (GDT, IDT, Page table を含む)
!	2nd BOOT の先頭にジャンプ
!

seg_2ndboot = 		0x0800
off_2ndboot = 		0x0000

seg_2ndboot_info =	0x7800
off_2ndboot_info =	0x0000
seg_size_info =		0x7800
off_size_info =		0x0000
seg_loadpoint =		0x0100		! loadpoint = 0x1000
off_loadpoint =		0x0000

.text
begtext:

.text

entry		first_boot

		.org	0x0000
first_boot:
		mov	ax,#0x07c0
		mov	ds,ax
		mov	ax,#0x7000
		mov	es,ax
		mov	cx,#512
		sub	si,si
		sub	di,di
		cld
		rep
		movsw
		jmpi	#restart, #0x7000

restart:
		mov	ax, #0x7000
		mov	es, ax
		mov	ss, ax
		mov	ds, ax
		mov	sp, #2048

		mov	bx, #message
		call	print

		call	read2ndboot			! 2nd boot の読み込み

		mov	bx, #done_message
		call	print

		mov	ax, #seg_2ndboot		! 2nd boot の呼び出し
		push	ax
		mov	ax, #off_2ndboot
		push	ax
		reti

delay:
		mov	ax,#0x1000
		mov	bx,#0xffff
delay1:
		dec	bx
		jnz	delay1
		mov	bx,#1000
		dec	ax
		jnz	delay1
		ret
	
read2ndboot:
		! 2nd BOOT の情報をロード
		mov	bx, #boot2_message
		call	print

		mov	seg_addr, #seg_size_info
		mov	off_addr, #off_size_info

		movb	ah, #0x0d			! HD のリセット
		movb	dl, #0x80
		int	0x13

		movb	bl, s_sector			! パーティションの開始位置を指定
		movb	sector, bl			!

		mov	bx, s_cylinder			!
		mov	cylinder, bx			!

		movb	bl, s_head			!
		movb	head, bl			!
		inc	sector				! 1st boot の分 (1 sector 分を
		call	readdisk			! インクリメント)
		inc	sector

		! 読み取った情報を使ってFDから2nd BOOT本体を読み取る
		mov	seg_addr, #seg_loadpoint
		mov	off_addr, #off_loadpoint
		mov	ax, #seg_size_info
		mov	es, ax
		seg es
		mov	ax, off_size_info

load_loop:
		call	print_dot
		cmp	ax, #0
		jz	loop_done

		movb	bl, n_sector
		cmpb	sector, bl		! セクタ番号が上限を越えた
		jne	l2

		movb	sector, #1		! セクタ番号を1に戻す
		inc	head			! 次のヘッドへ移動

		movb	bl, n_head		! ヘッド番号が上限を越えた
		cmp	head, bl		!
		jne	l3			
		inc	cylinder		! 次のシリンダへ移動
		movb	head, #0		! ヘッドを 0 に変更

l3:
l2:
		call	readdisk
		add	seg_addr, #0x0020	! 512 bytes/sector
		dec	ax
		inc	sector
		jmp	load_loop

loop_done:
!		mov	bx, #done_message
!		call	print
		ret
				

!
!	下受け関数群
!
!
! print --- 文字列をコンソールに表示する。
!
! 引数：
!	bx	表示する文字列のアドレス
!		文字列は、<長さ>:1バイト
!			  <文字列>:最大255バイト
!		となっている。
print:
		push	ax
		push	bx
		push	cx
		push	dx

		push	bx
		mov	ah,#0x03		! get cursor pos
		xor	bh,bh
		int	0x10
		pop	bx

		xor	cx, cx
		movb	cl, (bx)
		inc	bx
		mov	bp, bx
		mov	bx, #0x0007
		mov	ax, #0x1301
		int	0x10
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret

!
!
!
print_dot:
		push	ax
		mov	ax, #0xe2e		! '.'
		int	0x10
		pop	ax
		ret
		

!
!	fatal --- 恒久的な失敗! 無限ループに入る
!
fatal:
		j	fatal

!
!	disk io routines
!
! 	ディスクから指定したセクタを読み込む。
!	BIOS ルーチン 0x13 を使用する。
!
readdisk:
		push	ax
		push	bx
		push	cx
		push	dx

		movb	dl, drive		! ドライブ番号

		mov	ax, cylinder
		mov	ch, al			! シリンダ番号のセット(下位 8 ビット)

		movb	al, head
		movb	dh, al			! ヘッド番号のセット

		movb	cl, sector		! セクタ番号のセット
		shl	ah, 6			! セクタ番号の上位 2 ビットにシリンダ番号の
		add	cl, ah			! 上位 2 ビットをもってくる。

		mov	ax, seg_addr		! 読み込む場所のセグメント値
		mov	es, ax
		mov	bx, off_addr		! 読み込む場所のオフセット値
		movb	al, #0x01
		movb	ah, #0x02
		int	0x13			! BIOS の呼び出し
		jc	fatal

		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret


! テンポラリ変数群
seg_addr:	.word	0	! バッファのセグメント
off_addr:	.word	0	! バッファのオフセット
cylinder:	.word	0	! シリンダ番号
sector:		.byte	0	! セクタ番号
head:		.byte	0	! ヘッダ番号
fatal_message:	
		.byte	35
		.ascii	"can't read 2ndboot program from FD."


!
! 	データ群 (主に文字列。。。)
!
message: 	.byte	28
		.ascii	"1st boot for btron/386(PC)"
		.byte   13, 10

!
!	read2ndboot --- 2nd BOOT プログラムのロード
!
read2ndboot:
		ret

boot2_message:
		.byte   21
		.ascii	"loading 2nd boot..."
		.byte   13, 10

done_message:
		.byte   25
		.ascii	"loading 2nd boot...done"
		.byte	13, 10



fatal_message:
		.byte	34
		.ascii	"can't read 2ndboot program from FD."
		.byte	0

.org    498
loader_offset:	.word   0xffff

.org    500

.org	501
drive:		.byte 0x80	!  0x0 = FD
				! 0x80 = HD

! パーティションの開始位置
.org	502
! HD の場合 (パーティション 0)
s_cylinder:	.word 1
s_head:		.byte 2
s_sector:	.byte 3

.org	506
! HD の GEOMETORY 情報
! 例：Cylinder 255, Head 10, Sector 55
n_cylinder:	.word 4
n_head:		.byte 5		! old value is 16.
n_sector:	.byte 6		! セクタだけは、最大数 + 1 にする。

.org	510
boot_mark:
		.word 0xAA55

.text
endtext:

!.data
!enddata:

!.bss
!endbss:

		end	
	
