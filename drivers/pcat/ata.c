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
//#include <string.h>
//#include <nerve/config.h>
#include <nerve/kcall.h>
#include <arch/archfunc.h>
#include <arch/ata.h>
#include <mpu/io.h>
#include <time.h>
#include <services.h>

#define POLLING_WAIT_COUNT (4)
#define POLLING_FAULT (1)
#define POLLING_ERROR (2)
#define POLLING_NOT_READY (3)

typedef struct {
	uint16_t data;
	uint16_t control;
	uint16_t bus_master;
	uint8_t no_interrupt;
} ata_channel_t;

typedef struct {
	uint16_t type;
	uint8_t channel;
	uint8_t controller;
	uint16_t signature;
	uint16_t capabilities;
	uint32_t command_sets;
	uint32_t size;
	uint8_t model[41];
} ata_device_t;

static ata_channel_t channels[2];
static ata_device_t devices[4];
static uint8_t ata_buf[2048];
//volatile uint8_t ata_irq_invoked = 0;
//static uint8_t atapi_packet[12] = { 0xa8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static uint32_t bar[5];

static uint8_t ata_read_data(uint8_t, uint8_t);
#if 0
static uint8_t ata_read_control(uint8_t, uint8_t);
static uint8_t ata_read_data_ex(uint8_t, uint8_t);
#endif
static void ata_write_data(uint8_t, uint8_t, uint8_t);
static void ata_write_control(uint8_t, uint8_t, uint8_t);
#if 0
static void ata_write_data_ex(uint8_t, uint8_t, uint8_t);
#endif
static void ata_read_buf(uint32_t *, uint8_t, uint8_t, size_t);
#if 0
static uint8_t ata_polling(uint8_t, uint32_t);
static uint8_t ata_error(uint8_t);
#endif
static void identify(const int, const int, ata_device_t *dev);
static unsigned int msleep(unsigned int);


int ata_set_bar(const int index, const uint32_t value)
{
	if ((index >= 0)
			&& (index < sizeof(bar) / sizeof(bar[0]))) {
		bar[index] = value;
		return 0;
	} else
		return (-1);
}

static uint8_t ata_read_data(uint8_t channel, uint8_t reg)
{
	return inb(channels[channel].data + reg);
}
#if 0
static uint8_t ata_read_control(uint8_t channel, uint8_t reg)
{
	return inb(channels[channel].control + reg);
}

static uint8_t ata_read_data_ex(uint8_t channel, uint8_t reg)
{
	uint8_t result = 0;
	ata_channel_t *ch = &(channels[channel]);
	ata_write_control(channel, ATA_REGISTER_DEVICE_CONTROL,
			0x80 | ch->no_interrupt);
	result = inb(ch->data + reg);
	ata_write_control(channel, ATA_REGISTER_DEVICE_CONTROL,
			ch->no_interrupt);
	return result;
}
#endif
static void ata_write_data(uint8_t channel, uint8_t reg, uint8_t data)
{
	outb(channels[channel].data + reg, data);
}

static void ata_write_control(uint8_t channel, uint8_t reg, uint8_t data)
{
	outb(channels[channel].control + reg, data);
}
#if 0
static void ata_write_data_ex(uint8_t channel, uint8_t reg, uint8_t data)
{
	ata_channel_t *ch = &(channels[channel]);
	ata_write_control(channel, ATA_REGISTER_DEVICE_CONTROL,
			0x80 | ch->no_interrupt);
	outb(ch->data + reg, data);
	ata_write_control(channel, ATA_REGISTER_DEVICE_CONTROL,
			ch->no_interrupt);
}
#endif
static void ata_read_buf(uint32_t *buf, uint8_t channel, uint8_t reg,
		size_t cnt)
{
	uint16_t port = channels[channel].data + reg;
	for (int i = 0; i < cnt; i++)
		buf[i] = inl(port);
}
#if 0
static uint8_t ata_polling(uint8_t channel, uint32_t check)
{
	for (int i = 0; i < POLLING_WAIT_COUNT; i++)
		ata_read_control(channel, ATA_REGISTER_ALTERNATE_STATUS);

	while (ata_read_data(channel, ATA_REGISTER_STATUS) & ATA_STATUS_BSY);

	if (check) {
		uint8_t status = ata_read_data(channel, ATA_REGISTER_STATUS);
		if (status & ATA_STATUS_ERRCHK)
			return POLLING_ERROR;

		if (status & ATA_STATUS_DFSE)
			return POLLING_FAULT;

		if (!(status & ATA_STATUS_DRQ))
			return POLLING_NOT_READY;
	}

	return 0;
}

