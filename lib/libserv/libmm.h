#ifndef _LIBSERV_LIBMM_H_
#define _LIBSERV_LIBMM_H_
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
#include <mm/segment.h>

extern int process_create(ID pid, VP addr, size_t initial_len, VP heap_end);
extern int process_destroy(ID pid);
extern int process_duplicate(ID src_process, ID dest_process);
extern int process_copy_stack(ID src_thread, W esp, ID dest_thread);
extern int process_set_context(ID tid, W eip, B *stack, W stacksz);
extern int vmap(ID pid, VP addr, UW len, W attr);
extern int vunmap(ID pid, VP addr, UW len);
extern ER_ID thread_create(ID pid, FP entry);

#endif
