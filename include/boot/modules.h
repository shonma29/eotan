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
#ifndef __MODULES_H__
#define __MODULES_H__ 1

#define MEMORY_INFO_END 0x00007000
#define MEMORY_INFO_ADDR 0x00007004

#define MODULES_ADDR 0x0000a000

enum ModuleType
{
	mod_end = 0,
	mod_kernel = 1,
	mod_server = 2,
	mod_user = 3,
	mod_initrd = 4
};

typedef struct _ModuleHeader
{
	enum ModuleType type;
	size_t length;
	size_t bytes;
	size_t zBytes;
} ModuleHeader;

typedef struct
{
	unsigned int baseLow;
	unsigned int baseHigh;
	unsigned int sizeLow;
	unsigned int sizeHigh;
	unsigned int type;
} MemoryInfo;

#endif
