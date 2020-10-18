/* MonoLis (minimam Lisp interpreter)
written by kenichi sasagawa 2016/1

*/
#include <ctype.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include "nscm.h"

#ifdef USE_FLONUM
#include <math.h>
#endif

#define OFFSET_CALLEE (0)
#define OFFSET_ARGC (1)
#define OFFSET_ARGV (2)

#define USING_BIT (1)
#define MASK_MARK MASK_TAG_OF_POINTER

typedef struct _hash {
	unsigned long (*calc)(const cell_t *);
	long (*compare)(const cell_t *, const cell_t *);
	list_t table[HASH_SIZE];
} hash_t;

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
	bool has_last;
	char last;
	token_type_e type;
	const cell_t *value;
	char buf[BUFSIZ];//TODO use malloc
	long line;
} token_t;

static token_t token = {
	NULL, false, NUL, TOKEN_UNKNOWN, NULL, { NUL }, 1
};
static list_t protect_guard;
static list_t cells_guard;
static list_t free_guard;
static long num_of_cells;
static long num_of_cells_on_last_gc;

//TODO per context
static const cell_t *call_stack[CALL_STACK_SIZE];
static const cell_t **call_top;
static const cell_t *parameter_stack[PARAMETER_STACK_SIZE];
static const cell_t **parameter_top;
#ifdef USE_TRACE
long trace_max_call = 0;
long trace_max_param = 0;
#endif

static hash_t hash_global;
static hash_t hash_symbol;

cell_t *null_cell;
cell_t *unspecified_cell;
cell_t *true_cell;
cell_t *false_cell;


static inline cell_t *getHashParent(const list_t *p)
{
    return ((cell_t *) ((uintptr_t) p - offsetof(cell_t, hash)));
}

static inline cell_t *getCellsParent(const list_t *p)
{
    return ((cell_t *) ((uintptr_t) p - offsetof(cell_t, cells)));
}

static inline long words_of_frame(const long argc)
{
	return (argc + 2);
}

static inline bool is_eq(const cell_t *cell1, const cell_t *cell2)
{
	return (cell1 == cell2);
}

static inline void set_using(cell_t *cell)
{
	cell->tag |= USING_BIT;
}

static inline void unset_using(cell_t *cell)
{
	cell->tag &= ~USING_BIT;
}

static inline long get_using(cell_t *cell)
{
	return (tag_of(cell) & MASK_MARK);
}

static inline list_t *list_head(list_t *guard)
{
	return guard->next;
}

static inline bool list_is_edge(list_t *guard, list_t *p)
{
	return (p == guard);
}

static inline cell_t *create_refer_local(const long offset)
{
	return ((cell_t *) ((offset << FIX_TAG_BITS) | CELL_REFER_LOCAL));
}

static void list_initialize(list_t *);
static void list_insert(list_t *, list_t *);
static void list_remove(list_t *);

static void hash_initialize(hash_t *, unsigned long (*)(const cell_t *),
		long (*)(const cell_t *, const cell_t *));
static unsigned long hash_calc_symbol(const cell_t *);
static long hash_compare_symbol(const cell_t *, const cell_t *);
static long hash_compare_global(const cell_t *, const cell_t *);
static const cell_t *hash_get(hash_t *, const long, const cell_t *);
static void hash_put(hash_t *, const long, cell_t *);
static void hash_remove(cell_t *);

static void destroy_cell(cell_t *);

static cell_t *set_global(const char *, const cell_t *);
static const cell_t *get_value(const cell_t *, const cell_t *);
static long get_index(const cell_t *, const cell_t *);

static cell_t *create_symbol(const char *);
static const cell_t *create_pair(const cell_t *, const cell_t *);
static const cell_t *create_lambda(const cell_t *, const cell_t *);
static cell_t *create_boolean(const bool);

static const cell_t *load(token_t *);
static const cell_t *token_parse_value(token_t *);
static const cell_t *token_parse_list(token_t *);
static void token_get_token(token_t *);
static int token_getc(token_t *);
static void token_get_until(token_t *, const char);

static const cell_t *eval_item(const cell_t *, const cell_t *);
static const cell_t *get_and_replace_symbol(const cell_t *, const cell_t *);
static const cell_t *eval_list(const cell_t *, const cell_t *,
		const bool);
