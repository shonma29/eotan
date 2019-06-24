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
#include <core.h>
#include <mpu/io.h>
#include "8042.h"
#include "archfunc.h"
#include "keyboard.h"
#include "psaux.h"
#include "../../lib/libserv/libserv.h"

static ER keyboard_initialize(void);
static ER psaux_initialize(void);
static UB _read(void);
static void _writeCommand(const UB b);
static void _writeData(const UB b);
static BOOL isAck(void);
static BOOL _writeAux(const UB b);


ER kbc_initialize(void)
{
	UB b;
	UB c = KBC_CMD_DISABLE_KBD | KBC_CMD_DISABLE_AUX;

	/* disable devices */
	_writeCommand(KBC_DISABLE_KBD);
	_writeCommand(KBC_DISABLE_AUX);

	while (inb(KBC_PORT_CMD) & KBC_STATUS_OBF);

	/* disable interrupt */
	_writeCommand(KBC_READ_CMD);
	b = _read();
	b &= ~(KBC_CMD_INTERRUPT_AUX
			| KBC_CMD_INTERRUPT_KBD);
	_writeCommand(KBC_WRITE_CMD);
	_writeData(b);

	/* self test */
	_writeCommand(KBC_TEST_SELF);

	if (_read() != KBC_SELF_TEST_OK)
		return E_SYS;

	/* test KBD interface */
	_writeCommand(KBC_TEST_KBD);

	if (!_read()) {
		_writeCommand(KBC_ENABLE_KBD);

		if (keyboard_initialize())
			_writeCommand(KBC_DISABLE_KBD);
		else
			c &= ~(KBC_CMD_DISABLE_KBD);
	}

	/* test AUX interface */
	if (b & KBC_CMD_DISABLE_AUX) {
		_writeCommand(KBC_ENABLE_AUX);
		_writeCommand(KBC_READ_CMD);
		b = _read();

		if (!(b & KBC_CMD_DISABLE_AUX)) {
			_writeCommand(KBC_DISABLE_AUX);
			_writeCommand(KBC_TEST_AUX);

			if (!_read()) {
				_writeCommand(KBC_ENABLE_AUX);

				if (psaux_initialize())
					_writeCommand(KBC_DISABLE_AUX);
				else
					c &= ~(KBC_CMD_DISABLE_AUX);
			}
		}
	}

	/* enable interrupt */
	_writeCommand(KBC_READ_CMD);
	b = _read() | KBC_CMD_TYPE;

	if (!(c & KBC_CMD_DISABLE_KBD))
		b |= KBC_CMD_INTERRUPT_KBD;

	if (!(c & KBC_CMD_DISABLE_AUX))
		b |= KBC_CMD_INTERRUPT_AUX;

	_writeCommand(KBC_WRITE_CMD);
	_writeData(b);

	log_info("kbc: status=%x\n", c);

	return E_OK;
}

static ER keyboard_initialize(void)
{
	/* reset */
	_writeData(KBD_RESET);

	return (isAck() ? E_OK : E_SYS);
}

static ER psaux_initialize(void)
{
	do {
		/* disable */
		if (!_writeAux(AUX_DISABLE))
			break;

		/* reset */
		if (!_writeAux(AUX_RESET))
			break;

		if (_read() != AUX_RESULT_OK)
			break;

		if (_read() != AUX_RESULT_MOUSE)
			break;

		/* enable */
		if (!_writeAux(AUX_SET_SCALING))
			break;

		if (!_writeAux(AUX_ENABLE))
			break;

		return E_OK;
	} while (false);

	return E_SYS;
}

static UB _read(void)
{
	kbc_wait_to_read();

	return inb(KBC_PORT_DATA);
}

static void _writeCommand(const UB b)
{
	kbc_wait_to_write();
	outb(KBC_PORT_CMD, b);
}

static void _writeData(const UB b)
{
	kbc_wait_to_write();
	outb(KBC_PORT_DATA, b);
}

static BOOL isAck(void)
{
	return _read() == KBC_DEVICE_ACK;
}

static BOOL _writeAux(const UB b)
{
	_writeCommand(KBC_WRITE_AUX);
	_writeData(b);

	return isAck();
}
