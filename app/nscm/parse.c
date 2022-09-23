#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "nscm.h"
#include "parse.h"

static const cell_t *token_parse_value(token_t *);
static const cell_t *token_parse_list(token_t *);
static void token_get_token(token_t *);
static int token_getc(token_t *);
static void token_get_until(token_t *, const char);
static void token_get_string(token_t *, const char);
static void skip_comment_block(token_t *);
static int peek_get(token_t *);
static long peek_len(token_t *);
static int peek_put(token_t *, const int);
static const cell_t *preprocess(cell_t *);

const cell_t *load(token_t *token)
{
	token_get_token(token);
	return token_parse_value(token);
}

static const cell_t *token_parse_value(token_t *token)
{
	switch (token->type) {
	case TOKEN_LPAREN:
		return preprocess((cell_t *) token_parse_list(token));
	case TOKEN_QUOTE:
		token_get_token(token);
		return create_pair(create_symbol("quote"),
				create_pair(token_parse_value(token), null_cell));
	case TOKEN_VALUE:
		return token->value;
	default:
		throw("cannot read", NULL);
	}
}

static const cell_t *token_parse_list(token_t *token)
{
	token_get_token(token);
	switch (token->type) {
	case TOKEN_RPAREN:
		return null_cell;
	case TOKEN_DOT: {
			token_get_token(token);
			const cell_t *next = token_parse_value(token);
			token_get_token(token);
			if (token->type != TOKEN_RPAREN)
				throw("bad dot", NULL);

			return next;
		}
	default:
		return create_pair(token_parse_value(token), token_parse_list(token));
	}
}

static void token_get_token(token_t *token)
{
	int c;
	do {
restart:
		c = token_getc(token);
		switch (c) {
		case EOF:
			exit(EXIT_SUCCESS);
		case ';':
			do {//TODO should suspend? when find #
				c = token_getc(token);
				if (c == EOF)
					exit(EXIT_SUCCESS);
			} while (c != LF);
			break;
		case '|':
			token_get_until(token, '|');//TODO should suspend? when find #;
			token->type = TOKEN_VALUE;
			token->value = create_symbol(token->buf);
			return;
		case '"':
			token_get_string(token, '"');//TODO should suspend? when find #;
			token->type = TOKEN_VALUE;
			token->value = create_string(token->buf);//TODO save to constant hash?
			return;
		case '#':{
				int c2 = token_getc(token);
				if (c2 == '|') {
					skip_comment_block(token);
					goto restart;
				} else
					peek_put(token, c2);
			}
			break;
		default:
			break;
		}
	} while (isspace(c));//TODO skip 0?

	switch (c) {
	case '(':
		token->type = TOKEN_LPAREN;
		break;
	case ')':
		token->type = TOKEN_RPAREN;
		break;
	case '\'':
		token->type = TOKEN_QUOTE;
		break;
	default:{
			long pos = 0;
			token->buf[pos++] = c;//TODO use pointer

			while (((c = token_getc(token)) != EOF)//TODO skip 0?
					&& (!isspace(c))
					&& (pos < sizeof(token->buf))//TODO realloc buf
					&& (c != ';')//TODO use map
					&& (c != '(')
					&& (c != ')'))//TODO #|"' are delimiters?
				token->buf[pos++] = c;

			token->buf[pos] = NUL;
			token->type = TOKEN_VALUE;
			peek_put(token, c);

			if (!strcmp(token->buf, ".")) {
				token->type = TOKEN_DOT;
				break;
			}

			char *end;
			//TODO check errno
			long integer = strtol(token->buf, &end, 10);
			if (token->buf[0] && !(*end))
				token->value = create_integer(integer);
			else {
				//TODO symbol must not start with '[0-9]'
#ifdef USE_FLONUM
				//TODO check errno
				double flonum = strtod(token->buf, &end);
				if (token->buf[0] && !(*end)) {
					token->value = create_flonum(flonum);
				} else
#endif
					token->value = create_symbol(token->buf);
			}
		}
	}
}

static int token_getc(token_t *token)
{
	int c;
	if (token->peekable) {
		c = peek_get(token);
		if (c == PEEK_EMPTY)
			c = fgetc(token->in);
	} else
		c = fgetc(token->in);

	if (c == LF)
		token->line++;

	return c;
}