static const cell_t *eval_first(const cell_t *, const cell_t *);
static long eval_rest(const cell_t *, const cell_t *);
static long eval_reverse(const cell_t *, const cell_t *);
static const cell_t *eval_and_replace(const cell_t *, const cell_t *,
		const bool);
static const cell_t *apply(const cell_t *);
static void parameter_to_list(void);
static const cell_t *create_tail(const cell_t *, const cell_t *,
		const cell_t *);

static void push_call(const cell_t *);
static void pop_call(void);
static void push_parameter(const cell_t *);
static void pop_parameter(void);

static void protect_cell(cell_t *);
static void unprotect_cell(cell_t *);
static void gc(void);
static void gc_mark(void);
static void gc_mark_cell(cell_t *);
static void gc_sweep(void);

#ifdef USE_TRACE
static const cell_t *find_symbol_by_procedure(const cell_t *);
#endif
#if !defined(USE_REPL) || defined(USE_TRACE)
static void simple_print(FILE *, const cell_t *);
#endif
static void validate_pair(const cell_t *);

static void import_core(void);

static const cell_t *lib_quote(const cell_t *, const cell_t *);
static const cell_t *lib_lambda(const cell_t *, const cell_t *);
static const cell_t *lib_define(const cell_t *, const cell_t *);
static const cell_t *lib_if(const cell_t *, const cell_t *);
static const cell_t *lib_car(const long, const cell_t **);
static const cell_t *lib_cdr(const long, const cell_t **);
static const cell_t *lib_cons(const long, const cell_t **);
static const cell_t *lib_is_eq(const long, const cell_t **);
static const cell_t *lib_is_pair(const long, const cell_t **);

static void list_initialize(list_t *p)
{
	p->prev = p->next = p;
}

static void list_insert(list_t *guard, list_t *p)
{
	list_t *next = guard->next;
	p->next = next;
	p->prev = guard;
	guard->next = p;
	next->prev = p;
}

static void list_remove(list_t *p)
{
	list_t *next = p->next;
	list_t *prev = p->prev;
	next->prev = prev;
	prev->next = next;
	p->prev = p->next = NULL;
}

static void hash_initialize(hash_t *hash, unsigned long (*calc)(const cell_t *),
		long (*compare)(const cell_t *, const cell_t *))
{
	hash->calc = calc;
	hash->compare = compare;

	for (long i = 0; i < HASH_SIZE; i++)
		list_initialize(&(hash->table[i]));
}

static unsigned long hash_calc_symbol(const cell_t *symbol)
{
	unsigned long value = 0;
	for (unsigned char *p = (unsigned char *) (symbol->name); *p; p++)
		value += *p;

	return (value % HASH_SIZE);
}

static long hash_compare_symbol(const cell_t *cell, const cell_t *symbol)
{
	return strcmp(cell->name, symbol->name);
}

static long hash_compare_global(const cell_t *global, const cell_t *pair)
{
	return ((car(global) == car(pair)) ? 0 : 1);
}

static const cell_t *hash_get(hash_t *hash, const long key, const cell_t *symbol)
{
	list_t *guard = &(hash->table[key]);
	for (list_t *p = list_head(guard); !list_is_edge(guard, p); p = p->next) {
		cell_t *cell = getHashParent(p);
		if (!(hash->compare(cell, symbol)))
			return cell;
	}

	return NULL;
}

static void hash_put(hash_t *hash, const long key, cell_t *cell)
{
	cell_t *head = (cell_t *) hash_get(hash, key, cell);
	if (!head)//TODO is guarded?
		list_insert(&(hash->table[key]), &(cell->hash));
}

static void hash_remove(cell_t *cell)//TODO hash field is not needed? (search and remove)
{
	if (cell->hash.next)//TODO is needed?
		list_remove(&(cell->hash));
}

