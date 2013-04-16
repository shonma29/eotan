#ifndef _NERVE_SVCNO_H_
#define _NERVE_SVCNO_H_
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

/**
 * service call no
 */
#define SVC_THREAD_CREATE (1)
#define SVC_THREAD_DESTROY (2)
#define SVC_THREAD_START (3)
#define SVC_THREAD_END (4)
#define SVC_THREAD_END_AND_DESTROY (5)
#define SVC_THREAD_TERMINATE (6)
#define SVC_THREAD_CHANGE_PRIORITY (7)
#define SVC_THREAD_RELEASE (8)
#define SVC_THREAD_GET_ID (9)
#define SVC_THREAD_SUSPEND (10)
#define SVC_THREAD_RESUME (11)
#define SVC_FLAG_CREATE_AUTO (12)
#define SVC_FLAG_DESTROY (13)
#define SVC_FLAG_SET (14)
#define SVC_FLAG_CLEAR (15)
#define SVC_FLAG_WAIT (16)
#define SVC_TIME_SET (17)
#define SVC_TIME_GET (18)
#define SVC_THREAD_DELAY (19)
#define SVC_ALARM_CREATE (20)
#define SVC_INTERRUPT_BIND (21)
#define SVC_GET_SYSTEM_INFO (22)
#define SVC_KERNLOG (23)
#define SVC_REGION_CREATE (24)
#define SVC_REGION_DESTROY (25)
#define SVC_REGION_MAP (26)
#define SVC_REGION_UNMAP (27)
#define SVC_REGION_DUPLICATE (28)
#define SVC_REGION_PUT (29)
#define SVC_REGION_GET (30)
#define SVC_REGION_GET_STATUS (31)
#define SVC_GET_PHYSICAL_ADDRESS (32)
#define SVC_MISC (33)
#define SVC_MPU_COPY_STACK (34)
#define SVC_MPU_SET_CONTEXT (35)
#define SVC_MPU_USE_FLOAT (36)
#define SVC_PORT_CREATE (37)
#define SVC_PORT_CREATE_AUTO (38)
#define SVC_PORT_DESTROY (39)
#define SVC_PORT_CALL (40)
#define SVC_PORT_ACCEPT (41)
#define SVC_PORT_REPLY (42)

#endif
