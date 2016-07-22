#ifndef __ARCH_8042_H__
#define __ARCH_8042_H__ 1
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
#include <mpu/io.h>

/* ports */
#define KBC_PORT_CMD 0x0064
#define KBC_PORT_DATA 0x0060

/* status */
#define KBC_STATUS_F1 0x08
#define KBC_STATUS_IBF 0x02
#define KBC_STATUS_OBF 0x01

/* commands */
#define KBC_READ_CMD 0x20
#define KBC_READ_RAM 0x21
#define KBC_WRITE_CMD 0x60
#define KBC_WRITE_RAM 0x61
#define KBC_DISABLE_AUX 0xa7
#define KBC_ENABLE_AUX 0xa8
#define KBC_TEST_AUX 0xa9
#define KBC_TEST_SELF 0xaa
#define KBC_TEST_KBD 0xab
#define KBC_DISABLE_KBD 0xad
#define KBC_ENABLE_KBD 0xae
#define KBC_READ_IN_PORT 0xc0
#define KBC_POLL_IN_PORT_LOW 0xc1
#define KBC_POLL_IN_PORT_HIGH 0xc2
#define KBC_READ_OUT_PORT 0xd0
#define KBC_WRITE_OUT_PORT 0xd1
#define KBC_WRITE_KBD_BUF 0xd2
#define KBC_WRITE_AUX_BUF 0xd3
#define KBC_WRITE_AUX 0xd4
#define KBC_READ_TEST_PIN 0xe0
#define KBC_PULSE 0xf0

/* KBC command parameter */
#define KBC_CMD_TYPE 0x40
#define KBC_CMD_DISABLE_AUX 0x20
#define KBC_CMD_DISABLE_KBD 0x10
#define KBC_CMD_INTERRUPT_AUX 0x02
#define KBC_CMD_INTERRUPT_KBD 0x01

static inline void kbc_wait_to_write(void)
{
	while (inb(KBC_PORT_CMD) & (KBC_STATUS_IBF | KBC_STATUS_OBF));
}

static inline void kbc_wait_to_read(void)
{
	while (!(inb(KBC_PORT_CMD) & KBC_STATUS_OBF));
}

#endif
