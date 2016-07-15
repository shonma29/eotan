#if 0
typedef struct {
	list_t brothers;
	ID thread_id;
	size_t offset;
	size_t rest;
	unsigned char *buf;
	fs_request *request;
} caller_t;
typedef struct {
	node_t node;
	ID dd;
	unsigned char *buf;
	size_t size;
	union {
		struct {
			unsigned short read;
			unsigned short write;
		} s;
		unsigned int w;
	} pos;
	caller_t readers;
	caller_t writers;
} pipe_t;

open()
{
	palloc
}
close()
{
	release readers;
	release writers;
	pfree
}
read()
{
	// if you are head
		// if buf is many
			// read
			// reply
			// deal with brothers
		// else
			// read
			// link to brothers
		// deal with writers
		// if status is changed
			// loop
	// else
		// link to brothers
	// sleep
}
write()
{
	// if you are head
		// if buf is many
			// write
			// reply
			// deal with brothers
		// else
			// write
			// link to brothers
		// deal with readers
		// if status is changed
			// loop
	// else
		// link to brothers
	// sleep
}
#endif
