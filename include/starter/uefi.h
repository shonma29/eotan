#ifndef _STARTER_UEFI_H_
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
#include <stdbool.h>
#include <stdint.h>

typedef uint16_t wchar_t;

typedef uintptr_t uintn_t;
typedef uintn_t efi_status_t;

typedef struct efi_simple_text_output_protocol {
	char _padding1[8];
	efi_status_t (*OutputString)(
			const struct efi_simple_text_output_protocol *,
			const wchar_t *);
} efi_simple_text_output_protocol_t;

typedef struct {
	uint16_t Year;
	uint8_t Month;
	uint8_t Day;
	uint8_t Hour;
	uint8_t Minute;
	uint8_t Second;
	uint8_t Pad1;
	uint32_t Nanosecond;
	int16_t TimeZone;
	uint8_t Daylight;
	uint8_t Pad2;
} efi_time_t;

typedef struct {
	uint32_t Resolution;
	uint32_t Accuracy;
	bool SetsToZero;
} efi_time_capabilities_t;

typedef enum {
	EfiResetCold,
	EfiResetWarm,
	EfiResetShutdown,
	EfiResetPlatformSpecific
} efi_reset_type_t;

typedef struct efi_runtime_services {
	char _padding1[24];
	efi_status_t (*GetTime)(efi_time_t *, efi_time_capabilities_t *);
	efi_status_t (*SetTime)(const efi_time_t *);
	char _padding2[64];
	void (*ResetSystem)(const efi_reset_type_t, const efi_status_t,
			const uintn_t, const void *);
} efi_runtime_services_t;

typedef uint64_t efi_physical_address_t;
typedef uint64_t efi_virtual_address_t;

typedef enum {
	EfiReservedMemoryType,
	EfiLoaderCode,
	EfiLoaderData,
	EfiBootServicesCode,
	EfiBootServicesData,
	EfiRuntimeServicesCode,
	EfiRuntimeServicesData,
	EfiConventionalMemory,
	EfiUnusableMemory,
	EfiACPIReclaimMemory,
	EfiACPIMemoryNVS,
	EfiMemoryMappedIO,
	EfiMemoryMappedIOPortSpace,
	EfiPalCode,
	EfiPersistentMemory,
	EfiUnacceptedMemoryType,
	EfiMaxMemoryType
} efi_memory_type_e;

typedef enum {
	AllocateAnyPages,
	AllocateMaxAddress,
	AllocateAddress,
	MaxAllocateType
} efi_allocate_type_e;

typedef struct {
	uint32_t Type;
	efi_physical_address_t PhysicalStart;
	efi_virtual_address_t VirtualStart;
	uint64_t NumberOfPages;
	uint64_t Attribute;
} efi_memory_descriptor_t;

typedef void *efi_handle_t;

typedef struct {
	uint32_t Data1;
	uint16_t Data2;
	uint16_t Data3;
	uint8_t Data4[8];
} efi_guid_t;

typedef struct {
	char _padding1[56];
	efi_status_t (*GetMemoryMap)(uintn_t *, efi_memory_descriptor_t *,
			uintn_t *, uintn_t *, uint32_t *);
	efi_status_t (*AllocatePool)(efi_memory_type_e, uintn_t, void **);
	efi_status_t (*FreePool)(void **);
	char _padding2[152];
	efi_status_t (*ExitBootServices)(const efi_handle_t *,
			const uintn_t);
	char _padding3[80];
	efi_status_t (*LocateProtocol)(efi_guid_t *, void *, void **);
} efi_boot_services_t;

typedef struct {
	char _padding1[60];
	efi_simple_text_output_protocol_t *ConOut;
	char _padding2[16];
	efi_runtime_services_t *RuntimeServices;
	efi_boot_services_t *BootServices;
} efi_system_table_t;

#define EFI_SUCCESS (0)
#define EFI_INVALID_PARAMETER (0x8000000000000002)
#define EFI_BUFFER_TOO_SMALL (0x8000000000000005)

#define EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID { \
	0x9042a9de, 0x23dc, 0x4a38, \
	{ 0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a } \
}

typedef enum {
	PixelRedGreenBlueReserved8BitPerColor,
	PixelBlueGreenRedReserved8BitPerColor,
	PixelBitMask,
	PixelBltOnly,
	PixelFormatMax
} efi_graphics_pixel_format_e;

typedef struct {
	uint32_t Version;
	uint32_t HorizontalResolution;
	uint32_t VerticalResolution;
	efi_graphics_pixel_format_e PixelFormat;
	char _padding1[16];
	uint32_t PixelsPerScanLine;
} efi_graphics_output_mode_information_t;

typedef struct {
	uint32_t MaxMode;
	uint32_t Mode;
	efi_graphics_output_mode_information_t *Info;
	uintn_t SizeOfInfo;
	efi_physical_address_t FrameBufferBase;
	uintn_t FrameBufferSize;
} efi_graphics_output_protocol_mode_t;

typedef struct efi_graphics_output_protcol {
	char _padding1[24];
	efi_graphics_output_protocol_mode_t *Mode;
} efi_graphics_output_protocol_t;

#endif