int main(int argc, char **argv)
{
	list_initialize(&protect_guard);
	list_initialize(&cells_guard);
	list_initialize(&free_guard);
	hash_initialize(&hash_global, hash_calc_symbol, hash_compare_global);
	hash_initialize(&hash_symbol, hash_calc_symbol, hash_compare_symbol);

	import_core();
	IMPORT_BASE();//TODO test duplicate fields in symbol and env
	num_of_cells_on_last_gc = num_of_cells;
#ifdef USE_TRACE
	fprintf(stderr, "TRACE: num_of_cells=%ld\n", num_of_cells);
#endif

	call_top = &(call_stack[CALL_STACK_SIZE - 1]);
	*call_top = null_cell;
	call_stack[0] = null_cell;
	parameter_top = &(parameter_stack[PARAMETER_STACK_SIZE - 1]);
	*parameter_top = null_cell;

	if (argc > 1) {
		token.in = fopen(argv[1], "r");
		if (!(token.in))
			throw("can't open", create_string(argv[1]));
	} else
		token.in = stdin;

	for (;;) {
		const cell_t *cell = load(&token);
		protect_cell((cell_t *) cell);
		PRINT(stdout, eval(null_cell, cell, false));
#ifdef USE_TRACE
		fprintf(stderr, "\nTRACE: max_call=%ld\n", trace_max_call);
		fprintf(stderr, "TRACE: max_param=%ld\n", trace_max_param);
		fprintf(stderr, "TRACE: num_of_cells=%ld\n", num_of_cells);
		fprintf(stderr, "TRACE: num_of_cells_on_last_gc=%ld\n", num_of_cells_on_last_gc);
#endif
		unprotect_cell((cell_t *) cell);
	}

	return 0;
}

cell_t *create_cell(const cell_tag_e tag)
{
	cell_t *cell;
	list_t *p = list_head(&free_guard);
	if (list_is_edge(&free_guard, p))
		cell = (cell_t *) malloc(sizeof(*cell));
	else {
		free_guard.next = p->next;
		cell = getCellsParent(p);
	}

	if (!cell)
		throw("no memory", NULL);

	cell->tag = tag;
	cell->first = NULL;
	cell->rest = NULL;//TODO NULL or null_cell?
	cell->protect_count = 0;
	cell->hash.next = NULL;
	cell->hash.prev = NULL;

	list_insert(&cells_guard, &(cell->cells));
	num_of_cells++;
	return cell;
}

static void destroy_cell(cell_t *cell)
{
	switch (tag_of(cell)) {
	case CELL_INTEGER:
	case CELL_REFER_LOCAL:
		return;
	case CELL_SYMBOL:
		hash_remove(cell);
	case CELL_STRING:
	case CELL_VECTOR:
		free(cell->name);
		break;
	default:
		break;
	}

	list_remove(&(cell->cells));
	cell->cells.next = free_guard.next;
	free_guard.next = &(cell->cells);
	num_of_cells--;
}

const cell_t *find_global(const cell_t *symbol)
{
	cell_t pair;
	pair.first = symbol;
	return hash_get(&hash_global, symbol->hash_key, &pair);
}

static cell_t *set_global(const char *name, const cell_t *value)
{
	cell_t *key = create_symbol(name);
	cell_t *pair = (cell_t *) find_global(key);
	if (pair)
		pair->rest = value;
	else {
		pair = (cell_t *) create_pair(key, value);
		hash_put(&hash_global, key->hash_key, pair);
	}

	return key;
}

static const cell_t *get_value(const cell_t *env, const cell_t *symbol)
{//TODO combine to 'set!'
	const cell_t **reference = find_env(env, symbol);
	if (reference)
		return *reference;

	const cell_t *pair = find_global(symbol);
	return (pair ? cdr(pair) : NULL);
}

const cell_t **find_env(const cell_t *env, const cell_t *symbol)
{
	for (const cell_t **p = parameter_top;;) {
		for (;; p += words_of_frame(integer_value_of(p[OFFSET_ARGC]))) {
			if (is_null(p[OFFSET_CALLEE]))
				return NULL;

			if (p[OFFSET_CALLEE] == env)
				break;
		}

		long index = get_index(car(env->lambda), symbol);
		if (index >= 0)
			return &(p[index + OFFSET_ARGV]);

		env = env->env;
		if (is_null(env))
			return NULL;
	}
}

static long get_index(const cell_t *vars, const cell_t *symbol)
{
	if (vars->tag == CELL_SYMBOL) {
		if (vars == symbol)
			return 0;
	} else
		for (long index = 0; !is_null(vars); index++, vars = cdr(vars))//TODO check if list
			if (car(vars) == symbol)
				return index;

	return (-1);
}

static cell_t *create_symbol(const char *name)
{
	cell_t symbol;
	symbol.name = (char *) name;
	symbol.hash_key = hash_symbol.calc(&symbol);
	cell_t *cell = (cell_t *) hash_get(&hash_symbol, symbol.hash_key, &symbol);
	if (cell)
		return cell;

	cell = create_cell(CELL_SYMBOL);
	cell->name = (char *) malloc(strlen(name) + 1);
	if (!(cell->name))
		throw("no memory", NULL);

	strcpy(cell->name, name);
	cell->hash_key = symbol.hash_key;
	hash_put(&hash_symbol, cell->hash_key, cell);//TODO when delete?
	return cell;
}

