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

// results of poll
#define POLL_FAULT (1)
#define POLL_ERROR (2)
#define POLL_NOT_READY (3)
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
	uint32_t size;
	uint8_t model[41];
} ata_device_t;

static int default_channel = -1;
static ata_port_t ports[2];
static ata_device_t devices[4];
#if 0
volatile uint8_t irq_invoked = 0;
static uint8_t atapi_packet[12] = { 0xa8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
#endif
static uint32_t bar[5];
static uint8_t data_buf[2048];

static uint8_t _read_data(const uint8_t, const uint8_t);
static uint8_t _read_control(const uint8_t, const uint8_t);
static void _write_data(const uint8_t, const uint8_t, const uint8_t);
static void _write_control(const uint8_t, const uint8_t, const uint8_t);
static void _read_buf(uint32_t *, const uint8_t, const uint8_t, const size_t);
static uint8_t _read_error(const uint8_t);
static uint8_t _poll(const uint8_t, const bool);
static void _identify(ata_device_t *dev, const int, const int);
static void _set_address(const int, const int, const int, const int);
int ata_read(void *, const int, const int, const int);
int ata_write(const void *, const int, const int, const int);
static void _test(void);
static unsigned int _msleep(const unsigned int);


int ata_set_bar(const int index, const uint32_t value)
{
	if ((index >= 0)
			&& (index < sizeof(bar) / sizeof(bar[0]))) {
		bar[index] = value;
		return 0;
	} else
		return (-1);
}

static uint8_t _read_data(const uint8_t port, const uint8_t reg)
{
	return inb(ports[port].data + reg);
}

static uint8_t _read_control(const uint8_t port, const uint8_t reg)
{
	return inb(ports[port].control + reg);
}

static void _write_data(const uint8_t port, const uint8_t reg,
		const uint8_t data)
{
	outb(ports[port].data + reg, data);
}

static void _write_control(const uint8_t port, const uint8_t reg,
		const uint8_t data)
{
	outb(ports[port].control + reg, data);
}

static void _read_buf(uint32_t *buf, const uint8_t port, const uint8_t reg,
		const size_t cnt)
{
	uint16_t address = ports[port].data + reg;
	for (int i = 0; i < cnt; i++)
		buf[i] = inl(address);
}

static uint8_t _read_error(const uint8_t port)
{
	return _read_data(port, ATA_REGISTER_ERROR);
}

static uint8_t _poll(const uint8_t port, const bool check)
{
	for (int i = 0; i < POLL_WAIT_COUNT; i++)
		_read_control(port, ATA_REGISTER_ALTERNATE_STATUS);

	while (_read_data(port, ATA_REGISTER_STATUS) & ATA_STATUS_BSY);

	if (check) {
		uint8_t status = _read_data(port, ATA_REGISTER_STATUS);
		if (status & ATA_STATUS_ERRCHK)
			return POLL_ERROR;

		if (status & ATA_STATUS_DFSE)
			return POLL_FAULT;

		if (!(status & ATA_STATUS_DRQ))
			return POLL_NOT_READY;
	}

	return 0;
}

static void _identify(ata_device_t *dev, const int port, const int device)
{
	dev->type = 0;
	dev->port = port;
	dev->device = device;
	_write_data(port, ATA_REGISTER_DEVICE, 0xa0 | (device << 4));
	_msleep(1);
	_write_data(port, ATA_REGISTER_COMMAND, IDENTIFY_DEVICE);
	_msleep(1);

	if (!_read_data(port, ATA_REGISTER_STATUS))
		return;

	for (;;) {
		uint8_t status = _read_data(port, ATA_REGISTER_STATUS);
		if (status & ATA_STATUS_ERRCHK) {
			uint8_t bl = _read_data(port, ATA_REGISTER_LBA_MID);
			uint8_t bh = _read_data(port, ATA_REGISTER_LBA_HIGH);
			if (((bl == 0x14) && (bh == 0xeb))
					|| ((bl == 0x69) && (bh == 0x96)))
				dev->type = ATAPI;
			else
				return;

			_write_data(port, ATA_REGISTER_COMMAND,
					IDENTIFY_PACKET_DEVICE);
			_msleep(1);
			break;
		}

		if (!(status & ATA_STATUS_BSY)
				&& (status & ATA_STATUS_DRQ)) {
			dev->type = ATA;
			break;
		}
	}

	_read_buf((uint32_t *) &data_buf, port, ATA_REGISTER_DATA,
			ATA_SIZE_OF_IDENTIFY / sizeof(uint32_t));
	dev->signature = *((uint16_t *) &(data_buf[ATA_IDENTIFY_GENERAL_CONFIGURATION]));
	dev->capabilities =
			*((uint16_t *) &(data_buf[ATA_IDENTIFY_CAPABILITIES]));
	dev->command_sets =
			*((uint32_t *) &(data_buf[ATA_IDENTIFY_COMMAND_SETS_SUPPORTED]));
	dev->size = *((uint32_t *) &(data_buf[
			(dev->command_sets & ATA_COMMAND_SETS_48BIT_ADDRESS) ?
					ATA_IDENTIFY_MAX_LBA48
					: ATA_IDENTIFY_TOTAL_NUMBER]));

	for (int k = 0; k < sizeof(dev->model) - 1; k += 2) {
		dev->model[k] = data_buf[ATA_IDENTIFY_MODEL_NUMBER + k + 1];
		dev->model[k + 1] = data_buf[ATA_IDENTIFY_MODEL_NUMBER + k];
	}
	dev->model[sizeof(dev->model) - 1] = '\0';
}

static void _set_address(const int port, const int device,
		const int address, const int n)
{
	while (_read_data(port, ATA_REGISTER_STATUS) & ATA_STATUS_BSY);

	_write_data(port, ATA_REGISTER_DEVICE,
			// use 0xa0 for old device
			0xa0 | ATA_DEVICE_LBA | (device << 4));
	_write_data(port, ATA_REGISTER_SECTOR_COUNT, (n >> 16) & 0xff);
	_write_data(port, ATA_REGISTER_LBA_LOW, (address >> 24) & 0xff);
	_write_data(port, ATA_REGISTER_LBA_MID, 0);
	_write_data(port, ATA_REGISTER_LBA_HIGH, 0);
	_write_data(port, ATA_REGISTER_SECTOR_COUNT, n & 0xff);
	_write_data(port, ATA_REGISTER_LBA_LOW, address & 0xff);
	_write_data(port, ATA_REGISTER_LBA_MID, (address >> 8) & 0xff);
	_write_data(port, ATA_REGISTER_LBA_HIGH, (address >> 16) & 0xff);
}

int ata_read(void *buf, const int channel, const int address, const int n)
{
	if (channel != default_channel)
		return (-1);

	ata_device_t *d = &(devices[channel]);
	int port = d->port;
	int device = d->device;
	kcall->printk("read %x %x %x %x\n", address, n, port, device);
	_set_address(port, device, address, n);
	_write_data(port, ATA_REGISTER_COMMAND, READ_SECTORS_EXT);

	for (int i = 0; i < n; i++) {
		int result = _poll(port, true);
		if (result) {
			if (result == POLL_ERROR)
				result = (_read_error(port) << 16) | result;

			kcall->printk("ata: read error=%x\n", result);
			break;
		}

		uint16_t *w = (uint16_t *) buf;
		for (int j = 0; j < ATA_DEFAULT_SECTOR_SIZE / 2; j++)
			w[j] = inw(ports[port].data + ATA_REGISTER_DATA);
	}

	return 0;
}

int ata_write(const void *buf, const int channel, const int address,
		const int n)
{
	if (channel != default_channel)
		return (-1);

	ata_device_t *d = &(devices[channel]);
	int port = d->port;
	int device = d->device;
	kcall->printk("write %x %x %x %x\n", address, n, port, device);
	_set_address(port, device, address, n);
	_write_data(port, ATA_REGISTER_COMMAND, WRITE_SECTORS_EXT);

	for (int i = 0; i < n; i++) {
		_poll(port, false);

		uint16_t *w = (uint16_t *) buf;
		for (int j = 0; j < ATA_DEFAULT_SECTOR_SIZE / 2; j++)
			outw(ports[port].data + ATA_REGISTER_DATA, w[j]);
	}

	_write_data(port, ATA_REGISTER_COMMAND, FLUSH_CACHE_EXT);
	_poll(port, false);
	return 0;
}

static void _test(void)
{
	ata_read(data_buf, default_channel, 0, 1);
	kcall->printk("%x %x\n", data_buf[0], data_buf[1]);
	kcall->printk("%x %x\n", data_buf[510], data_buf[511]);

	uint16_t *w = (uint16_t *) data_buf;
	for (int i = 0; i < ATA_DEFAULT_SECTOR_SIZE / 2; i++)
		w[i] = i + 1;

	ata_write(data_buf, default_channel, devices[default_channel].size - 1,
			1);
	kcall->printk("%x %x\n", data_buf[0], data_buf[1]);
	kcall->printk("%x %x\n", data_buf[510], data_buf[511]);

	for (int i = 0; i < ATA_DEFAULT_SECTOR_SIZE; i++)
		data_buf[i] = 0;

	kcall->printk("%x %x\n", data_buf[0], data_buf[1]);
	kcall->printk("%x %x\n", data_buf[510], data_buf[511]);

	ata_read(data_buf, default_channel, devices[default_channel].size - 1,
			1);
	kcall->printk("%x %x\n", data_buf[0], data_buf[1]);
	kcall->printk("%x %x\n", data_buf[510], data_buf[511]);
}

void ata_initialize(void)
{
	ata_port_t *p = &(ports[PORT_PRIMARY]);
	p->data = bar[0] ? (bar[0] & BAR_PORT_MASK) : PORT1_DATA;
	p->control = bar[1] ? (bar[1] & BAR_PORT_MASK) : PORT1_CONTROL;
	p->bus_master = bar[4] & BAR_PORT_MASK;
	_write_control(PORT_PRIMARY, ATA_REGISTER_DEVICE_CONTROL, 2);
	_identify(&(devices[0]), PORT_PRIMARY, DEVICE0);
	_identify(&(devices[1]), PORT_PRIMARY, DEVICE1);

	p = &(ports[PORT_SECONDARY]);
	p->data = bar[2] ? (bar[2] & BAR_PORT_MASK) : PORT2_DATA;
	p->control = bar[3] ? (bar[3] & BAR_PORT_MASK) : PORT2_CONTROL;
	p->bus_master = (bar[4] & BAR_PORT_MASK) + 8;
	_write_control(PORT_SECONDARY, ATA_REGISTER_DEVICE_CONTROL, 2);
	_identify(&(devices[2]), PORT_SECONDARY, DEVICE0);
	_identify(&(devices[3]), PORT_SECONDARY, DEVICE1);

	for (int i = 0; i < sizeof(devices) / sizeof(devices[0]); i++) {
		ata_device_t *p = &(devices[i]);
		if (p->type)
			kcall->printk("ata: (%d, %d) %d, %x, %d, %s\n",
					p->port, p->device, p->type, p->size,
					p->command_sets, p->model);
		if ((default_channel < 0)
				&& (p->type == ATA)
				&& (p->capabilities & ATA_CAPABILITY_LBA)
				&& ((p->command_sets & LBA48_COMMAND_SETS_MASK)
						== LBA48_COMMAND_SETS_MASK))
			default_channel = i;
	}
	kcall->printk("ata: default_channel=%d\n", default_channel);
	_test();
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
