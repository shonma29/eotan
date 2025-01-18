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
#include <archfunc.h>
#include <nerve/kcall.h>
#include <mpu/io.h>
#include <time.h>
#include <services.h>
#include "ata.h"
#include "pci.h"

// ports
#define PORT_PRIMARY 0x00
#define PORT_SECONDARY 0x01

// devices
#define DEVICE0 0x00
#define DEVICE1 0x01

// I/O port
#define PORT1_DATA 0x1f0
#define PORT1_CONTROL 0x3f6
#define PORT2_DATA 0x170
#define PORT2_CONTROL 0x376

// PCI BAR
#define BAR_PORT_MASK 0xfffffffc

// availablility mask
#define LBA48_COMMAND_SETS_MASK (ATA_COMMAND_SETS_48BIT_ADDRESS \
			| ATA_COMMAND_SETS_FLUSH_CACHE_EXT)

#define MASK_LBA48_OFFSET 0x01fffffffffffe00
#define MASK_LBA48_SIZE 0x01fffe00

#define POLL_WAIT_COUNT (4)

typedef struct {
	uint16_t data;
	uint16_t control;
	uint16_t bus_master;
	uint8_t no_interrupt;
} ata_port_t;

typedef struct {
	ata_type_e type;
	uint8_t port;
	uint8_t device;
	uint16_t signature;
	uint16_t capabilities;
	uint32_t command_sets;
	uint64_t size;
	uint8_t model[41];
} ata_unit_t;

