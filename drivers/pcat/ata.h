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
// status
#define ATA_STATUS_BSY 0x80
#define ATA_STATUS_DRDY 0x40
#define ATA_STATUS_DFSE 0x20
#define ATA_STATUS_DSC 0x10
#define ATA_STATUS_DRQ 0x08
#define ATA_STATUS_ERRCHK 0x01

// errors
#define ATA_ERROR_BAD_BLOCK 0x80
#define ATA_ERROR_UNCORRECTABLE_DATA 0x40
#define ATA_ERROR_MEDIA_CHANGED 0x20
#define ATA_ERROR_ID_MARK_NOT_FOUND 0x10
#define ATA_ERROR_MEDIA_CHANGE_REQUEST 0x08
#define ATA_ERROR_COMMAND_ABORTED 0x04
#define ATA_ERROR_TRACK0_NOT_FOUND 0x02
#define ATA_ERROR_ADDRESS_MARK_NOT_FOUND 0x01

// commands
#define ATA_COMMAND_READ_PIO 0x20
#define ATA_COMMAND_READ_PIO_EXT 0x24
#define ATA_COMMAND_WRITE_PIO 0x30
#define ATA_COMMAND_WRITE_PIO_EXT 0x34
#define ATA_COMMAND_READ_DMA 0xc8
#define ATA_COMMAND_READ_DMA_EXT 0x25
#define ATA_COMMAND_WRITE_DMA 0xca
#define ATA_COMMAND_WRITE_DMA_EXT 0x35
#define ATA_COMMAND_FLUSH 0xe7
#define ATA_COMMAND_FLUSH_EXT 0xea
#define ATA_COMMAND_PACKET 0xa0
#define ATA_COMMAND_IDENTIFY_PACKET 0xa1
#define ATA_COMMAND_IDENTIFY 0xec

#define ATAPI_COMMAND_READ 0xa8
#define ATAPI_COMMAND_EJECT 0x1b

// identify offset
#define ATA_IDENTIFY_DEVICE_TYPE (0)
#define ATA_IDENTIFY_CYLINDERS (2)
#define ATA_IDENTIFY_HEADS (6)
#define ATA_IDENTIFY_SECTORS (12)
#define ATA_IDENTIFY_SERIAL (20)
#define ATA_IDENTIFY_MODEL (54)
#define ATA_IDENTIFY_CAPABILITIES (98)
#define ATA_IDENTIFY_FIELD_VALID (106)
#define ATA_IDENTIFY_MAX_LBA (120)
#define ATA_IDENTIFY_COMMAND_SETS (164)
#define ATA_IDENTIFY_MAX_LBA_EXT (200)

// types
#define ATA_TYPE_ATA 0x01
#define ATA_TYPE_ATAPI 0x02

// contollers
#define ATA_CONTROLLER_MASTER 0x00
#define ATA_CONTROLLER_SLAVE 0x01

// registers
// data
#define ATA_REGISTER_DATA 0x0
#define ATA_REGISTER_ERROR 0x01
#define ATA_REGISTER_FEATURES 0x01
#define ATA_REGISTER_SECTOR_COUNT0 0x02
#define ATA_REGISTER_LBA_LOW 0x03
#define ATA_REGISTER_LBA_MIDDLE 0x04
#define ATA_REGISTER_LBA_HIGH 0x05
#define ATA_REGISTER_DEVICE 0x06
#define ATA_REGISTER_STATUS 0x07
#define ATA_REGISTER_COMMAND 0x07
// control
#define ATA_REGISTER_ALTERNATE_STATUS (0x0c - 0x0a)
#define ATA_REGISTER_DEVICE_CONTROL (0x0c - 0x0a)
#define ATA_REGISTER_DEVICE_ADDRESS (0x0d - 0x0a)

// channels
#define ATA_CHANNEL_PRIMARY 0x00
#define ATA_CHANNEL_SECONDARY 0x01

// directions
#define ATA_DIRECTION_READ 0x00
#define ATA_DIRECTION_WRITE 0x01

// bar
#define BAR_PORT_MASK 0xfffffffc

#endif
