#ifndef __PCAT_8259A_H__
#define __PCAT_8259A_H__ 1
/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/

enum PicPort {
	pic_master1 = 0x0020,
	pic_master2 = 0x0021,
	pic_slave1 = 0x00a0,
	pic_slave2 = 0x00a1
};

#define PIC_INT_NO_MASK 0x07
#define PIC_VECTOR_MASK 0xf8

#define PIC_OCW2_ROTATE 0x80
#define PIC_OCW2_SPECIFIC_EOI 0x60
#define PIC_OCW2_NON_SPECIFIC_EOI 0x20
#define PIC_OCW2_AUTO_EOI 0x00
#define PIC_OCW2_SET_PRIORITY 0xc0

#define PIC_OCW3_BASE 0x08
#define PIC_OCW3_SPECIAL_MASK_SET 0x60
#define PIC_OCW3_SPECIAL_MASK_RESET 0x40
#define PIC_OCW3_POLL 0x04
#define PIC_OCW_READ_ISR 0x03
#define PIC_OCW_READ_IRR 0x02

#define PIC_ICW1_BASE 0x10
#define PIC_ICW1_LTIM_LEVEL 0x08
#define PIC_ICW1_ADI_4 0x04
#define PIC_ICW1_SNGL 0x02
#define PIC_ICW1_NEED_IC4 0x01

#define PIC_ICW4_SPECIAL_NEST 0x10
#define PIC_ICW4_MASTER_BUFFER 0x0c
#define PIC_ICW4_SLAVE_BUFFER 0x08
#define PIC_ICW4_AUTO_EOI 0x02
#define PIC_ICW4_8086 0x01

/* vector */
#define PIC_MASTER_VECTOR 0x20
#define PIC_SLAVE_VECTOR 0x28
#define PIC_CASCADE_NO (2)

/* legacy devices */
enum PicIr {
	ir_pit = 0,
	ir_keyboard = 1,
	ir_cascade = 2,
	ir_com2 = 3,
	ir_com1 = 4,
	ir_fdc = 6,
	ir_parallel = 7,
	ir_rtc = 8,
	ir_mouse = 12,
	ir_coprocessor = 13,
	ir_ide_primary = 14,
	ir_ide_secondary = 15
};

#define PIC_IR_BIT(ir) (1 << (ir))
#define PIC_IR_VECTOR(ir) (PIC_MASTER_VECTOR + (ir))

#endif