static void token_get_until(token_t *token, const char last)
{
	long pos = 0;
	for (;;) {
		int c = token_getc(token);
		if (c == EOF)
			exit(EXIT_SUCCESS);
		else if (c == last)
			break;

		token->buf[pos++] = c;
	}

	token->buf[pos] = NUL;
}

static void token_get_string(token_t *token, const char last)
{
	bool escaping = false;
	long pos = 0;
	for (;;) {
		int c = token_getc(token);
		if (c == EOF)
			exit(EXIT_SUCCESS);

		if (escaping) {
			escaping = false;
			switch (c) {
			case 'a':
				c = 0x7;
				break;
			case 'b':
				c = 0x8;
				break;
			case 't':
				c = 0x9;
				break;
			case 'n':
				c = 0xa;
				break;
			case 'r':
				c = 0xd;
				break;
			default:
				//TODO process escape *, x;
				break;
			}
		} else {
			if (c == '\\') {
				escaping = true;
				continue;
			} else if (c == last)
				break;
		}

		token->buf[pos++] = c;
	}

	token->buf[pos] = NUL;
}

//TODO should ignore? ;"|
static void skip_comment_block(token_t *token)
{
	token->comment_depth = 1;

	int c;
	do {
		c = token_getc(token);
		switch (c) {
		case '#':
			c = token_getc(token);
			if (c == '|') {
				token->comment_depth++;
				if (token->comment_depth <= 0)
					throw("comment depth overflow", NULL);
			}
			break;
		case '|':
			c = token_getc(token);
			if (c == '#') {
				token->comment_depth--;
				if (!(token->comment_depth))
					return;
			}
			break;
		default:
			break;
		}
	} while (c != EOF);

	exit(EXIT_SUCCESS);
}

static int peek_get(token_t *token)
{
	int c;
	if (peek_len(token) > 0) {
		c = token->peek_buf[token->peek_read_pos];
		token->peek_read_pos = peek_next_pos(token->peek_read_pos);
	} else {
		c = PEEK_EMPTY;
		token->peekable = false;
	}

	return c;
}

static long peek_len(token_t *token)
{
	return ((token->peek_write_pos - token->peek_read_pos)
			+ ((token->peek_read_pos <= token->peek_write_pos) ?
					0 : (sizeof(token->peek_buf) / sizeof(token->peek_buf[0]))));
}

static int peek_put(token_t *token, const int c)
{
	long next_pos = peek_next_pos(token->peek_write_pos);
	if (next_pos == token->peek_read_pos)
		throw("peek overflow", NULL);

	token->peek_buf[token->peek_write_pos] = c;
	token->peek_write_pos = next_pos;
	token->peekable = true;
	return 0;
}

#ifdef USE_DERIVED
static const cell_t *symbol_define;
static const cell_t *symbol_begin;
static const cell_t *symbol_and;
static const cell_t *symbol_or;
static const cell_t *symbol_let;
static const cell_t *symbol_lambda;
static const cell_t *symbol_if;
static const cell_t *symbol_v;
#endif

void preprocess_initialize(void)
{
#ifdef USE_DERIVED
	symbol_define = create_symbol("define");
	protect_cell((cell_t *) symbol_define);
	symbol_begin = create_symbol("begin");
	protect_cell((cell_t *) symbol_begin);
	symbol_and = create_symbol("and");
	protect_cell((cell_t *) symbol_and);
	symbol_or = create_symbol("or");
	protect_cell((cell_t *) symbol_or);
	symbol_let = create_symbol("let");
	protect_cell((cell_t *) symbol_let);
	symbol_lambda = create_symbol("lambda");
	protect_cell((cell_t *) symbol_lambda);
	symbol_if = create_symbol("if");
	protect_cell((cell_t *) symbol_if);
	symbol_v = create_symbol("v");
	protect_cell((cell_t *) symbol_v);
#endif
}

