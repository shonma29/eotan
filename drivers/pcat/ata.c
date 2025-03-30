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
#undef DEBUG

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <archfunc.h>
#include <event.h>
#include <nerve/kcall.h>
#include <nerve/icall.h>
#include <mpu/io.h>
#include <ata.h>
#include <arch/8259a.h>
#include "../../lib/libserv/libserv.h"
#include "pci.h"

#define MYNAME "ata"

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

// masks
#define PROGIF_MASK (PCI_PROGIF_BUS_MASTER | PCI_PROGIF_PRIMARY_NATIVE \
	| PCI_PROGIF_SECONDARY_NATIVE)

#define LBA48_COMMAND_SETS_MASK (ATA_COMMAND_SETS_48BIT_ADDRESS \
			| ATA_COMMAND_SETS_FLUSH_CACHE_EXT)

#define LBA48_OFFSET_MASK 0x01fffffffffffe00
#define LBA48_SIZE_MASK 0x01fffe00

#define STATUS_MASK (ATA_STATUS_DFSE | ATA_STATUS_DRQ | ATA_STATUS_ERRCHK)
#define WAIT_MASK (ATA_STATUS_BSY | ATA_STATUS_DRQ)
#define ERROR_MASK (ATA_STATUS_DFSE | ATA_STATUS_ERRCHK)

#define POLL_WAIT_COUNT (4)

#define NONE (-1)

#define ERROR (-1)
#define ERROR_ATA (1)

typedef struct {
	uint16_t data;
	uint16_t control;
	uint16_t bus_master;
	ID isr;
	ID caller;
	uint8_t error_detail;
} ata_port_t;

typedef struct {
	ata_type_e type;
	uint8_t port;
	uint8_t device;
	uint16_t signature;//TODO delete if unused
	uint16_t capabilities;
	uint32_t command_sets;
	uint64_t size;
	uint8_t model[41];
} ata_unit_t;

typedef struct {
	const ata_unit_t *unit;
	uint64_t lba;
	uint64_t size;
	uint8_t type;
} ata_partition_t;

static ata_port_t ports[2];
static ata_unit_t units[4];
static ata_partition_t partitions[1];
static uint32_t bar[5];
static char data_buf[2048];//TODO use palloc for DMA

static void _handle_1st(VP_INT);
static void _handle_2nd(const int, const int);
static uint8_t _read_data(const ata_port_t *, const uint8_t);
static uint8_t _read_control(const ata_port_t *, const uint8_t);
static void _write_data(const ata_port_t *, const uint8_t, const uint8_t);
static void _write_control(const ata_port_t *, const uint8_t, const uint8_t);
static void _read_buf(uint16_t *, const ata_port_t *, const uint8_t,
		const size_t);
static void _set_error_detail(ata_port_t *, const uint8_t);
static int _poll(ata_port_t *);
static void _delay(const ata_port_t *);
static uint8_t _busywait(const ata_port_t *, const uint8_t);
static uint8_t _select_device(ata_port_t *, const int);
static void _identify(ata_unit_t *, const int, const int);
static bool _check_partition(const void *);
static bool _check_param(const void *, off_t *, size_t *);
static int _set_address(ata_port_t *, const int, const off_t,
		const size_t);
static void *_find_partition(const ata_unit_t *, const uint8_t);
static void _initialize_interrupt(ata_port_t *);
static int _set_interrupt(const int, const void *);

static inline bool has_error(uint8_t status)
{
	return ((status & ERROR_MASK) ? true : false);
}


static void _handle_1st(VP_INT exinf)
{
	ata_port_t *p = (ata_port_t *) exinf;
	if (p->caller >= 0)
		icall->handle(_handle_2nd, exinf, 0);
}