static uint8_t ata_error(uint8_t channel)
{
	return ata_read_data(channel, ATA_REGISTER_ERROR);
}
#endif
static void identify(const int channel, const int controller, ata_device_t *dev)
{
	dev->type = 0;
	dev->channel = channel;
	dev->controller = controller;
	ata_write_data(channel, ATA_REGISTER_DEVICE, 0xa0 | (controller << 4));
	msleep(1);
	ata_write_data(channel, ATA_REGISTER_COMMAND, ATA_COMMAND_IDENTIFY);
	msleep(1);

	if (!ata_read_data(channel, ATA_REGISTER_STATUS))
		return;

	for (;;) {
		uint8_t status = ata_read_data(channel, ATA_REGISTER_STATUS);
		if (status & ATA_STATUS_ERRCHK) {
			uint8_t cl = ata_read_data(channel,
					ATA_REGISTER_LBA_MIDDLE);
			uint8_t ch = ata_read_data(channel,
					ATA_REGISTER_LBA_HIGH);
			if (((cl == 0x14) && (ch == 0xeb))
					|| ((cl == 0x69) && (ch == 0x96)))
				dev->type = ATA_TYPE_ATAPI;
			else
				return;

			ata_write_data(channel, ATA_REGISTER_COMMAND,
					ATA_COMMAND_IDENTIFY_PACKET);
			msleep(1);
			break;
		}

		if (!(status & ATA_STATUS_BSY)
				&& (status & ATA_STATUS_DRQ)) {
			dev->type = ATA_TYPE_ATA;
			break;
		}
	}

	ata_read_buf((uint32_t *) &ata_buf, channel, ATA_REGISTER_DATA, 128);
	dev->signature = *((uint16_t *) &(ata_buf[ATA_IDENTIFY_DEVICE_TYPE]));
	dev->capabilities =
			*((uint16_t *) &(ata_buf[ATA_IDENTIFY_CAPABILITIES]));
	dev->command_sets =
			*((uint32_t *) &(ata_buf[ATA_IDENTIFY_COMMAND_SETS]));
	dev->size = *((uint32_t *) &(ata_buf[
			(dev->command_sets & (1 << 26)) ?
					ATA_IDENTIFY_MAX_LBA_EX
					: ATA_IDENTIFY_MAX_LBA]));

	for (int k = 0; k < sizeof(dev->model) - 1; k += 2) {
		dev->model[k] = ata_buf[ATA_IDENTIFY_MODEL + k + 1];
		dev->model[k + 1] = ata_buf[ATA_IDENTIFY_MODEL + k];
	}
	dev->model[sizeof(dev->model) - 1] = '\0';
}

void ata_initialize(void)
{
	ata_channel_t *ch = &(channels[ATA_CHANNEL_PRIMARY]);
	ch->data = bar[0] ? (bar[0] & BAR_PORT_MASK) : 0x1f0;
	ch->control = bar[1] ? (bar[1] & BAR_PORT_MASK) : 0x3f6;
	ch->bus_master = bar[4] & BAR_PORT_MASK;
	ata_write_control(ATA_CHANNEL_PRIMARY, ATA_REGISTER_DEVICE_CONTROL, 2);
	identify(ATA_CHANNEL_PRIMARY, ATA_CONTROLLER_MASTER, &(devices[0]));
	identify(ATA_CHANNEL_PRIMARY, ATA_CONTROLLER_SLAVE, &(devices[1]));

	ch = &(channels[ATA_CHANNEL_SECONDARY]);
	ch->data = bar[2] ? (bar[2] & BAR_PORT_MASK) : 0x170;
	ch->control = bar[3] ? (bar[3] & BAR_PORT_MASK) : 0x376;
	ch->bus_master = (bar[4] & BAR_PORT_MASK) + 8;
	ata_write_control(ATA_CHANNEL_SECONDARY, ATA_REGISTER_DEVICE_CONTROL, 2);
	identify(ATA_CHANNEL_SECONDARY, ATA_CONTROLLER_MASTER, &(devices[2]));
	identify(ATA_CHANNEL_SECONDARY, ATA_CONTROLLER_SLAVE, &(devices[3]));

	for (int i = 0; i < sizeof(devices) / sizeof(devices[0]); i++)
		if (devices[i].type)
			kcall->printk("ata: (%d, %d) %d, %x, %s\n",
					devices[i].channel,
					devices[i].controller,
					devices[i].type,
					devices[i].size, devices[i].model);
}

static unsigned int msleep(unsigned int ms)
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
