#ifndef __ARCH_ATA_H__
#define __ARCH_ATA_H__
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
// data registers
#define ATA_REGISTER_DATA 0x0
#define ATA_REGISTER_ERROR 0x01
#define ATA_REGISTER_FEATURES 0x01
#define ATA_REGISTER_SECTOR_COUNT 0x02
#define ATA_REGISTER_LBA_LOW 0x03
#define ATA_REGISTER_LBA_MID 0x04
#define ATA_REGISTER_LBA_HIGH 0x05
#define ATA_REGISTER_DEVICE 0x06
#define ATA_REGISTER_STATUS 0x07
#define ATA_REGISTER_COMMAND 0x07

// control registers
#define ATA_REGISTER_ALTERNATE_STATUS (0x02)
#define ATA_REGISTER_DEVICE_CONTROL (0x02)

// statuses
#define ATA_STATUS_BSY 0x80
#define ATA_STATUS_DRDY 0x40
#define ATA_STATUS_DFSE 0x20
#define ATA_STATUS_DSC 0x10
#define ATA_STATUS_DRQ 0x08
#define ATA_STATUS_ERRCHK 0x01

// errors
#define ATA_ERROR_COMMAND_ABORTED 0x04

// commands
typedef enum {
	READ_SECTORS_EXT = 0x24,
#if 0
	READ_DMA_EXT = 0x25,
#endif
	WRITE_SECTORS_EXT = 0x34,
#if 0
	WRITE_DMA_EXT = 0x35,
	PACKET = 0xa0,
#endif
	IDENTIFY_PACKET_DEVICE = 0xa1,
#if 0
	IDLE = 0xe3
#endif
	IDENTIFY_DEVICE = 0xec,
	FLUSH_CACHE_EXT = 0xea
} ata_command_e;

#if 0
typedef enum {
	START_STOP_UNIT = 0x1b,
	READ_12  = 0xa8
} atapi_command_e;
#endif

// identify
#define ATA_IDENTIFY_GENERAL_CONFIGURATION (0)
#define ATA_IDENTIFY_SERIAL_NUMBER (20)
#define ATA_IDENTIFY_MODEL_NUMBER (54)
#define ATA_IDENTIFY_CAPABILITIES (98)
#define ATA_IDENTIFY_FIELD_VALIDITY (106)
#define ATA_IDENTIFY_TOTAL_NUMBER (120)
#define ATA_IDENTIFY_COMMAND_SETS_SUPPORTED (164)
#define ATA_IDENTIFY_MAX_LBA48 (200)

#define ATA_SIZE_OF_IDENTIFY (512)

#define ATA_GENERAL_CONFIGURATION_ATA 0x8000
#if 0
#define ATA_GENERAL_CONFIGURATION_REMOVABLE 0x80
#define ATA_GENERAL_CONFIGURATION_INCOMPLETE 0x04

#define ATA_CAPABILITY_DMA 0x0100
#endif
#define ATA_CAPABILITY_LBA 0x0200

#if 0
#define ATA_COMMAND_SETS_POWER_MANAGEMENT 0x00000008
#endif
#define ATA_COMMAND_SETS_48BIT_ADDRESS 0x04000000
#define ATA_COMMAND_SETS_FLUSH_CACHE_EXT 0x20000000

// others
#define ATA_DEVICE_LBA 0x40

#define ATA_DEFAULT_SECTOR_SIZE (512)

// types
typedef enum {
	ATA = 0x01,
	ATAPI = 0x02
} ata_type_e;

#endif
