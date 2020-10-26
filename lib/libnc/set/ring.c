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
#include <string.h>
#include <set/ring.h>

static inline ring_pos_t cutdown(const ring_t *ring, const ring_pos_t n)
{
	return ((n >= ring->size) ? 0 : n);
}

static inline size_t min(const size_t a, const size_t b)
{
	return ((a < b) ? a : b);
}


ring_t *ring_create(void *buf, const size_t size)
{
	if (size <= sizeof(ring_t) + sizeof(ring_chr_t))
		return NULL;

	ring_t *ring = (ring_t *) buf;
	ring->read = 0;
	ring->write = 0;
	ring->size = size - sizeof(*ring);
	return ring;
}

size_t ring_get_rest(const ring_t *ring)
{
	size_t len;

	if (ring->read <= ring->write)
		len = ring->size - ring->write + ring->read - 1;
	else
		len = ring->read - ring->write - 1;

	return len;
}

size_t ring_get(ring_t *ring, ring_chr_t *buf, const size_t size)
{
	bool last = false;
	size_t rest = size;
	for (size_t len; rest;) {
		if (ring->read <= ring->write) {
			len = ring->write- ring->read;
			last = true;
		} else
			len = ring->size - ring->read;

		if (!len)
			break;

		len = min(len, rest);
		memcpy(&(buf[size - rest]), &(ring->buf[ring->read]), len);
		ring->read = cutdown(ring, ring->read + len);
		rest -= len;

		if (last)
			break;
	}

	return (size - rest);
}

size_t ring_put(ring_t *ring, const ring_chr_t *buf, const size_t size)
{
	bool last = false;
	size_t rest = size;
	for (size_t len; rest;) {
		if (ring->read <= ring->write) {
			len = ring->size - ring->write;
			if (!ring->read) {
				len--;
				last = true;
			}
		} else {
			len = ring->read - ring->write - 1;
			last = true;
		}

		if (!len)
			break;

		len = min(len, rest);
		memcpy(&(ring->buf[ring->write]), &(buf[size - rest]), len);
		ring->write = cutdown(ring, ring->write + len);
		rest -= len;

		if (last)
			break;
	}

	return (size - rest);
}
