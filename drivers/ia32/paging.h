#ifndef __MPU_PAGING_H__
#define __MPU_PAGING_H__ 1
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

#define PAGE_ADDR_MASK 0xfffff000
#define PAGE_BIG_ADDR_MASK 0xffc00000

#define PAGE_BIG_PAT 0x1000
#define PAGE_GLOBAL 0x0100
#define PAGE_BIG 0x80
#define PAGE_PAT 0x80
#define PAGE_DIRTY 0x40
#define PAGE_ACCESS 0x20
#define PAGE_CACHE_DISABLED 0x10
#define PAGE_WRITE_THROUGH 0x08
#define PAGE_USER 0x04
#define PAGE_WRITABLE 0x02
#define PAGE_PRESENT 0x1

#endif