static void _handle_2nd(const int port, const int dummy)
{
	ata_port_t *p = (ata_port_t *) port;
#ifdef DEBUG
	int result =
#endif
			kcall->ipc_notify(p->caller, EVENT_INTERRUPT);
#ifdef DEBUG
	if (result)
		log_debug(MYNAME ": failed to notify(%d) %d\n",
				p->caller, result);
#endif
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

static void _read_buf(uint16_t *buf, const ata_port_t *p, const uint8_t reg,
		const size_t cnt)
{
	uint16_t address = p->data + reg;
	for (int i = 0; i < cnt; i++)
		buf[i] = inw(address);
}

static void _set_error_detail(ata_port_t *p, const uint8_t status)
{
	p->error_detail = 0;

	if (status & ATA_STATUS_ERRCHK)
		p->error_detail = _read_data(p, ATA_REGISTER_ERROR);
}

static int _poll(ata_port_t *p)
{
	int result = kcall->ipc_listen();
	if (result != E_RLWAI)
		return result;

	uint8_t status = _read_data(p, ATA_REGISTER_STATUS) & ERROR_MASK;
	if (status)
		_set_error_detail(p, status);

	return status;
}

static void _delay(const ata_port_t *p)
{
	for (int i = 0; i < POLL_WAIT_COUNT; i++)
		_read_control(p, ATA_REGISTER_ALTERNATE_STATUS);
}

static uint8_t _busywait(const ata_port_t *p, const uint8_t flag)
{
	uint8_t status;
	do {
		status = _read_data(p, ATA_REGISTER_STATUS);
	} while ((status & WAIT_MASK) != flag);

	return (status & STATUS_MASK);
}

static uint8_t _select_device(ata_port_t *p, const int device)
{
	_busywait(p, 0);
	_write_data(p, ATA_REGISTER_DEVICE,
			// use 0xa0 for old device
			0xa0 | ATA_DEVICE_LBA | (device << 4));
	_delay(p);
	return _busywait(p, 0);
}

static void _identify(ata_unit_t *unit, const int port, const int device)
{
	unit->type = 0;
	unit->port = port;
	unit->device = device;

	ata_port_t *p = &(ports[port]);
	if (_select_device(p, device))
		return;

	_write_data(p, ATA_REGISTER_COMMAND, IDENTIFY_DEVICE);
	_delay(p);

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
			_delay(p);
			break;
		}

		if (!(status & ATA_STATUS_BSY)
				&& (status & ATA_STATUS_DRQ)) {
			unit->type = ATA;
			break;
		}
	}

	_read_buf((uint16_t *) &data_buf, p, ATA_REGISTER_DATA,
			ATA_SIZE_OF_IDENTIFY / sizeof(uint16_t));
	unit->signature =
			*((uint16_t *) &(data_buf[ATA_IDENTIFY_GENERAL_CONFIGURATION]));
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

static bool _check_partition(const void *partition)
{
	//TODO check more strictly
	return (partition ? true : false);
}

static bool _check_param(const void *partition, off_t *offset, size_t *size)
{
	if (!_check_partition(partition))
		return false;

	if (*offset & (~LBA48_OFFSET_MASK))
		return false;

	if (*size & (~LBA48_SIZE_MASK))
		return false;

	ata_partition_t *p = (ata_partition_t *) partition;
	*offset &= LBA48_OFFSET_MASK;
	*offset >>= ATA_DEFAULT_SECTOR_SHIFT;
	if (*offset >= p->size)
		return false;

	size_t rest = p->size - *offset;
	*size &= LBA48_SIZE_MASK;
	*size >>= ATA_DEFAULT_SECTOR_SHIFT;
	if (*size > rest)
		return false;

	*offset += p->lba;
	return true;
}

static int _set_address(ata_port_t *p, const int device,
		const off_t address, const size_t n)
{
	uint8_t status = _select_device(p, device);
	if (has_error(status)) {
		_set_error_detail(p, status);
		log_warning(MYNAME ": select error %x %x\n", status,
				p->error_detail);
		return ERROR_ATA;
	}

	_write_data(p, ATA_REGISTER_SECTOR_COUNT, (n >> 8) & 0xff);
	_write_data(p, ATA_REGISTER_LBA_LOW, (address >> 24) & 0xff);
	_write_data(p, ATA_REGISTER_LBA_MID, (address >> 32) & 0xff);
	_write_data(p, ATA_REGISTER_LBA_HIGH, (address >> 40) & 0xff);
	_write_data(p, ATA_REGISTER_SECTOR_COUNT, n & 0xff);
	_write_data(p, ATA_REGISTER_LBA_LOW, address & 0xff);
	_write_data(p, ATA_REGISTER_LBA_MID, (address >> 8) & 0xff);
	_write_data(p, ATA_REGISTER_LBA_HIGH, (address >> 16) & 0xff);
	return 0;
}