static const cell_t *create_pair(const cell_t *first, const cell_t *rest)
{
	cell_t *cell = create_cell(CELL_PAIR);
	cell->first = first;
	cell->rest = rest;
	return cell;
}

static const cell_t *create_lambda(const cell_t *env, const cell_t *lambda)
{
	cell_t *cell = create_cell(CELL_LAMBDA);
	cell->lambda = lambda;
	cell->env = env;
	return cell;
}

static cell_t *create_boolean(const bool boolean)
{
	cell_t *cell = create_cell(CELL_BOOLEAN);
	cell->boolean = boolean;
	return cell;
}

cell_t *create_integer(const long integer)
{
	return ((cell_t *) ((integer << FIX_TAG_BITS) | CELL_INTEGER));
}
#ifdef USE_FLONUM
cell_t *create_flonum(const double flonum)
{
	if (isnan(flonum))//TODO is not error?
		throw("NaN", NULL);

	cell_t *cell = create_cell(CELL_FLONUM);
	cell->flonum = flonum;
	return cell;
}
#endif
cell_t *create_string(const char *str)
{
	cell_t *cell = create_cell(CELL_STRING);
	cell->name = (char *) malloc(strlen(str) + 1);
	if (!(cell->name))
		throw("no memory", NULL);

	strcpy(cell->name, str);
	return cell;
}

const cell_t *cadr(const cell_t *pair)
{
	return car(cdr(pair));
}

long length(const cell_t *list)//TODO slow. use array
{
	long len = 0;

	for (const cell_t *pair = list; !is_null(pair); pair = cdr(pair))
		len++;

	return len;
}

bool is_list(const cell_t *cell)
{
	for (; !is_null(cell); cell = cdr(cell))
		if (tag_of(cell) != CELL_PAIR)
			return false;

	return true;
}

static const cell_t *load(token_t *token)
{
	token_get_token(token);
	return token_parse_value(token);
}

