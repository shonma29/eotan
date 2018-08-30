#ifndef _FS_TFS_H_
#define _FS_TFS_H_
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
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

typedef uint32_t blkno_t;

#define TFS_MAGIC 0x30465374

#define TFS_RESERVED_BLOCKS (2)

#define TFS_MAXNAMLEN (255)
#define TFS_MINNAMLEN (sizeof(uint32_t) - sizeof(uint8_t))

static inline size_t num_of_1st_blocks(const blksize_t blksize)
{
	return (blksize - sizeof(struct sfs_inode)) / sizeof(uint32_t);
}

static inline size_t num_of_2nd_blocks(const blksize_t blksize)
{
	return blksize / sizeof(uint32_t);
}

static inline size_t real_name_len(const size_t n)
{
	return n + (TFS_MINNAMLEN - (n & TFS_MINNAMLEN));
}

struct tfs_dir {
	uint32_t d_fileno;
	uint8_t d_namlen;
	uint8_t d_name[TFS_MINNAMLEN];
};

#define TFS_MINDIRSIZE (sizeof(struct tfs_dir) * 2)

#endif