int ata_read(char *buf, const void *partition, const off_t offset,
		const size_t size)
{
	off_t o = offset;
	size_t s = size;
	if (!_check_param(partition, &o, &s))
		return ERROR;

	if (!s)
		return 0;

	ata_unit_t *u = (ata_unit_t *) ((ata_partition_t *) partition)->unit;
	ata_port_t *p = &(ports[u->port]);
	if (_set_address(p, u->device, o, s))
		return ERROR;

	p->caller = kcall->thread_get_id();
	_write_data(p, ATA_REGISTER_COMMAND, READ_SECTORS_EXT);

	for (int i = 0; i < s; i++) {
		int result = _poll(p);
		if (result) {
			log_warning(MYNAME ": read error %x %x\n", result,
					p->error_detail);
			p->caller = NONE;
			return ERROR;
		}

		_read_buf((uint16_t *) buf, p, ATA_REGISTER_DATA,
				ATA_DEFAULT_SECTOR_SIZE / sizeof(uint16_t));
	}

	_read_control(p, ATA_REGISTER_ALTERNATE_STATUS);

	uint8_t status = _read_data(p, ATA_REGISTER_STATUS);
	if (has_error(status)) {
		_set_error_detail(p, status);
		log_warning(MYNAME ": read error %x %x\n", status,
				p->error_detail);
		return ERROR;
	}

	p->caller = NONE;
	return size;
}

int ata_write(char *buf, const void *partition, const off_t offset,
		const size_t size)
{
	off_t o = offset;
	size_t s = size;
	if (!_check_param(partition, &o, &s))
		return ERROR;

	if (!s)
		return 0;

	ata_unit_t *u = (ata_unit_t *) ((ata_partition_t *) partition)->unit;
	ata_port_t *p = &(ports[u->port]);

	if (_set_address(p, u->device, o, s))
		return ERROR;

	p->caller = kcall->thread_get_id();
	_write_data(p, ATA_REGISTER_COMMAND, WRITE_SECTORS_EXT);
	_delay(p);

	uint8_t status = _busywait(p, ATA_STATUS_DRQ);
	if (has_error(status)) {
		_set_error_detail(p, status);
		log_warning(MYNAME ": write error %x %x\n", status,
				p->error_detail);
		p->caller = NONE;
		return ERROR;
	}

	uint16_t *w = (uint16_t *) buf;
	for (int i = 0; i < s; i++) {
		for (int j = 0; j < ATA_DEFAULT_SECTOR_SIZE / sizeof(*w); j++) {
			outw(p->data + ATA_REGISTER_DATA, *w);
			w++;
		}

		int result = _poll(p);
		if (result) {
			log_warning(MYNAME ": write error %x %x\n", result,
					p->error_detail);
			p->caller = NONE;
			return ERROR;
		}
	}

	_write_data(p, ATA_REGISTER_COMMAND, FLUSH_CACHE_EXT);

	int result = _poll(p);
	if (result) {
		log_warning(MYNAME ": flush error %x %x\n", result,
				p->error_detail);
		p->caller = NONE;
		return ERROR;
	}

	p->caller = NONE;
	return size;
}

static void *_find_partition(const ata_unit_t *unit, const uint8_t type)
{
	// set dummy partition
	partitions[0].unit = unit;
	partitions[0].lba = 0;
	partitions[0].size = unit->size;
	partitions[0].type = 0;

	// read MBR
	int result = ata_read(data_buf, &(partitions[0]), 0,
			ATA_DEFAULT_SECTOR_SIZE);
	if (result != ATA_DEFAULT_SECTOR_SIZE) {
		log_err(MYNAME ": failed to read MBR %d\n", result);
		return NULL;
	}

	// scan partition table
	mbr_partition_t *p =
			(mbr_partition_t *) &(data_buf[MBR_PARTITION_OFFSET]);
	for (int i = 0; (i < MBR_NUM_OF_PARTITION) && p[i].type; i++) {
		log_info(MYNAME ": partition[%d] %x %x %x %x\n", i,
				p[i].attribute, p[i].type, p[i].lba, p[i].size);

		if (p[i].type != type)
			continue;

		partitions[0].unit = unit;
		partitions[0].lba = p[i].lba;
		partitions[0].size = p[i].size;
		partitions[0].type = p[i].type;
		return &(partitions[0]);
	}

	return NULL;
}

