#ifndef __BOOT_VESA_H__
#define __BOOT_VESA_H__
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
#define VESA_INFO_ADDR 0x00006c00

typedef struct
{
	unsigned short mode_attr;
	unsigned char window_a_attr;
	unsigned char window_b_attr;
	unsigned short window_granularity;
	unsigned short window_size;
	unsigned short window_a_segment;
	unsigned short window_b_segment;
	unsigned int function;
	unsigned short bytes_per_line;
	/* option VESA 1.0/1.1 */
	unsigned short width;
	unsigned short height;
	unsigned char chr_width;
	unsigned char chr_height;
	unsigned char num_of_planes;
	unsigned char bits_per_pixel;
	unsigned char num_of_banks;
	unsigned char memory_model;
	unsigned char size_of_bank;
	unsigned char num_of_images;
	unsigned char reserved1;
	/* VBE 1.2 */
	unsigned char red_size;
	unsigned char red_position;
	unsigned char green_size;
	unsigned char green_position;
	unsigned char blue_size;
	unsigned char blue_position;
	unsigned char reserved_size;
	unsigned char reserved_position;
	unsigned char direct_color_mode;
	/* VBE 2.0 */
	unsigned int buffer_addr;
	unsigned int offscreen_position;
	unsigned short offscreen_size;
	/* VBE 3.0 */
	unsigned short bytes_per_scan_line_linear;
	unsigned char num_of_images_banked;
	unsigned char num_of_images_linear;
	unsigned char red_size_linear;
	unsigned char red_position_linear;
	unsigned char green_size_linear;
	unsigned char green_position_linear;
	unsigned char blue_size_linear;
	unsigned char blue_position_linear;
	unsigned char reserved_size_linear;
	unsigned char reserved_position_linear;
	unsigned int max_clock;
	unsigned char reserved2[190];
} VesaInfo;

#endif
