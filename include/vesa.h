#ifndef __VESA_H__
#define __VESA_H__
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
#include <stdint.h>

#define VESA_INFO_ADDR 0x00003c00

typedef struct
{
	uint16_t mode_attr;
	uint8_t window_a_attr;
	uint8_t window_b_attr;
	uint16_t window_granularity;
	uint16_t window_size;
	uint16_t window_a_segment;
	uint16_t window_b_segment;
	uint32_t function;
	uint16_t bytes_per_line;
	/* option VESA 1.0/1.1 */
	uint16_t width;
	uint16_t height;
	uint8_t chr_width;
	uint8_t chr_height;
	uint8_t num_of_planes;
	uint8_t bits_per_pixel;
	uint8_t num_of_banks;
	uint8_t memory_model;
	uint8_t size_of_bank;
	uint8_t num_of_images;
	uint8_t reserved1;
	/* VBE 1.2 */
	uint8_t red_size;
	uint8_t red_position;
	uint8_t green_size;
	uint8_t green_position;
	uint8_t blue_size;
	uint8_t blue_position;
	uint8_t reserved_size;
	uint8_t reserved_position;
	uint8_t direct_color_mode;
	/* VBE 2.0 */
	uint32_t buffer_addr;
	uint32_t offscreen_position;
	uint16_t offscreen_size;
	/* VBE 3.0 */
	uint16_t bytes_per_scan_line_linear;
	uint8_t num_of_images_banked;
	uint8_t num_of_images_linear;
	uint8_t red_size_linear;
	uint8_t red_position_linear;
	uint8_t green_size_linear;
	uint8_t green_position_linear;
	uint8_t blue_size_linear;
	uint8_t blue_position_linear;
	uint8_t reserved_size_linear;
	uint8_t reserved_position_linear;
	uint32_t max_clock;
	uint8_t reserved2[190];
} VesaInfo;

#endif
