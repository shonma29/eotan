#ifndef _STARTER_BOOT_H_
#define _STARTER_BOOT_H_
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
#include <features.h>
#include <stdint.h>

#define STARTER_ADDR (0x8000)
#define START_DATA_ADDR 0x400000

#define MAX_IMAGE_SIZE (480 * 1024)

#ifdef USE_UEFI
#include <starter/uefi.h>

#define EFIAPI __attribute__((ms_abi))

#define MEMORY_INFO_ADDR 0x00001000

typedef struct {
	char *map_buf;
	uintptr_t map_size;
	uintptr_t descriptor_size;
	void *acpi_rsdp;
	EFIAPI void (*ResetSystem)(const efi_reset_type_t, const efi_status_t,
			const uintn_t, const void *);
} MemoryInfo;
#endif

#endif
