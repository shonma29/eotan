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
#define SVC_THREAD_END_AND_DESTROY (1)
#define SVC_THREAD_GET_ID (2)
#define SVC_TIME_SET (3)
#define SVC_TIME_GET (4)
#define SVC_THREAD_DELAY (5)
#define SVC_INTERRUPT_BIND (6)
#define SVC_GET_SYSTEM_INFO (7)
#define SVC_MPU_COPY_STACK (8)
#define SVC_MPU_SET_CONTEXT (9)
#define SVC_PORT_CREATE (10)
#define SVC_PORT_CREATE_AUTO (11)
#define SVC_PORT_DESTROY (12)
#define SVC_PORT_CALL (13)
#define SVC_PORT_ACCEPT (14)
#define SVC_PORT_REPLY (15)

#endif
