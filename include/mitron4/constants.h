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
#include <stdbool.h>
#include <stddef.h>

/**
 * common constants
 */
#ifndef NULL
#define NULL (0)
#endif

#ifndef TRUE
#define TRUE (true)
#endif
#ifndef FALSE
#define FALSE (false)
#endif

#define E_OK (0)

/**
 * object attributes
 */
#define TA_NULL (0)

#define TA_HLNG (0x00)
#define TA_ASM (0x01)

#define TA_TFIFO (0x00)
#define TA_TPRI (0x01)

#define TA_MFIFO (0x00)
#define TA_MPRI (0x02)

#define TA_ACT (0x02)
#define TA_RSTR (0x04)

#define TA_WSGL (0x00)
#define TA_WMUL (0x02)
#define TA_CLR (0x04)

#define TA_INHERIT (0x02)
#define TA_CEILING (0x03)

#define TA_STA (0x02)
#define TA_PHS (0x04)

/**
 * timeout parameters
 */
#define TMO_POL (0)
#define TMO_FEVR (-1)
#define TMO_NBLK (-2)

/**
 * operation mode
 */
#define TWF_ANDW (0x00)
#define TWF_ORW (0x01)

/**
 * object status
 */
#define TTS_RUN (0x01)
#define TTS_RDY (0x02)
#define TTS_WAI (0x04)
#define TTS_SUS (0x08)
#define TTS_WAS (TTS_WAI | TTS_SUS)
#define TTS_DMT (0x10)

#define TTW_SLP (0x0001)
#define TTW_DLY (0x0002)
#define TTW_SEM (0x0004)
#define TTW_FLG (0x0008)
#define TTW_SDTQ (0x0010)
#define TTW_RDTQ (0x0020)
#define TTW_MBX (0x0040)
#define TTW_MTX (0x0080)
#define TTW_SMBF (0x0100)
#define TTW_RMBF (0x0200)
#define TTW_CAL (0x0400)
#define TTW_ACP (0x0800)
#define TTW_RDV (0x1000)
#define TTW_MPF (0x2000)
#define TTW_MPL (0x4000)

#define TTEX_ENA (0x00)
#define TTEX_DIS (0x01)

#define TCYC_STP (0x00)
#define TCYC_STA (0x01)

#define TALM_STP (0x00)
#define TALM_STA (0x01)

#define TOVR_STP (0x00)
#define TOVR_STA (0x01)

/**
 * others
 */
#define TSK_SELF (0)
#define TSK_NONE (0)

#define TPRI_SELF (0)
#define TPRI_INI (0)

#endif
