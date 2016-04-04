#ifndef __ARCH_PSAUX_H__
#define __ARCH_PSAUX_H__ 1
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

/* commands */
#define AUX_RESET 0xff
#define AUX_RESEND 0xfe
#define AUX_SET_DEFAULT 0xf6
#define AUX_DISABLE 0xf5
#define AUX_ENABLE 0xf4
#define AUX_SET_RATE 0xf3
#define AUX_READ_TYPE 0xf2
#define AUX_SET_REMOTE 0xf0
#define AUX_SET_WRAP 0xee
#define AUX_RESET_WRAP 0xec
#define AUX_READ_DATA 0xeb
#define AUX_SET_STREAM 0xea
#define AUX_REQUEST_STATUS 0xe9
#define AUX_SET_RESOLUTION 0xe8
#define AUX_SET_SCALING 0xe7
#define AUX_RESET_SCALING 0xe6

/* result */
#define AUX_ACK 0xfa
#define AUX_RESULT_OK 0xaa
#define AUX_RESULT_NG 0xfc
#define AUX_RESULT_MOUSE 0x00

#endif
