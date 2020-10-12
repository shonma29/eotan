#ifndef __ARCH_ARCHFUNC_H__
#define __ARCH_ARCHFUNC_H__	1
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
#include <core.h>
#include <interrupt.h>
#include <stddef.h>
#include <sys/types.h>

/* memory_map.c */
extern size_t get_max_address(void);
extern void set_reserved_pages(void);

/* 8259a.c */
extern void pic_initialize(void);
extern ER pic_reset_mask(const UB ir);

/* pic_mask_all.c */
extern void pic_mask_all(void);

/* rtc.c */
extern void rtc_get_time(time_t *seconds);

/* 8254.c */
extern ER pit_initialize(const UW freq);

/* 8042.c */
extern ER kbc_initialize(void);

#define arch_initialize pic_initialize

/* keyboard.c */
extern void keyboard_interrupt(VP_INT);

/* mouse.c */
extern void mouse_interrupt(VP_INT);

/* pci.c */
void peripheral_set_map(void);

#endif
