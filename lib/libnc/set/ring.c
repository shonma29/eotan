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
#include <string.h>
#include <set/ring.h>

static inline size_t roundup(const size_t len)
{
	return (len + sizeof(len) - 1) & ~(sizeof(len) - 1);
}

static inline size_t rotate(const ring_pos_t pos, const size_t ceil)
{
	return (pos >= ceil)? (pos - ceil):pos;
}

static inline size_t get_len(ring_t *r)
{
	size_t *p = (size_t*)(r->buf + r->read);

	r->read = rotate(r->read + sizeof(size_t), r->max);

	return *p;
}

static inline void put_len(ring_t *r, const size_t len)
{
	size_t *p = (size_t*)(r->buf + r->write);

	*p = len;
	r->write = rotate(r->write + sizeof(size_t), r->max);

	if (!r->write)
		r->cycle++;
}


ring_t *ring_create(void *buf, size_t size) {
	ring_t *r = (ring_t*)buf;

	if (size <= sizeof(*r) + sizeof(size_t))
		return NULL;

	r->read = 0;
	r->write = 0;
	r->cycle = 0;
	r->max = size - sizeof(*r);
	r->left = r->max;

	return r;
}

int ring_peak_len(ring_t *r)
{
	int *p;

	if (r->write == r->read)
		return RING_EMPTY;

	p = (int*)(r->buf + r->read);

	return *p;
}

int ring_get(ring_t *r, ring_chr_t *buf)
{
	size_t len;
	size_t bytes;
	size_t rbytes;
	ring_pos_t next;

	if (r->write == r->read)
		return RING_EMPTY;

	len = get_len(r);
	bytes = len;
	rbytes = roundup(len);
	next = rotate(r->read + rbytes, r->max);

	if (next < r->read) {
		size_t q = r->max - r->read;

		memcpy(&buf[q], r->buf, len - q);
		len = q;
	}

	memcpy(buf, &r->buf[r->read], len);
	r->read = next;
	r->left += sizeof(size_t) + rbytes;

	return bytes;
}

int ring_put(ring_t *r, const ring_chr_t *buf, const size_t len)
{
	size_t bytes;
	size_t total;
	ring_pos_t next;

	if (!len)
		return 0;

	if (len > RING_MAX_LEN)
		return RING_TOO_LONG;

	total = sizeof(size_t) + roundup(len);

	if (total >= r->left)
		return RING_FULL;

	next = rotate(r->write + total, r->max);
	put_len(r, len);
	bytes = len;

	if (next < r->write) {
		size_t q = r->max - r->write;

		memcpy(r->buf, buf + q, bytes - q);
		r->cycle++;
		bytes = q;
	}

	memcpy(&r->buf[r->write], buf, bytes);
	r->write = next;
	r->left -= total;

	return len;
}
