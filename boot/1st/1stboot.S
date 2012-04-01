! <1stboot.s> --- ファーストブート.一番最初にロードされる.
!
! $Date: 1999/12/23 17:54:17 $
! $Log: 1stboot.S,v $
! Revision 1.2  1999/12/23 17:54:17  kishida0
! loader $B$N0LCV$r(B 1st loader $B$K$&$a$`MM$K%(%s%H%j$rDI2C$7$?(B
!
! Revision 1.1  1999/03/15 02:10:52  monaka
! renamed some asm files included cpp macros. *.s was obsolute.
!
! Revision 1.7  1998/12/09 00:01:27  monaka
! Minor changed. (This is not bug fix)
!
! Revision 1.6  1998/11/20 08:02:16  monaka
! *** empty log message ***
!
! Revision 1.5  1998/05/23 15:29:32  night
! 藤永さんマシン用の変更。
! (ただし、この変更では、藤永さんマシンではうまく動かなかった)
!
! Revision 1.4  1996/07/06 13:10:58  night
! 2nd boot に処理を移行する前に 1 秒間の delay を置いた。
!
# Revision 1.3  1996/05/11  14:36:41  night
# 1stboot 終了時のメッセージを削除
#
# Revision 1.2  1996/05/11  14:22:19  night
# 2nd boot の情報が入ったセクタを変更(1 -> 2)
#
# Revision 1.1  1996/05/05  14:11:59  night
# 1st boot プログラム。
# BIOS により FD から読み込まれ、2nd boot を読み込む。
#
!
! Discription
!	本プログラムは、BTRON/386 のカーネルロードプログラムである。
!
!	このプログラムは、ＦＤからブートすることを前提としている。
!	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
info_sector	=	2		! 512 bytes/sector
info_cylinder	=	0

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
		call	delay
!		call	kill_motor
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

!
! stop motor.
!
kill_motor:
		push	dx
		mov	dx, #0x3f2
		xor	al, al
		outb
		pop	dx
		ret	
read2ndboot:	
		! BIOS が使用する FD パラメータの初期化
		mov	cx,#0
		sub	si,si
		mov	fs,cx
		mov	bx,#0x78		! fs:dl がパラメータテーブルのアドレスとなる
		push	ds
		seg fs
		lds	si,(bx)			! ds:si がテーブルに入れる情報のコピー元

		mov	cl,#6			! copy 12 bytes
		cld
		push	di

		rep
		movsw

		pop	di
		pop	ds

		movb	4(di),*36		! patch sector count
	
		seg fs
		mov	(bx),di
		seg fs
		mov	2(bx),es
	
		! 2nd BOOT の情報をロード
		mov	bx, #boot2_message
		call	print
		mov	seg_addr, #seg_size_info
		mov	off_addr, #off_size_info
		movb	sector, *info_sector
		movb	cylinder, *info_cylinder
		call	readdisk

		! 読み取った情報を使ってFDから2nd BOOT本体を読み取る
		movb	sector, *3
		movb	cylinder, *0
		mov	seg_addr, #seg_loadpoint
		mov	off_addr, #off_loadpoint
		mov	ax, #seg_size_info
		mov	es, ax
		seg es
		mov	ax, off_size_info

load_loop:
!		call	print_dot
		cmp	ax, #0
		jz	loop_done

		cmpb	sector, #19		! セクタ番号が 19 の場合
		jne	l2

		call	print_dot
		mov	sector, #1		! セクタ番号を1に戻す
		incb	cylinder

l2:
		call	readdisk
		add	seg_addr, #0x0020	! 512 bytes/sector
		dec	ax
		incb	sector
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

print_dot:
		pusha
		mov	ax, #0xe2e		! '.'
		int	0x10
		popa
		ret
		
!
!
!

!
!	fatal --- 恒久的な失敗! 無限ループに入る
!
fatal:
		mov	bx, #fatal_message
		call	print
fatal_loop:
		jmp	fatal_loop

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

!		xor	ah, ah			! reset FDC
!		xor	dl, dl			! ドライブ番号
!		int	0x13
retry_reading:
		xor	dx, dx
		movb	al, cylinder
		shr	al, #1
		movb	ch, al			! シリンダ番号のセット

		movb	al, cylinder
		and	al, #0x01
		movb	dh, al			! ヘッド番号のセット

		movb	cl, sector		! セクタ番号のセット

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
bad_rt:
		! reset FDC
		xor	ah, ah
		xor	dl, dl
		int	0x13

		! Print error message
		mov	bx, #retry_message
		call	print

		jmp	retry_reading


seg_addr:	.word	0	! バッファのセグメント
off_addr:	.word	0	! バッファのオフセット
cylinder:	.byte	0	! シリンダ番号(この中には、ヘッド番号も含まれる)
sector:		.byte	0	! セクタ番号
fatal_message:	
		.byte	35
		.ascii	"can't read 2ndboot program from FD."


!
! 	データ群 (主に文字列。。。)
!
message: 	.byte	28
		.ascii	"1st boot for btron/386(PC)"
		.byte   13, 10

message_dot:	.byte	1
		.ascii	"."

retry_message:	.byte	1
		.ascii	"e"

boot2_message:
		.byte   21
		.ascii	"loading 2nd boot..."
		.byte   13, 10

done_message:
		.byte   20
		.ascii	"loaded 2nd boot..."
		.byte	13, 10


seg_addr:	.word	0	! バッファのセグメント
off_addr:	.word	0	! バッファのオフセット
cylinder:	.byte	0	! シリンダ番号
sector:		.byte	0	! セクタ番号(この中には、ヘッド番号も含まれる)
! read_count:	.word	0	! 読み込むブロック数(512 単位)
fatal_message:
		.byte	34
		.ascii	"can't read 2ndboot program from FD."
		.byte	0

.org    498
loader_offset:  .word 0xffff

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
	
