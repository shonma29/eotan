#ifndef _MITRON4_CONFIG_H_
#define _MITRON4_CONFIG_H_
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
#include <mpu/limits.h>
#include <nerve/config.h>

/**
 * priority
 */
#define TMIN_TPRI (1)
#define TMAX_TPRI (MAX_PRIORITY)

#define TMIN_MPRI (TMIN_TPRI)
#define TMAX_MPRI (TMAX_TPRI)

/**
 * version
 */
#define TKERNEL_MAKER (0x0000)
#define TKERNEL_PRID (0x0000)
#define TKERNEL_SPVER (0x5403)
#define TKERNEL_PRVER (0x0000)

/**
 * nest
 */
#define TMAX_ACTCNT (0)
#define TMAX_WUPCNT (MAX_WAKEUP_COUNT)
#define TMAX_SUSCNT (0)

/**
 * bit pattern
 */
#define TBIT_TEXPTN (INT_BIT)
#define TBIT_FLGPTN (INT_BIT)
#define TBIT_RDVPTN (INT_BIT)

/**
 * tick
 */
#define TIC_NUME (1)
#define TIC_DENO (TIME_TICKS)

#endif
