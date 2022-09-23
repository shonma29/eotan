#ifndef _NSCM_PARSE_H_
#define _NSCM_PARSE_H_ 1

#include "nscm.h"

#define PEEK_SIZE (4)
#define PEEK_MASK (PEEK_SIZE - 1)

#define PEEK_EMPTY (-2)

typedef enum {
	TOKEN_UNKNOWN,
	TOKEN_LPAREN,
	TOKEN_RPAREN,
	TOKEN_QUOTE,
	TOKEN_DOT,
	TOKEN_VALUE
} token_type_e;

typedef struct _token {
	FILE *in;
	bool peekable;
	token_type_e type;
	const cell_t *value;
	long line;
	long comment_depth;
	long peek_read_pos;
	long peek_write_pos;
	int peek_buf[PEEK_SIZE];
	char buf[BUFSIZ];//TODO use malloc
} token_t;

static inline long peek_next_pos(const long pos)
{
	return ((pos + 1) & PEEK_MASK);
}

extern const cell_t *load(token_t *);
extern void preprocess_initialize(void);

#endif