static const cell_t *token_parse_value(token_t *token)
{
	switch (token->type) {
	case TOKEN_LPAREN:
		return token_parse_list(token);
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
		c = token_getc(token);
		switch (c) {
		case EOF:
			exit(EXIT_SUCCESS);
		case ';':
			do {
				c = token_getc(token);
				if (c == EOF)
					exit(EXIT_SUCCESS);
			} while (c != LF);
			break;
		case '|':
			token_get_until(token, '|');
			token->type = TOKEN_VALUE;
			token->value = create_symbol(token->buf);
			return;
		case '"':
			token_get_until(token, '"');
			token->type = TOKEN_VALUE;
			token->value = create_string(token->buf);//TODO save to constant hash?
			return;
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
					&& (c != ')'))
				token->buf[pos++] = c;

			token->buf[pos] = NUL;
			token->has_last = true;
			token->last = c;
			token->type = TOKEN_VALUE;

			if (!strcmp(token->buf, ".")) {
				token->type = TOKEN_DOT;
				break;
			}

			char *end;
			long integer = strtol(token->buf, &end, 10);
			if (token->buf[0] && !(*end))
				token->value = create_integer(integer);//TODO save to constant hash?
			else {
#ifdef USE_FLONUM
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
	if (token->has_last) {
		token->has_last = false;
		return token->last;
	}

	int c = fgetc(token->in);
	if (c == LF)
		token->line++;

	return c;
}

static void token_get_until(token_t *token, const char last)
{
	long pos = 0;
	for (;;) {
		int c = fgetc(token->in);
		if (c == EOF)
			exit(EXIT_SUCCESS);
		else if (c == last)
			break;

		token->buf[pos++] = c;//TODO process escape a, b, t, n, r, ", \, |, *, x;
	}

	token->buf[pos] = NUL;
}

const cell_t *eval(const cell_t *env, const cell_t *cell, const bool tail)
{
	return (is_list(cell) ?//TODO slow. if null?
		 eval_list(env, cell, tail) : eval_item(env, cell));
}

static const cell_t *eval_item(const cell_t *env, const cell_t *cell)
{
	long tag = tag_of(cell);
	switch (tag) {
	case CELL_SYMBOL: {//TODO when null?
			const cell_t *value = get_value(env, cell);
			if (value)
				return value;
			else
				throw("symbol not found", cell);
		}
		break;
	case CELL_BOOLEAN:
	case CELL_INTEGER:
#ifdef USE_FLONUM
	case CELL_FLONUM:
#endif
	case CELL_STRING:
	case CELL_VECTOR:
		return cell;
	case CELL_REFER_GLOBAL:
		return cdr(car(cell));
	case CELL_REFER_LOCAL: {
			const cell_t **p = (const cell_t **) ((uintptr_t) parameter_top
					+ integer_value_of(cell));
			return *p;
		}
	default://TODO other types? (dot pair, null, executable)
		break;
	}

	throw("cannot evaluate", cell);
}

static const cell_t *eval_list(const cell_t *env, const cell_t *list,
		const bool tail)
{
	const cell_t *f = eval_first(env, list);
	switch (tag_of(f)) {
	case CELL_SYNTAX: {
			if (!(f->inner))//TODO push to call_stack if skip in tail recursion
				push_call(f);

			const cell_t *result = f->syntax(env, cdr(list));//TODO !!recursive (rest parameters)
			if (!(f->inner))
				pop_call();

			return result;
		}
	case CELL_PROCEDURE: {
			push_call(f);
			long argc = eval_rest(env, cdr(list));//TODO !recursive (rest parameters)
			push_parameter(f);

			const cell_t *result = f->procedure(argc, &(parameter_top[OFFSET_ARGV]));
			pop_parameter();
			pop_call();
			return result;
		}
	case CELL_LAMBDA: {
			if (tail)
				for (const cell_t **p = call_top; (*p)->tag == CELL_LAMBDA; p++)
					if (*p == f)
						return create_tail(env, list, f);

			push_call(f);
			protect_cell((cell_t *) f);

			long argc = eval_rest(env, cdr(list));//TODO !recursive (rest parameters)
			const cell_t *result = null_cell;
			for (;;) {
				result = apply(f);//TODO !!recursive (lambda execution)
				if ((tag_of(result) != CELL_TAIL)
						|| (result->first != f))
					break;

				const cell_t **argv = (const cell_t **) (result->rest);
				for (int i = 0; i < argc; i++)
					parameter_top[i + OFFSET_ARGV] = argv[i + OFFSET_ARGV];
					parameter_top++;
					unprotect_cell((cell_t *) result);
					destroy_cell((cell_t *) result);//TODO either unprotect or destroy?
				}

			unprotect_cell((cell_t *) f);
			pop_parameter();
			pop_call();
			return result;
		}
	default:
		break;
	}

	throw("cannot evaluate", list);
}

static const cell_t *eval_first(const cell_t *env, const cell_t *list)
{
	const cell_t *in = car(list);
	const cell_t *f = in;
	switch (tag_of(in)) {
	case CELL_SYMBOL:
		f = get_and_replace_symbol(env, list);
		break;
	case CELL_PAIR:
		f = eval(env, in, false);//TODO !recursive (first parameter)
		break;
	case CELL_REFER_GLOBAL:
		f = cdr(car(in));
		break;
	case CELL_REFER_LOCAL: {
			const cell_t **p = (const cell_t **) ((uintptr_t) parameter_top
					+ integer_value_of(in));
			f = *p;
		}
		break;
	default://TODO return if error
		break;
	}

	return f;
}

static const cell_t *get_and_replace_symbol(const cell_t *env, const cell_t *list)
{
	const cell_t *symbol = car(list);
	const cell_t **p = find_env(env, symbol);
	if (p) {
		((cell_t *) list)->first = create_refer_local(
				(uintptr_t) p - (uintptr_t) parameter_top);
		return *p;
	}

	const cell_t *pair = find_global(symbol);
	if (!pair)
		throw("list symbol not found", symbol);

	cell_t *refer = create_cell(CELL_REFER_GLOBAL);
	refer->first = pair;
	((cell_t *) list)->first = refer;
	return cdr(pair);
}

static long eval_rest(const cell_t *env, const cell_t *list)
{
	if (is_null(list)) {//TODO may be not needed
		push_parameter(create_integer(0));
		return 0;
	}

	protect_cell((cell_t *) list);
	gc();

	long len = eval_reverse(env, list);//TODO !recursive
	unprotect_cell((cell_t *) list);
	push_parameter(create_integer(len));
	return len;
}

static long eval_reverse(const cell_t *env, const cell_t *list)
{
	if (is_null(list))
		return 0;

	const cell_t *arg = eval_and_replace(env, list, false);//TODO !recursive
	protect_cell((cell_t *) arg);

	long len = eval_reverse(env, cdr(list));
	unprotect_cell((cell_t *) arg);
	push_parameter(arg);
	return (len + 1);
}

static const cell_t *eval_and_replace(const cell_t *env, const cell_t *list,
		const bool tail)
{
	const cell_t *cell = car(list);
	return ((tag_of(cell) == CELL_SYMBOL) ?
		get_and_replace_symbol(env, list) : eval(env, cell, tail));//TODO !recursive
}

static const cell_t *apply(const cell_t *lambda)
{
	const cell_t *vars = car(lambda->lambda);
	if (vars->tag == CELL_SYMBOL)//TODO tag_of is not used since checked in lib_lambda
		parameter_to_list();
	else
		validate_length(integer_value_of(parameter_top[OFFSET_ARGC - 1]), length(vars));

	push_parameter(lambda);

	const cell_t *result = null_cell;
	for (const cell_t *p = cdr(lambda->lambda); !is_null(p); p = cdr(p))
		result = eval(lambda, car(p), is_null(cdr(p)));//TODO !recursive (only syntax modify env and tail)

	return result;
}

static void parameter_to_list(void)
{
	long len = integer_value_of(parameter_top[OFFSET_ARGC - 1]);
	if (len) {
		const cell_t *head = create_pair(parameter_top[OFFSET_ARGV - 1], null_cell);
		cell_t *y = (cell_t *) head;
		for (int i = 1; i < len; i++) {
			y->rest = create_pair(parameter_top[i + OFFSET_ARGV - 1], null_cell);
			y = (cell_t *) (y->rest);
		}

		parameter_top += len;
		parameter_top[OFFSET_ARGC - 1] = head;
	} else
		parameter_top[OFFSET_ARGC - 1] = null_cell;

	push_parameter(create_integer(1));
}

static const cell_t *create_tail(const cell_t *env, const cell_t *list,
		const cell_t *f)
{
	cell_t *result = create_cell(CELL_TAIL);
	protect_cell(result);//TODO save to global? unless multi threading
	eval_rest(env, cdr(list));//TODO !recursive (rest parameters)
	push_parameter(f);
	result->first = f;
	result->rest = (const cell_t *) parameter_top;
	pop_parameter();
	return result;
}

static void push_call(const cell_t *f)
{
	call_top--;
	if (*call_top == null_cell)//TODO slow
		throw("call stack overflow", f);
#ifdef USE_TRACE
	long depth = ((uintptr_t) &(call_stack[CALL_STACK_SIZE - 1])
					- (uintptr_t) call_top)
			/ sizeof(uintptr_t);
	if (depth > trace_max_call)
		trace_max_call = depth;
#endif
	*call_top = f;
}

static void pop_call(void)
{
	if (*call_top == null_cell)//TODO slow
		throw("call stack underflow", null_cell);

	call_top++;
}

static void push_parameter(const cell_t *cell)
{
	if ((uintptr_t) parameter_top <= (uintptr_t) parameter_stack)//TODO slow
		throw("param stack overflow", cell);

	parameter_top--;
#ifdef USE_TRACE
	long depth = ((uintptr_t) &(parameter_stack[PARAMETER_STACK_SIZE - 1])
					- (uintptr_t) parameter_top)
			/ sizeof(uintptr_t);
	if (depth > trace_max_param)
		trace_max_param = depth;
#endif
	*parameter_top = cell;
}

static void pop_parameter(void)
{
	if ((uintptr_t) parameter_top >= (uintptr_t) &(parameter_stack[PARAMETER_STACK_SIZE - 1]))//TODO slow
		throw("param stack underflow", null_cell);

	parameter_top += words_of_frame(integer_value_of(parameter_top[OFFSET_ARGC]));
}

static void protect_cell(cell_t *cell)
{
	if (get_fix_tag(cell))
		return;

	if (!(cell->protect_count)) {
		list_remove(&(cell->cells));
		list_insert(&protect_guard, &(cell->cells));
	}

	cell->protect_count++;
}

static void unprotect_cell(cell_t *cell)
{
	if (get_fix_tag(cell))
		return;

	cell->protect_count--;
	if (!(cell->protect_count)) {
		list_remove(&(cell->cells));
		list_insert(&cells_guard, &(cell->cells));
	}
}

static void gc(void)
{
	if (num_of_cells - num_of_cells_on_last_gc >= GC_THRESHOLD) {
		GC_MESSAGE(stderr, "enter GC cells=%ld\n", num_of_cells);
		gc_mark();
		gc_sweep();
		GC_MESSAGE(stderr, "leave GC cells=%ld\n", num_of_cells);
		num_of_cells_on_last_gc = num_of_cells;
	}
}

static void gc_mark(void)
{
	for (long i = 0; i < HASH_SIZE; i++) {
		list_t *guard = &(hash_global.table[i]);
		for (list_t *p = list_head(guard); !list_is_edge(guard, p); p = p->next)
			gc_mark_cell((cell_t *) getHashParent(p));
	}

	for (list_t *p = list_head(&protect_guard); !list_is_edge(&protect_guard, p);
			p = p->next)
		gc_mark_cell(getCellsParent(p));

	for (const cell_t **p = parameter_top; !is_null(p[OFFSET_CALLEE]);) {//TODO really?
		//gc_mark_cell(p[0]);//TODO lambda is marked befor 'apply'

		long len = integer_value_of(p[OFFSET_ARGC]);
		for (long i = 0; i < len; i++)
			gc_mark_cell((cell_t *) (p[i + OFFSET_ARGV]));

		p += words_of_frame(len);
	}
}

static void gc_mark_cell(cell_t *cell)
{
	if (get_using(cell))
		return;

	long tag = tag_of(cell);
	set_using(cell);

	switch (tag) {
	case CELL_PAIR:
	case CELL_LAMBDA:
		gc_mark_cell((cell_t *) car(cell));
		gc_mark_cell((cell_t *) cdr(cell));
		break;
	case CELL_VECTOR:
		for (long i = 0; i < cell->length; i++)
			gc_mark_cell((cell_t *) (cell->vector[i]));
		break;
	default:
		break;
	}
}

static void gc_sweep(void)
{
	for (list_t *p = list_head(&cells_guard); !list_is_edge(&cells_guard, p);) {
		cell_t *cell = getCellsParent(p);
		p = p->next;
		if (get_using(cell))
			unset_using(cell);
		else
			destroy_cell(cell);
	}

	for (list_t *p = list_head(&protect_guard); !list_is_edge(&protect_guard, p);
			p = p->next) {
		cell_t *cell = getCellsParent(p);
		if (get_using(cell))
			unset_using(cell);
	}
}

_Noreturn void throw(const char *message,
		const cell_t *args)
{
	fprintf(stderr, "%ld: %s", token.line, message);
#ifdef USE_REPL
	if (args) {
		fputc(' ', stderr);
		PRINT(stderr, args);
	} else
#else
	if (args) {
		fputc(' ', stderr);
		simple_print(stderr, args);
	} else
#endif
		fputc('\n', stderr);
#ifdef USE_TRACE
	for (const cell_t **p = call_top; !is_null(*p); p++) {
		const cell_t *symbol = find_symbol_by_procedure(*p);
		fputs(" at ", stderr);
		simple_print(stderr, symbol ? symbol : *p);
	}
#endif
	exit(EXIT_FAILURE);
}

#ifdef USE_TRACE
static const cell_t *find_symbol_by_procedure(const cell_t *f)
{
	for (long i = 0; i < HASH_SIZE; i++) {
		list_t *guard = &(hash_global.table[i]);
		for (list_t *p = list_head(guard); !list_is_edge(guard, p);
				p = p->next) {
			cell_t *cell = getHashParent(p);
			if (cell->rest == f)
				return cell->first;
		}
	}

	return NULL;
}
#endif

#if !defined(USE_REPL) || defined(USE_TRACE)
static void simple_print(FILE *out, const cell_t *cell)
{//TODO unify printf (repl.c)
	long tag = tag_of(cell);
	switch (tag) {
	case CELL_SYMBOL:
		fprintf(out, "%s\n", cell->name);
		break;
	case CELL_INTEGER:
		fprintf(out, "%ld\n", integer_value_of(cell));
		break;
#ifdef USE_FLONUM
	case CELL_FLONUM:
		fprintf(out, "%f\n", cell->flonum);
		break;
#endif
	default:
		fprintf(out, "<%ld:%p>\n", tag, cell->first);
		break;
	}
}
#endif

void validate_symbol(const cell_t *arg)
{
	if (tag_of(arg) != CELL_SYMBOL)
		throw("not symbol", arg);
}

static void validate_pair(const cell_t *arg)
{
	if (tag_of(arg) != CELL_PAIR)
		throw("not pair", arg);
}

void validate_length(const long argc, const long n)
{
	if (argc != n) {
		char buf[256];//TODO ugly
		sprintf(buf, "requires %ld argument, but %ld", n, argc);
		throw(buf, null_cell);
	}
}

static void import_core(void)//TODO forbid overwriting module
{
	null_cell = create_symbol("()");
	set_global("null", null_cell);
	unspecified_cell = create_string("");
	set_global("#<unspecified>", unspecified_cell);//TODO is needed to protect as symbol?

	true_cell = create_boolean(true);
	set_global("#t", true_cell);
	set_global("#true", true_cell);
	false_cell = create_boolean(false);
	set_global("#f", false_cell);
	set_global("#false", false_cell);

	bind_syntax("quote", lib_quote, false);
	bind_syntax("lambda", lib_lambda, false);
	bind_syntax("define", lib_define, false);
	bind_syntax("if", lib_if, true);

	bind_procedure("car", lib_car);
	bind_procedure("cdr", lib_cdr);
	bind_procedure("cons", lib_cons);
	bind_procedure("eq?", lib_is_eq);
	bind_procedure("pair?", lib_is_pair);
}

void bind_syntax(char *name,
		const cell_t *(*syntax) (const cell_t *, const cell_t *),
		const bool inner)
{
	cell_t *cell = create_cell(CELL_SYNTAX);
	cell->syntax = syntax;
	cell->inner = inner;
	set_global(name, cell);
}

void bind_procedure(char *name,
		const cell_t *(*procedure) (const long, const cell_t **))
{
	cell_t *cell = create_cell(CELL_PROCEDURE);
	cell->procedure = procedure;
	set_global(name, cell);
}

static const cell_t *lib_quote(const cell_t *env, const cell_t *args)
{
	validate_length(length(args), 1);

	return car(args);
}

static const cell_t *lib_lambda(const cell_t *env, const cell_t *args)
{
	long argc = length(args);
	if (argc < 2) {
		char buf[256];//TODO ugly
		sprintf(buf, "requires %ld argument, but %ld", (long) 2, argc);
		throw(buf, args);
	}

	if ((tag_of(car(args)) != CELL_SYMBOL)
			&& !is_list(car(args)))//TODO allow variable list
		throw("not symbol and list", car(args));

	return create_lambda(env, args);
}

static const cell_t *lib_define(const cell_t *env, const cell_t *args)
{
	validate_length(length(args), 2);//TODO support syntax sugar

	const cell_t *arg1 = car(args);
	validate_symbol(arg1);//TODO create pair before define (for recursion). create lambda in local

	set_global(arg1->name, eval(env, cadr(args), false));
	return unspecified_cell;
}

static const cell_t *lib_if(const cell_t *env, const cell_t *args)
{
	validate_length(length(args), 3);//TODO allow 2 parameters?

	return ((eval_and_replace(env, args, false) == false_cell) ?
			eval_and_replace(env, cdr(cdr(args)), true) : eval_and_replace(env, cdr(args), true));
}

static const cell_t *lib_car(const long argc, const cell_t **argv)
{
	validate_length(argc, 1);
	validate_pair(argv[0]);

	return car(argv[0]);
}

static const cell_t *lib_cdr(const long argc, const cell_t **argv)
{
	validate_length(argc, 1);
	validate_pair(argv[0]);

	return cdr(argv[0]);
}

static const cell_t *lib_cons(const long argc, const cell_t **argv)
{
	validate_length(argc, 2);

	return create_pair(argv[0], argv[1]);
}

static const cell_t *lib_is_eq(const long argc, const cell_t **argv)
{
	validate_length(argc, 2);

	return (is_eq(argv[0], argv[1]) ? true_cell : false_cell);
}

static const cell_t *lib_is_pair(const long argc, const cell_t **argv)
{
	validate_length(argc, 1);

	return ((tag_of(argv[0]) == CELL_PAIR) ? true_cell : false_cell);
}
