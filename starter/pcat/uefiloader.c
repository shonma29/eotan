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
#include <stddef.h>
#include <mpu/io.h>
#include <nerve/global.h>
#include <starter/boot.h>
#include <starter/uefi.h>

#define PUT_BUF_SIZE (128)

typedef struct {
	uintn_t map_key;
	void *map_buf;
	uintn_t map_size;
	uintn_t descriptor_size;
	uint32_t descriptor_version;
} memory_info_t;

extern const char KernelImage[];
extern const int32_t Len_KernelImage;

static efi_guid_t gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
static char character_table[] = {
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	'a', 'b', 'c', 'd', 'e', 'f'
};
static memory_info_t memory_info = { 0, NULL, 0, 0, 0 };

static void puth(efi_system_table_t *, const int64_t);
static void puts(efi_system_table_t *, const char *);
static bool set_gop(efi_system_table_t *);
static void copy_starter(void);
static bool get_memory_map(efi_system_table_t *);


static void puth(efi_system_table_t *sys, const int64_t n)
{
	wchar_t buf[16 + 1];
	wchar_t *w = &(buf[sizeof(buf) /sizeof(buf[0]) - 1]);
	*w = L'\0';

	uint64_t m = (uint64_t) n;
	do {
		*--w = character_table[m & 0xf];
		m >>= 4;
	} while (m);

	sys->ConOut->OutputString(sys->ConOut, w);
}

static void puts(efi_system_table_t *sys, const char *s)
{
	wchar_t buf[PUT_BUF_SIZE];
	wchar_t *w = buf;
	unsigned char *r = (unsigned char *) s;

	for (long i = 0; *r;) {
		if (i >= sizeof(buf) / sizeof(buf[0]) - 2) {
			*w = L'\0';
			sys->ConOut->OutputString(sys->ConOut, buf);
			i = 0;
			w = buf;
		} else if (*r == '\n') {
			*w++ = L'\r';
			*w++ = L'\n';
			r++;
			i += 2;
		} else {
			*w++ = *r++;
			i++;
		}
	}

	*w = L'\0';
	sys->ConOut->OutputString(sys->ConOut, buf);
}

static bool set_gop(efi_system_table_t *sys)
{
	efi_graphics_output_protocol_t *gop;
	efi_status_t result = sys->BootServices->LocateProtocol(
			&gop_guid, NULL, (void **) &gop);
	if (result != EFI_SUCCESS) {
		puts(sys, "locateProtocol failed ");
		puth(sys, result);
		return false;
	}

	efi_graphics_output_protocol_mode_t *gop_mode = gop->Mode;
	if (gop_mode->MaxMode == 0) {
		puts(sys, "No graphic mode");
		return false;
	}

	if (gop_mode->Info->PixelFormat != PixelBlueGreenRedReserved8BitPerColor) {
		puts(sys, "Unsupported graphic mode ");
		puth(sys, gop_mode->Info->PixelFormat);
		return false;
	}

	Display *d = &(sysinfo->display);
	d->r.min.x = 0;
	d->r.min.y = 0;
	d->r.max.x = gop_mode->Info->HorizontalResolution;
	d->r.max.y = gop_mode->Info->VerticalResolution;
	d->base = (void *) (gop_mode->FrameBufferBase);
	d->bpl = gop_mode->FrameBufferSize / gop_mode->Info->VerticalResolution;
	d->bpp = 4;
	return true;
}

static void copy_starter(void)
{
	int64_t *r = (int64_t *) KernelImage;
	int64_t *w = (int64_t *) STARTER_ADDR;
	for (long i = 0; i < Len_KernelImage / sizeof(*r); i++)
		*w++ = *r++;
}

static bool get_memory_map(efi_system_table_t *sys)
{
	efi_status_t result;
	for (;;) {
		result = sys->BootServices->GetMemoryMap(&(memory_info.map_size),
				(efi_memory_descriptor_t *) memory_info.map_buf,
				&(memory_info.map_key), &(memory_info.descriptor_size),
				&(memory_info.descriptor_version));
		if (result == EFI_SUCCESS) {
			MemoryInfo *m = (MemoryInfo *) MEMORY_INFO_ADDR;
			m->map_buf = memory_info.map_buf;
			m->map_size = memory_info.map_size;
			m->descriptor_size = memory_info.descriptor_size;
			return true;
		} else if (result == EFI_BUFFER_TOO_SMALL) {
			if (memory_info.map_buf) {
				result = sys->BootServices->FreePool(&(memory_info.map_buf));
				if (result != EFI_SUCCESS) {
					puts(sys, "FreePool failed ");
					break;
				}

				memory_info.map_buf = NULL;
			}

			result = sys->BootServices->AllocatePool(0, memory_info.map_size * 2,
					&(memory_info.map_buf));
			if (result != EFI_SUCCESS) {
				puts(sys, "AllocatePool failed ");
				break;
			}
		} else {
			puts(sys, "GetMemoryMap failed ");
			break;
		}
	}

	puth(sys, result);
	return false;
}

void start(efi_handle_t *image, efi_system_table_t *sys)
{
	puts(sys, "uefiloader 1.0\n");

	do {
		if (!set_gop(sys))
			break;

		copy_starter();

		if (!get_memory_map(sys))
			break;

		efi_status_t result = sys->BootServices->ExitBootServices(
				image, memory_info.map_key);
		if (result == EFI_SUCCESS) {
			void (*starter)(void) = (void (*)(void)) STARTER_ADDR;
			starter();
		} else {
			puts(sys, "ExitBootServices failed ");
			puth(sys, result);
		}
	} while (false);

	for (;;)
		halt();
}