void *ata_open(const int channel, const uint8_t type)
{
	if ((channel < 0)
			|| (channel >= sizeof(units) / sizeof(units[0])))
		return NULL;

	ata_unit_t *unit = &(units[channel]);
	if ((unit->type == ATA)
			&& (unit->capabilities & ATA_CAPABILITY_LBA)
			&& ((unit->command_sets & LBA48_COMMAND_SETS_MASK)
					== LBA48_COMMAND_SETS_MASK))
		return _find_partition(unit, type);

	return NULL;
}

static void _initialize_interrupt(ata_port_t *port)
{
	port->isr = NONE;
	port->caller = NONE;
	_write_control(port, ATA_REGISTER_DEVICE_CONTROL, ATA_CONTROL_NLEN);
}

static int _set_interrupt(const int irq, const void *handler)
{
	int port = irq - ir_ide_primary;
	T_CISR pk_cisr = {
		TA_HLNG,
		(VP_INT) (&(ports[port])),
		PIC_IR_VECTOR(irq),
		handler
	};
	ER_ID id = create_isr(&pk_cisr);
	if (id < 0) {
		log_err(MYNAME ": failed to bind(%d) %d\n", irq, id);
		return id;
	}

	int result = enable_interrupt(irq);
	if (result) {
		log_err(MYNAME ": failed to enable(%d) %d\n", irq, result);
		destroy_isr(id);
		return result;
	}

	ports[port].isr = id;
	_write_control(&(ports[port]), ATA_REGISTER_DEVICE_CONTROL, 0);
	return 0;
}

int ata_initialize(void)
{
	peripheral_t *p = peripheral_find(PCI_CLASS_MASS_STORAGE_CONTROLLER,
			PCI_SUBCLASS_IDE_CONTROLLER);
	if (!p)
		return ERROR;

	if ((p->type.progIf & PROGIF_MASK) != PCI_PROGIF_BUS_MASTER)
		return ERROR;

	for (int i = 0; i < 5; i++) {
		uint32_t field = peripheral_get_config(p->bus, p->device,
				p->func, i + 4);
		log_debug(MYNAME ": bar%d = %x\n", i, field);
		bar[i] = field;
	}

	ata_port_t *port = &(ports[PORT_PRIMARY]);
	port->data = bar[0] ? (bar[0] & BAR_PORT_MASK) : PORT1_DATA;
	port->control = bar[1] ? (bar[1] & BAR_PORT_MASK) : PORT1_CONTROL;
	port->bus_master = bar[4] & BAR_PORT_MASK;
	_initialize_interrupt(port);
	_identify(&(units[0]), PORT_PRIMARY, DEVICE0);
	_identify(&(units[1]), PORT_PRIMARY, DEVICE1);

	port = &(ports[PORT_SECONDARY]);
	port->data = bar[2] ? (bar[2] & BAR_PORT_MASK) : PORT2_DATA;
	port->control = bar[3] ? (bar[3] & BAR_PORT_MASK) : PORT2_CONTROL;
	port->bus_master = (bar[4] & BAR_PORT_MASK) + 8;
	_initialize_interrupt(port);
	_identify(&(units[2]), PORT_SECONDARY, DEVICE0);
	_identify(&(units[3]), PORT_SECONDARY, DEVICE1);

	for (int i = 0; i < sizeof(units) / sizeof(units[0]); i++) {
		ata_unit_t *unit = &(units[i]);
		if (unit->type)
			log_info(MYNAME ": (%d, %d) %d, %x %x, %x, %s\n",
					unit->port, unit->device, unit->type,
					((int32_t *) &(unit->size))[1],
					((int32_t *) &(unit->size))[0],
					unit->command_sets, unit->model);
	}

	if (_set_interrupt(ir_ide_primary, _handle_1st))
		return ERROR;

	if (_set_interrupt(ir_ide_secondary, _handle_1st)) {
		//TODO disable interrupt
		destroy_isr(ports[PORT_PRIMARY].isr);
		ports[PORT_PRIMARY].isr = NONE;
		return ERROR;
	}

	return 0;
}
