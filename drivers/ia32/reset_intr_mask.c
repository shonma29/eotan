/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
#include "../../include/itron/types.h"
#include "../../include/mpu/io.h"
#include "../../kernel/sync.h"
#include "../../kernel/arch/arch.h"

/*************************************************************************
 * reset_intr_mask 
 *
 * 引数：	intn	割り込み番号
 *
 * 返値：	なし
 *
 * 処理：	割り込みマスクをリセットする。
 *
 */
void reset_intr_mask(W intn)
{
    enter_critical();
    if (intn < 8) {
	outb(MASTER_8259A_DATA, inb(MASTER_8259A_DATA) & ~(1 << intn));
    } else {
	outb(SLAVE_8259A_DATA, inb(SLAVE_8259A_DATA) & ~(1 << (intn - 8)));
    }
    leave_critical();
}
