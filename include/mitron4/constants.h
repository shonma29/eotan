#ifndef _MITRON4_CONSTANTS_H_
#define _MITRON4_CONSTANTS_H_
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
#include <mitron4/types.h>

/**
 * object attributes
 */
#define TA_HLNG 0x00
#define TA_ASM 0x01

#define TA_TFIFO 0x00
#define TA_TPRI 0x01

/**
 * object status
 */
#define TTS_RUN 0x01
#define TTS_RDY 0x02
#define TTS_WAI 0x04
#define TTS_SUS 0x08
#define TTS_WAS 0x0c
#define TTS_DMT 0x10

/**
 * others
 */
#define TSK_SELF (0)
#define TSK_NONE (0)

#define TPRI_SELF (0)
#define TPRI_INI (0)


typedef struct
{
  H	utime;
  UW	ltime;
} systime_t;

#endif
