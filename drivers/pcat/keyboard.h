#ifndef __ARCH_KEYBOARD_H__
#define __ARCH_KEYBOARD_H__ 1
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

/* reset */
#define KBD_ACK 0xfa
#define KBD_BAT_OK 0xaa
#define KBD_BAT_NG 0xfc
#define KBD_BREAK_PREFIX 0xf0
#define KBD_ECHO 0xee
#define KBD_OVERRUN1 0x00
#define KBD_OVERRUN2 0xff
#define KBD_RESEND 0xfe
#define KBD_DEFAULT 0xf5

/* commands */
#define KBD_ENABLE 0xf4
#define KBD_READ_ID 0xf2
#define KBD_RESET 0xff
#define KBD_SET_ALL_TYPEMATIC 0xf7
#define KBD_SET_ALL_MAKE_BREAK 0xf8
#define KBD_SET_ALL_MAKE 0xf9
#define KBD_SET_ALL_TYPEMATIC_MAKE_BREAK 0xfa
#define KBD_SET_DEFAULT 0xf6
#define KBD_SET_KEY_TYPEMATIC 0xfb
#define KBD_SET_KEY_MAKE_BREAK 0xfc
#define KBD_SET_KEY_MAKE 0xfd
#define KBD_SET_LED 0xed
#define KBD_SET_DELAY 0xf3

#endif
