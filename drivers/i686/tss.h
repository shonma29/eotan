#ifndef __MPU_TSS_H__
#define __MPU_TSS_H__ 1
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

typedef struct {
	unsigned short back;
	unsigned short pad1;
	unsigned long esp0;
	unsigned short ss0;
	unsigned short pad2;
	unsigned long esp1;
	unsigned short ss1;
	unsigned short pad3;
	unsigned long esp2;
	unsigned short ss2;
	unsigned short pad4;
	unsigned long cr3;
	unsigned long eip;
	unsigned long eflags;
	unsigned long eax;
	unsigned long ecx;
	unsigned long edx;
	unsigned long ebx;
	unsigned long esp;
	unsigned long ebp;
	unsigned long esi;
	unsigned long edi;
	unsigned short es;
	unsigned short pad5;
	unsigned short cs;
	unsigned short pad6;
	unsigned short ss;
	unsigned short pad7;
	unsigned short ds;
	unsigned short pad8;
	unsigned short fs;
	unsigned short pad9;
	unsigned short gs;
	unsigned short pad10;
	unsigned short ldt;
	unsigned short pad11;
	unsigned short t;
	unsigned short iomap;
} tss_t;

#endif
