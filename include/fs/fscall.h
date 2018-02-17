#ifndef _FS_FSCALL_H_
#define _FS_FSCALL_H_
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

enum FsCall {
	fscall_fork = 0,
	fscall_waitpid = 1,
	fscall_exec = 2,
	fscall_exit = 3,
	fscall_kill = 4,
	fscall_chdir = 5,
	fscall_create = 6,
	fscall_remove = 7,
	fscall_fstat = 8,
	fscall_chmod = 9,
	fscall_dup2 = 10,
	fscall_open = 11,
	fscall_lseek = 12,
	fscall_read = 13,
	fscall_write = 14,
	fscall_close = 15,
	fscall_link = 16,
	fscall_mount = 17,
	fscall_unmount = 18
};

#endif