static ata_port_t ports[2];
static ata_unit_t units[4];
#if 0
volatile uint8_t irq_invoked = 0;
static uint8_t atapi_packet[12] = { 0xa8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
#endif
static uint32_t bar[5];
static uint8_t data_buf[2048];

static int _set_bar(const int, const uint32_t);
static uint8_t _read_data(const ata_port_t *, const uint8_t);
static uint8_t _read_control(const ata_port_t *, const uint8_t);
static void _write_data(const ata_port_t *, const uint8_t, const uint8_t);
static void _write_control(const ata_port_t *, const uint8_t, const uint8_t);
static void _read_buf(uint32_t *, const ata_port_t *, const uint8_t,
		const size_t);
#if 0
static uint8_t _read_error(const uint8_t);
#endif
static int _poll(const ata_port_t *);
static void _identify(ata_unit_t *, const int, const int);
static bool _check_unit(const void *);
static bool _check_param(const void *, off_t *, size_t *);
static void _set_address(const ata_port_t *, const int, const off_t,
		const size_t);
static unsigned int _msleep(const unsigned int);


static int _set_bar(const int index, const uint32_t value)
{
	if ((index >= 0)
			&& (index < sizeof(bar) / sizeof(bar[0]))) {
		bar[index] = value;
		return 0;
	} else
		return (-1);
}

static uint8_t _read_data(const ata_port_t *p, const uint8_t reg)
{
	return inb(p->data + reg);
}

static uint8_t _read_control(const ata_port_t *p, const uint8_t reg)
{
	return inb(p->control + reg);
}

static void _write_data(const ata_port_t *p, const uint8_t reg,
		const uint8_t data)
{
	outb(p->data + reg, data);
}

static void _write_control(const ata_port_t *p, const uint8_t reg,
		const uint8_t data)
{
	outb(p->control + reg, data);
}

static void _read_buf(uint32_t *buf, const ata_port_t *p, const uint8_t reg,
		const size_t cnt)
{
	uint16_t address = p->data + reg;
	for (int i = 0; i < cnt; i++)
		buf[i] = inl(address);
}
#if 0
static uint8_t _read_error(const ata_port_t *p)
{
	return _read_data(p, ATA_REGISTER_ERROR);
}
#endif
static int _poll(const ata_port_t *p)
{
	for (int i = 0; i < POLL_WAIT_COUNT; i++)
		_read_control(p, ATA_REGISTER_ALTERNATE_STATUS);

	while (_read_data(p, ATA_REGISTER_STATUS) & ATA_STATUS_BSY);

	return (_read_data(p, ATA_REGISTER_STATUS)
			& (ATA_STATUS_ERRCHK | ATA_STATUS_DFSE)) ?
			(-1) : 0;
}

static void _identify(ata_unit_t *unit, const int port, const int device)
{
	unit->type = 0;
	unit->port = port;
	unit->device = device;

	ata_port_t *p = &(ports[port]);
	_write_data(p, ATA_REGISTER_DEVICE, 0xa0 | (device << 4));
	_msleep(1);
	_write_data(p, ATA_REGISTER_COMMAND, IDENTIFY_DEVICE);
	_msleep(1);

	if (!_read_data(p, ATA_REGISTER_STATUS))
		return;

	for (;;) {
		uint8_t status = _read_data(p, ATA_REGISTER_STATUS);
		if (status & ATA_STATUS_ERRCHK) {
			uint8_t bl = _read_data(p, ATA_REGISTER_LBA_MID);
			uint8_t bh = _read_data(p, ATA_REGISTER_LBA_HIGH);
			if (((bl == 0x14) && (bh == 0xeb))
					|| ((bl == 0x69) && (bh == 0x96)))
				unit->type = ATAPI;
			else
				return;

			_write_data(p, ATA_REGISTER_COMMAND,
					IDENTIFY_PACKET_DEVICE);
			_msleep(1);
			break;
		}

		if (!(status & ATA_STATUS_BSY)
				&& (status & ATA_STATUS_DRQ)) {
			unit->type = ATA;
			break;
		}
	}

	_read_buf((uint32_t *) &data_buf, p, ATA_REGISTER_DATA,
			ATA_SIZE_OF_IDENTIFY / sizeof(uint32_t));
	unit->signature = *((uint16_t *) &(data_buf[ATA_IDENTIFY_GENERAL_CONFIGURATION]));
	unit->capabilities =
			*((uint16_t *) &(data_buf[ATA_IDENTIFY_CAPABILITIES]));
	unit->command_sets =
			*((uint32_t *) &(data_buf[ATA_IDENTIFY_COMMAND_SETS_SUPPORTED]));
	unit->size = (unit->command_sets & ATA_COMMAND_SETS_48BIT_ADDRESS) ?
			*((uint64_t *) &(data_buf[ATA_IDENTIFY_MAX_LBA48]))
			: *((uint32_t *) &(data_buf[ATA_IDENTIFY_TOTAL_NUMBER]));

	for (int k = 0; k < sizeof(unit->model) - 1; k += 2) {
		unit->model[k] = data_buf[ATA_IDENTIFY_MODEL_NUMBER + k + 1];
		unit->model[k + 1] = data_buf[ATA_IDENTIFY_MODEL_NUMBER + k];
	}
	unit->model[sizeof(unit->model) - 1] = '\0';
}

static bool _check_unit(const void *unit)
{
	//TODO check more strictly
	return (unit ? true : false);
}

static bool _check_param(const void *unit, off_t *offset, size_t *size)
{
	if (!_check_unit(unit))
		return false;

	if (*offset & (~MASK_LBA48_OFFSET))
		return false;

	if (*size & (~MASK_LBA48_SIZE))
		return false;

	ata_unit_t *u = (ata_unit_t *) unit;
	*offset &= MASK_LBA48_OFFSET;
	*offset >>= ATA_DEFAULT_SECTOR_SHIFT;
	if (*offset >= u->size)
		return false;

	size_t rest = u->size - *offset;
	*size &= MASK_LBA48_SIZE;
	*size >>= ATA_DEFAULT_SECTOR_SHIFT;
	if (*size > rest)
		return false;

	return true;
}

static void _set_address(const ata_port_t *p, const int device,
		const off_t address, const size_t n)
{
	while (_read_data(p, ATA_REGISTER_STATUS) & ATA_STATUS_BSY);

	_write_data(p, ATA_REGISTER_DEVICE,
			// use 0xa0 for old device
			0xa0 | ATA_DEVICE_LBA | (device << 4));
	_write_data(p, ATA_REGISTER_SECTOR_COUNT, (n >> 8) & 0xff);
	_write_data(p, ATA_REGISTER_LBA_LOW, (address >> 24) & 0xff);
	_write_data(p, ATA_REGISTER_LBA_MID, (address >> 32) & 0xff);
	_write_data(p, ATA_REGISTER_LBA_HIGH, (address >> 40) & 0xff);
	_write_data(p, ATA_REGISTER_SECTOR_COUNT, n & 0xff);
	_write_data(p, ATA_REGISTER_LBA_LOW, address & 0xff);
	_write_data(p, ATA_REGISTER_LBA_MID, (address >> 8) & 0xff);
	_write_data(p, ATA_REGISTER_LBA_HIGH, (address >> 16) & 0xff);
}

int ata_read(char *buf, const void *unit, const off_t offset, const size_t size)
{
	off_t o = offset;
	size_t s = size;
	if (!_check_param(unit, &o, &s))
		return (-1);

	if (!s)
		return 0;

	ata_unit_t *u = (ata_unit_t *) unit;
	ata_port_t *p = &(ports[u->port]);
	_set_address(p, u->device, o, s);
	_write_data(p, ATA_REGISTER_COMMAND, READ_SECTORS_EXT);

	uint16_t *w = (uint16_t *) buf;
	for (int i = 0; i < s; i++) {
		if (_poll(p)) {
#if 0
			kcall->printk("ata: read error\n");
#endif
			return (-1);
		}

		for (int j = 0; j < ATA_DEFAULT_SECTOR_SIZE / 2; j++) {
			*w = inw(p->data + ATA_REGISTER_DATA);
			w++;
		}
	}

	return size;
}

int ata_write(char *buf, const void *unit, const off_t offset,
		const size_t size)
{
	off_t o = offset;
	size_t s = size;
	if (!_check_param(unit, &o, &s))
		return (-1);

	if (!s)
		return 0;

	ata_unit_t *u = (ata_unit_t *) unit;
	ata_port_t *p = &(ports[u->port]);
	_set_address(p, u->device, o, s);
	_write_data(p, ATA_REGISTER_COMMAND, WRITE_SECTORS_EXT);

	uint16_t *w = (uint16_t *) buf;
	for (int i = 0; i < s; i++) {
		if (_poll(p)) {
#if 0
			kcall->printk("ata: write error\n");
#endif
			return (-1);
		}

		for (int j = 0; j < ATA_DEFAULT_SECTOR_SIZE / 2; j++) {
			outw(p->data + ATA_REGISTER_DATA, *w);
			w++;
		}
	}

	_write_data(p, ATA_REGISTER_COMMAND, FLUSH_CACHE_EXT);
	if (_poll(p)) {
#if 0
		kcall->printk("ata: flush error\n");
#endif
		return (-1);
	}

	return size;
}

void *ata_open(const int channel)
{
	if ((channel < 0)
			|| (channel >= sizeof(units) / sizeof(units[0])))
		return NULL;

	ata_unit_t *unit = &(units[channel]);
	if ((unit->type == ATA)
			&& (unit->capabilities & ATA_CAPABILITY_LBA)
			&& ((unit->command_sets & LBA48_COMMAND_SETS_MASK)
					== LBA48_COMMAND_SETS_MASK))
		return &(units[channel]);

	return NULL;
}

int ata_initialize(void)
{
	peripheral_t *p = peripheral_find(PCI_CLASS_MASS_STORAGE_CONTROLLER,
			PCI_SUBCLASS_IDE_CONTROLLER);
	if (!p)
		return (-1);
#if 0
	field = peripheral_get_config(p->bus, p->device, p->func, 15);
	outb(0xcfc, 0xfe);
	field = peripheral_get_config(p->bus, p->device, p->func, 15);
	if ((field & 0xff) == 0xfe)
		kcall->printk("ata: need IRQ\n");
	else if ((p->type.progIf == 0x8a)
			|| (p->type.progIf == 0x80))
		kcall->printk("ata: parallel\n");
#endif
	if (!(p->type.progIf & PCI_PROGIF_BUS_MASTER))
		return (-1);

	for (int i = 0; i < 5; i++) {
		uint32_t field = peripheral_get_config(p->bus, p->device,
				p->func, i + 4);
#if 0
		kcall->printk("ata: bar%d = %x\n", i, field);
#endif
		_set_bar(i, field);
	}

	ata_port_t *port = &(ports[PORT_PRIMARY]);
	port->data = bar[0] ? (bar[0] & BAR_PORT_MASK) : PORT1_DATA;
	port->control = bar[1] ? (bar[1] & BAR_PORT_MASK) : PORT1_CONTROL;
	port->bus_master = bar[4] & BAR_PORT_MASK;
	_write_control(&(ports[PORT_PRIMARY]), ATA_REGISTER_DEVICE_CONTROL,
			ATA_CONTROL_NLEN);
	_identify(&(units[0]), PORT_PRIMARY, DEVICE0);
	_identify(&(units[1]), PORT_PRIMARY, DEVICE1);

	port = &(ports[PORT_SECONDARY]);
	port->data = bar[2] ? (bar[2] & BAR_PORT_MASK) : PORT2_DATA;
	port->control = bar[3] ? (bar[3] & BAR_PORT_MASK) : PORT2_CONTROL;
	port->bus_master = (bar[4] & BAR_PORT_MASK) + 8;
	_write_control(&(ports[PORT_SECONDARY]), ATA_REGISTER_DEVICE_CONTROL,
			ATA_CONTROL_NLEN);
	_identify(&(units[2]), PORT_SECONDARY, DEVICE0);
	_identify(&(units[3]), PORT_SECONDARY, DEVICE1);

	for (int i = 0; i < sizeof(units) / sizeof(units[0]); i++) {
		ata_unit_t *unit = &(units[i]);
		if (unit->type)
			kcall->printk("ata: (%d, %d) %d, %x %x, %x, %s\n",
					unit->port, unit->device, unit->type,
					((int32_t *) &(unit->size))[1],
					((int32_t *) &(unit->size))[0],
					unit->command_sets, unit->model);
	}

	return 0;
}

static unsigned int _msleep(const unsigned int ms)
{
	struct timespec t = { 0, ms * 1000 * 1000 };
	ER_UINT reply_size = kcall->ipc_call(PORT_TIMER, &t, sizeof(t));
	if (reply_size == sizeof(ER)) {
		ER *result = (ER *) &t;
		switch (*result) {
		case E_TMOUT:
			return 0;
		case E_PAR:
		case E_NOMEM:
			return ms;
		default:
			break;
		}
	}

	return ms;
}
