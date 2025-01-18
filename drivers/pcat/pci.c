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
#include <string.h>
#include <archfunc.h>
#include <nerve/config.h>
#include <nerve/kcall.h>
#include <mpu/io.h>

#define PCI_CONFIG_MECHANISM1_OK 0x0001
#define PCI_CONFIG_HEADER_MULTIPLE 0x00800000

static peripheral_t peripherals[MAX_PERIPHERAL];
static int num_of_peripherals;

static bool add_peripheral(const uint8_t, const uint8_t,
		const uint8_t, const uint32_t);
static bool is_valid_vendor(const uint32_t);
static bool is_multiple_header(const uint32_t);
static bool is_pci_bridge(const uint32_t);
static uint8_t get_secondary_bus_number(const uint32_t);
static void walk_config(const uint8_t);


static bool add_peripheral(const uint8_t bus, const uint8_t device,
		const uint8_t func, const uint32_t field2)
{
	if (num_of_peripherals >= MAX_PERIPHERAL) {
		kcall->printk("pci: too many peripheral\n");
		return false;
	}

	//TODO heap sort
	peripheral_t *p = &(peripherals[num_of_peripherals++]);
	p->field2 = field2;
	p->bus = bus;
	p->device = device;
	p->func = func;
	return true;
}

peripheral_t *peripheral_find(const uint8_t classCode,
		const uint8_t subClass)
{
	for (int i = 0; i < num_of_peripherals; i++) {
		peripheral_t *p = &(peripherals[i]);
		if ((p->type.classCode == classCode)
				&& (p->type.subClass == subClass))
			return p;
	}

	return NULL;
}

uint32_t peripheral_get_config(const uint8_t bus, const uint8_t device,
		const uint8_t func, const uint8_t field)
{
	outl(0xcf8,
			0x80000000 | (bus << 16) | (device << 11)
					| (func << 8) | (field << 2));
	return inl(0xcfc);
}

static bool is_valid_vendor(const uint32_t field0)
{
	return ((field0 & 0xffff) != 0xffff);
}

static bool is_multiple_header(const uint32_t field3)
{
	return (field3 & PCI_CONFIG_HEADER_MULTIPLE);
}

static bool is_pci_bridge(const uint32_t field2)
{
	return (((field2 >> 16) & 0xffff) == 0x0604);
}

static uint8_t get_secondary_bus_number(const uint32_t field6)
{
	return ((field6 >> 8) & 0xff);
}

static void walk_config(const uint8_t bus)
{
//	kcall->printk("pci: bus %d\n", bus);

	for (uint8_t device = 0; device < 32; device++) {
		uint32_t field = peripheral_get_config(bus, device, 0, 0);
		if (!is_valid_vendor(field))
			continue;

		uint32_t field1 = peripheral_get_config(bus, device, 0, 1);
		uint32_t field2 = peripheral_get_config(bus, device, 0, 2);
		uint32_t field3 = peripheral_get_config(bus, device, 0, 3);
		kcall->printk("pci: (%d, %d, 0) %x %x %x %x\n",
					bus, device, field,
					field1, field2, field3);
		add_peripheral(bus, device, 0, field2);

		if (is_pci_bridge(field2)) {
//			kcall->printk("pci: bridge %d 0\n", device);
			field = peripheral_get_config(bus, device, 0, 6);
			walk_config(get_secondary_bus_number(field));
		}

		if (!is_multiple_header(field3))
			continue;

//		kcall->printk("pci: multiple %d %d 0\n", bus, device);
		for (uint8_t func = 1; func < 8; func++) {
			field = peripheral_get_config(bus, device, func, 0);
			if (!is_valid_vendor(field))
				continue;

			field1 = peripheral_get_config(bus, device, func, 1);
			field2 = peripheral_get_config(bus, device, func, 2);
			field3 = peripheral_get_config(bus, device, func, 3);
			kcall->printk("pci: (%d, %d, %d) %x %x %x %x\n",
					bus, device, func, field,
					field1, field2, field3);
			add_peripheral(bus, device, func, field2);

			if (is_pci_bridge(field2)) {
//				kcall->printk("pci: bridge %d %d\n",
//						device, func);
				field = peripheral_get_config(bus, device, func,
						6);
				walk_config(get_secondary_bus_number(field));
			}
		}
	}
}

void peripheral_set_map(void)
{
	memset(peripherals, 0, sizeof(sizeof(peripherals)));

	uint16_t *info = (void *) PERIPHERAL_INFO_ADDR;
//	kcall->printk("PERIPHERAL_INFO: %x\n", *info);

	if (*info != PCI_CONFIG_MECHANISM1_OK)
		return;

	uint32_t field = peripheral_get_config(0, 0, 0, 0);
	if (!is_valid_vendor(field))
		return;

	field = peripheral_get_config(0, 0, 0, 3);
	if (is_multiple_header(field))
		for (uint8_t func = 0; func < 8; func++) {
			field = peripheral_get_config(0, 0, func, 0);
			if (!is_valid_vendor(field))
				break;

			walk_config(func);
		}
	else
		walk_config(0);
}