//TODO split to other file
static const cell_t *preprocess(cell_t *list)
{
#ifdef USE_DERIVED
	long len = list_length(list);
	if (len <= 0)
		return list;

	const cell_t *head = car(list);
	if (tag_of(head) != CELL_SYMBOL)
		return list;

	if (head == symbol_define) {
		if (len >= 3) {
			cell_t *second = (cell_t *) cdr(list);
			cell_t *third = (cell_t *) car(second);
			if (list_length(third) > 0) {
				const cell_t *body = cdr(second);
				second->first = car(third);
				second->rest = third;
				third->first = create_pair(symbol_lambda,
						create_pair(cdr(third), body));
				third->rest = null_cell;
			}
		}
	} else if (head == symbol_begin) {
		if (len < 2)
			//TODO skip when not match
			//TODO really required 1?
			throw("requires 1 argument, but 0", list);//TODO sprintf?validate_length?

		list->first = null_cell;
		list = (cell_t *) create_pair(
				create_pair(symbol_lambda, list),
				null_cell);
	} else if (head == symbol_and) {
		if (len == 1)
			return true_cell;//TODO destroy pair

		if (len == 2)
			return cadr(list);//TODO destroy pair

		cell_t *dest = NULL;
		cell_t *source = list;
		for (; len > 3; len--) {
			cell_t *firstPair = (cell_t *) cdr(source);
			source->rest = cdr(firstPair);

			const cell_t *nextDest = create_pair(source,
					create_pair(false_cell, null_cell));
			firstPair->rest = nextDest;
			const cell_t *body = create_pair(symbol_if, firstPair);

			if (dest)
				dest->first = body;
			else
				list = (cell_t *) body;

			dest = (cell_t *) nextDest;
		}

		source->first = symbol_if;
		((cell_t *) cdr(cdr(source)))->rest =
				create_pair(false_cell, null_cell);
	} else if (head == symbol_or) {
		if (len == 1)
			return false_cell;//TODO destroy pair

		if (len == 2)
			return cadr(list);//TODO destroy pair

		cell_t *dest = NULL;
		cell_t *source = list;
		for (; len > 3; len--) {
			cell_t *firstPair = (cell_t *) cdr(source);
			source->rest = cdr(cdr(source));
			firstPair->rest = null_cell;

			const cell_t *nextDest = create_pair(source, null_cell);
			const cell_t *body = create_pair(
					create_pair(symbol_lambda,
							create_pair(create_pair(symbol_v, null_cell),
									create_pair(
											create_pair(symbol_if,
													create_pair(symbol_v,
															create_pair(symbol_v,
																	nextDest))),
											null_cell))),
					firstPair);

			if (dest)
				dest->first = body;
			else
				list = (cell_t *) body;

			dest = (cell_t *) nextDest;
		}

		source->first = create_pair(symbol_lambda,
				create_pair(create_pair(symbol_v, null_cell),
						create_pair(
								create_pair(symbol_if,
										create_pair(symbol_v,
												create_pair(symbol_v,
														cdr(cdr(source))))),
								null_cell)));

		cell_t *last = (cell_t *) cdr(source);
		last->rest = null_cell;
		source->rest = last;
	} else if (head == symbol_let) {
		if (len < 3)
			//TODO really requires 2?
			throw("requires 2 argument, but 0", list);//TODO sprintf?validate_length?

		const cell_t *vars = cadr(list);
		if (list_length(vars) <= 0)
			//TODO really requires 1?
			throw("requires 1 argument, but 0", vars);//TODO sprintf?validate_length?

		const cell_t *keyHead = null_cell;
		const cell_t *valueHead = null_cell;
		cell_t *keys = NULL;
		cell_t *values = NULL;
		//TODO check same name
		for (const cell_t *p = vars; !is_null(p); p = cdr(p)) {
			const cell_t *q = car(p);//TODO destroy outer pairs
			if (list_length(q) != 2)
				//TODO really requires 2?
				throw("requires 2 argument, but 0", q);//TODO sprintf?validate_length?

			if (is_null(keyHead)) {
				keyHead = keys = (cell_t *) q;
				valueHead = values = (cell_t *) cdr(q);
				keys->rest = null_cell;
			} else {
				keys->rest = q;
				keys = (cell_t *) (keys->rest);
				values->rest = cdr(q);
				values = (cell_t *) (values->rest);
				keys->rest = null_cell;
			}
		}

		list->first = symbol_lambda;
		((cell_t *) cdr(list))->first = keyHead;
		list = (cell_t *) create_pair(list, valueHead);
	}
#endif
	return list;
}
