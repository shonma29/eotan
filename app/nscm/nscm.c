/* MonoLis (minimam Lisp interpreter)
written by kenichi sasagawa 2016/1

*/
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "nscm.h"
#include "parse.h"
#include "vm.h"

#ifdef USE_FLONUM
#include <math.h>
#endif

#ifdef USE_ANALYSIS
#ifdef __MACH__
#include <malloc/malloc.h>
#define MALLOC_SIZE malloc_size
#else
#ifdef __linux__
#include <malloc.h>
#define MALLOC_SIZE malloc_usable_size
#else
#include <malloc.h>
#define MALLOC_SIZE(p) (0)
#endif
#endif
#endif

#define USING_BIT (1)
#define MASK_MARK MASK_TAG_OF_POINTER

typedef struct _hash {
	unsigned long (*calc)(const cell_t *);
	long (*compare)(const cell_t *, const cell_t *);
	list_t table[HASH_SIZE];
} hash_t;

static token_t token = {
	NULL, false, TOKEN_UNKNOWN, NULL, 1, 0, 0, 0
};
static list_t protect_guard;
static list_t cells_guard;
static list_t free_guard;
static long num_of_cells;
static long num_of_cells_on_last_gc;
static long num_of_spare_cells;
#ifdef USE_ANALYSIS
static size_t size_of_memory;
static size_t max_memory;
static size_t num_of_malloc;
static size_t num_of_free;
#endif

static hash_t hash_global;
static hash_t hash_symbol;
static hash_t hash_module;

cell_t *null_cell;
cell_t *unspecified_cell;
cell_t *true_cell;
cell_t *false_cell;
cell_t *empty_vector_cell;
cell_t *command_line;

static context_t current_context;

static inline cell_t *getHashParent(const list_t *p)
{
    return ((cell_t *) ((uintptr_t) p - offsetof(cell_t, hash)));
}

static inline cell_t *getCellsParent(const list_t *p)
{
    return ((cell_t *) ((uintptr_t) p - offsetof(cell_t, cells)));
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

#ifdef USE_ANALYSIS
static void show_analysis(void);
#endif
static const cell_t *evaluate(context_t *, const cell_t *);

static cell_t *create_boolean(const bool);

static void gc_mark(void);
static void gc_mark_cell(cell_t *);
static void gc_sweep(void);

static const cell_t *find_symbol_from_table(hash_t *, const cell_t *);
#ifndef USE_REPL
static void simple_print(FILE *, const cell_t *);
#endif

static void validate_pair(const cell_t *);

static void import_core(void);

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
	hash_initialize(&hash_module, hash_calc_symbol, hash_compare_global);

	import_core();

	//TODO set read-only
	command_line = null_cell;
	for (int i = argc - 1; i >= 0; i--) {
		const cell_t *value = create_string((const char *) (argv[i]));
		command_line = (cell_t *) create_pair(value, command_line);
		protect_cell((cell_t *) value);
		protect_cell(command_line);
	}

	IMPORT_BASE();//TODO test duplicate fields in symbol and env
	preprocess_initialize();
	num_of_cells_on_last_gc = num_of_cells;
#ifdef USE_ANALYSIS
	fprintf(stderr, "ANALYSIS: num_of_cells=%ld\n", num_of_cells);
#endif

	vm_initialize(&current_context);
	pcompile_initialize((const void **) vm_start(NULL, NULL));

	if (argc > 1) {
		token.in = fopen(argv[1], "r");
		if (!(token.in))
			throw("can't open", create_string(argv[1]));
	} else
		token.in = stdin;

	for (;;) {
		PRINT(stdout, evaluate(&current_context, load(&token)));
#ifdef USE_ANALYSIS
		show_analysis();
#endif
	}

	return 0;
}

#ifdef USE_ANALYSIS
static void show_analysis(void)
{
	fprintf(stderr, "\nANALYSIS: max_call=%ld\n", current_context.analysis_max_call);
	fprintf(stderr, "ANALYSIS: max_param=%ld\n", current_context.analysis_max_param);
	fprintf(stderr, "ANALYSIS: num_of_cells=%ld\n", num_of_cells);
	fprintf(stderr, "ANALYSIS: num_of_cells_on_last_gc=%ld\n", num_of_cells_on_last_gc);
	fprintf(stderr, "ANALYSIS: num_of_spare_cells=%ld\n", num_of_spare_cells);
	fprintf(stderr, "ANALYSIS: max_memory=%zu\n", max_memory);
	fprintf(stderr, "ANALYSIS: size_of_memory=%zu\n", size_of_memory);
	fprintf(stderr, "ANALYSIS: num_of_malloc=%zu\n", num_of_malloc);
	fprintf(stderr, "ANALYSIS: num_of_free=%zu\n", num_of_free);
}
#endif

static const cell_t *evaluate(context_t *cc, const cell_t *cell)
{
#ifdef USE_REPL
	if (!is_null(cell)) {
		if (tag_of(cell) == CELL_SYMBOL) {
			const cell_t *value = find_global(cell);
			if (value)
				return cdr(value);

			value = find_module(cell);
			if (value)
				return cdr(value);

			throw("symbol not defined", cell);
		} else if (tag_of(cell) != CELL_PAIR)
			return cell;
	}
#endif
	//TODO is needed to protect S expression?
		//TODO can destroy structure on pcompile?
	const cell_t *code = pcompile(cell);
	protect_cell((cell_t *) code);

	//TODO dependent on vm
	const cell_t *result = vm_start(cc, code);
	unprotect_cell((cell_t *) code);
	destroy_cell((cell_t *) code);
	return result;
}

cell_t *create_cell(const cell_tag_e tag)
{
	cell_t *cell;
	list_t *p = list_head(&free_guard);
	if (list_is_edge(&free_guard, p))
		cell = (cell_t *) nmalloc(sizeof(*cell));
	else {
		free_guard.next = p->next;
		cell = getCellsParent(p);
		if (cell)
			num_of_spare_cells--;
	}

	if (!cell)
		throw("no memory", NULL);

	cell->tag = tag;
	cell->first = NULL;
	cell->rest = NULL;
	cell->protect_count = 0;
	cell->hash.next = NULL;
	cell->hash.prev = NULL;

	list_insert(&cells_guard, &(cell->cells));
	num_of_cells++;
	return cell;
}

void destroy_cell(cell_t *cell)
{
	switch (tag_of(cell)) {
	case CELL_INTEGER:
	case CELL_OPCODE:
	case CELL_REFER_RELATIVE:
		return;
	case CELL_SYMBOL:
		hash_remove(cell);
	case CELL_CODE://TODO empty is allowed?
	case CELL_STRING://TODO empty is allowed?
	case CELL_BYTEVECTOR://TODO empty is allowed?
		nfree(cell->name);
		break;
	case CELL_VECTOR:
		if (cell->vector)
			nfree(cell->vector);

		break;
	case CELL_FILE:
		if (cell->mode) {
			close(cell->fd);
			cell->mode = 0;
		}
		break;
	default:
		break;
	}

	list_remove(&(cell->cells));
	num_of_cells--;

	if (num_of_spare_cells >= MAX_SPARE_CELLS) {
		nfree(cell);
	} else {
		cell->cells.next = free_guard.next;
		free_guard.next = &(cell->cells);
		num_of_spare_cells++;
	}
}

const cell_t *find_global(const cell_t *symbol)
{
	cell_t pair;
	pair.first = symbol;
	return hash_get(&hash_global, symbol->hash_key, &pair);
}

cell_t *set_global(const char *name, const cell_t *value)
{
	cell_t *key = create_symbol(name);
	cell_t *pair = (cell_t *) find_global(key);
	if (pair)
		pair->rest = value;
	else {
		pair = create_refer_indirect(key, value);
		//TODO rehash
		hash_put(&hash_global, key->hash_key, pair);
	}

	return pair;
}

cell_t *create_symbol(const char *name)
{
	cell_t symbol;
	symbol.name = (char *) name;
	symbol.hash_key = hash_symbol.calc(&symbol);
	//TODO delete hash_symbol after precompile? (excluding repl)
	cell_t *cell = (cell_t *) hash_get(&hash_symbol, symbol.hash_key, &symbol);
	if (cell)
		return cell;

	cell = create_cell(CELL_SYMBOL);
	cell->name = (char *) nmalloc(strlen(name) + 1);
	if (!(cell->name))
		throw("no memory", NULL);

	strcpy(cell->name, name);
	cell->hash_key = symbol.hash_key;
	hash_put(&hash_symbol, cell->hash_key, cell);//TODO when delete?
	return cell;
}

const cell_t *create_pair(const cell_t *first, const cell_t *rest)
{
	cell_t *cell = create_cell(CELL_PAIR);
	cell->first = first;
	cell->rest = rest;
	return cell;
}

const cell_t *create_lambda(const cell_t *env, const cell_t *code)
{
	cell_t *cell = create_cell(CELL_LAMBDA);
	cell->code = code;
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
	//TODO check overflow
	return to_integer(integer);
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
	cell->name = (char *) nmalloc(strlen(str) + 1);
	if (!(cell->name))
		throw("no memory", NULL);

	strcpy(cell->name, str);
	return cell;
}

cell_t *create_vector(const long size)
{
	if (!size)
		return empty_vector_cell;

	cell_t *cell = create_cell(CELL_VECTOR);
	cell->vector = (const cell_t **) nmalloc(size * sizeof(cell->vector[0]));
	if (!(cell->vector))
		throw("no memory", NULL);

	cell->length = size;
	return cell;
}

cell_t *create_refer_indirect(const cell_t *key, const cell_t *value)
{
	cell_t *refer = create_cell(CELL_REFER_INDIRECT);
	refer->first = key;
	refer->rest = value;
	return refer;
}

long list_length(const cell_t *list)
{
	long len = 0;

	for (const cell_t *pair = list; !is_null(pair); pair = cdr(pair)) {
		if (tag_of(pair) != CELL_PAIR)
			return (-1);

		len++;
	}

	return len;
}

void protect_cell(cell_t *cell)
{
	if (get_fix_tag(cell))
		return;

	if (!(cell->protect_count)) {
		list_remove(&(cell->cells));
		list_insert(&protect_guard, &(cell->cells));
	}

	cell->protect_count++;//TODO check overflow
}

void unprotect_cell(cell_t *cell)
{
	if (get_fix_tag(cell))
		return;

	cell->protect_count--;//TODO check underflow
	if (!(cell->protect_count)) {
		list_remove(&(cell->cells));
		list_insert(&cells_guard, &(cell->cells));
	}
}

void gc(void)
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
	for (long i = 0; i < HASH_SIZE; i++) {//TODO extract common logic
		list_t *guard = &(hash_module.table[i]);
		for (list_t *p = list_head(guard); !list_is_edge(guard, p); p = p->next)
			gc_mark_cell((cell_t *) getHashParent(p));
	}

	for (long i = 0; i < HASH_SIZE; i++) {
		list_t *guard = &(hash_global.table[i]);
		for (list_t *p = list_head(guard); !list_is_edge(guard, p); p = p->next)
			gc_mark_cell((cell_t *) getHashParent(p));
	}

	for (list_t *p = list_head(&protect_guard); !list_is_edge(&protect_guard, p);
			p = p->next)
		gc_mark_cell(getCellsParent(p));

	gc_mark_cell((cell_t *) (current_context.d1));

	//TODO can guard on interrupt (may push null)?
	for (const cell_t **p = current_context.sp; !is_null(p[OFFSET_CALLEE]);) {//TODO really?
		gc_mark_cell((cell_t *) (p[OFFSET_CALLEE]));

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
	case CELL_REFER_INDIRECT:
	case CELL_LAMBDA:
	case CELL_PAIR:
		gc_mark_cell((cell_t *) car(cell));
		gc_mark_cell((cell_t *) cdr(cell));
		break;
	case CELL_CODE://TODO skip children of closure/indirect
		for (long i = 0; i < OFFSET_OPCODE; i++)
			gc_mark_cell((cell_t *) (cell->vector[i]));
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

	for (const cell_t **p = current_context.sp; !is_null(p[OFFSET_CALLEE]);
			p += words_of_frame(integer_value_of(p[OFFSET_ARGC]))) {
		const cell_t *symbol = (tag_of(p[OFFSET_CALLEE]) == CELL_PROCEDURE) ?
				find_symbol_by_procedure(p[OFFSET_CALLEE]) : NULL;
		fputs(" at ", stderr);
#ifdef USE_REPL
		print(stderr, symbol ? symbol : p[OFFSET_CALLEE]);
#else
		simple_print(stderr, symbol ? symbol : p[OFFSET_CALLEE]);
#endif
	}

	exit(EXIT_FAILURE);
}

const cell_t *find_symbol_by_procedure(const cell_t *f)
{
	const cell_t *symbol = find_symbol_from_table(&hash_module, f);
	if (!symbol)
		symbol = find_symbol_from_table(&hash_global, f);

	return symbol;
}

static const cell_t *find_symbol_from_table(hash_t *hash, const cell_t *f)
{
	for (long i = 0; i < HASH_SIZE; i++) {
		list_t *guard = &(hash->table[i]);
		for (list_t *p = list_head(guard); !list_is_edge(guard, p);
				p = p->next) {
			cell_t *cell = getHashParent(p);
			if (cell->rest == f)
				return cell->first;
		}
	}

	return NULL;
}

#ifndef USE_REPL
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
	case CELL_LAMBDA://TODO redefine
		fputs("<lambda ", out);
		{
			const cell_t *head = cell->code->vector[OFFSET_VARS];
			switch (tag_of(head)) {
			case CELL_SYMBOL:
				fprintf(out, "%s", head->name);
				break;
			case CELL_VECTOR:
				fputc('(', out);
				for (long i = 0; i < head->length; i++) {
					if (i)
						fputs(" ", out);

					fprintf(out, "%s", head->vector[i]->name);
				}
				fputc(')', out);
				break;
			default:
				break;
			}
		}
		fputs(">\n", out);
		break;
	case CELL_CODE:
		//TODO show variables? like display
		fprintf(out, "<code %p>\n", cell);
		break;
	case CELL_REFER_RELATIVE:
		fprintf(out, "<relative %ld>\n", integer_value_of(cell));
		break;
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

const cell_t *find_module(const cell_t *symbol)
{
	cell_t pair;
	pair.first = symbol;
	return hash_get(&hash_module, symbol->hash_key, &pair);
}

cell_t *set_module(const char *name, const cell_t *value)
{
	cell_t *key = create_symbol(name);
	cell_t *pair = (cell_t *) find_module(key);
	if (pair)
		pair->rest = value;
	else {
		pair = create_refer_indirect(key, value);
		hash_put(&hash_module, key->hash_key, pair);
	}

	return key;
}

static void import_core(void)//TODO forbid overwriting module
{
	null_cell = create_symbol("()");
	set_module("null", null_cell);
	unspecified_cell = create_string("");
	set_module("#<unspecified>", unspecified_cell);//TODO is needed to protect as symbol?
	empty_vector_cell = create_cell(CELL_VECTOR);
	set_module("#()", empty_vector_cell);

	true_cell = create_boolean(true);
	set_module("#t", true_cell);
	set_module("#true", true_cell);
	false_cell = create_boolean(false);
	set_module("#f", false_cell);
	set_module("#false", false_cell);

	//TODO useless, but needed in pcompile
	bind_syntax("quote", lib_quote);
	bind_syntax("lambda", lib_lambda);
	bind_syntax("if", lib_if);

	bind_syntax("define", lib_define);
	bind_syntax("set!", lib_set);

	//TODO move to base.c
	bind_procedure("import", lib_import);
	bind_procedure("car", lib_car);
	bind_procedure("cdr", lib_cdr);
	bind_procedure("cons", lib_cons);
	bind_procedure("eq?", lib_is_eq);
	bind_procedure("pair?", lib_is_pair);
}

void bind_syntax(char *name,
		const cell_t *(*syntax) (const long, const cell_t **))
{
	cell_t *cell = create_cell(CELL_PROCEDURE);
	cell->procedure = syntax;
	cell->syntax = true;
	set_module(name, cell);
}

void bind_procedure(char *name,
		const cell_t *(*procedure) (const long, const cell_t **))
{
	cell_t *cell = create_cell(CELL_PROCEDURE);
	cell->procedure = procedure;
	set_module(name, cell);
}

const cell_t *lib_quote(const long argc, const cell_t **argv)
{
	throw("is syntax", null_cell);
}

const cell_t *lib_lambda(const long argc, const cell_t **argv)
{
	throw("is syntax", null_cell);
}

const cell_t *lib_if(const long argc, const cell_t **argv)
{
	throw("is syntax", null_cell);
}
//TODO move to interpret?
const cell_t *lib_define(const long argc, const cell_t **argv)
{
#ifdef CHECK_SYNTAX_ON_EXECUTING
	validate_length(argc, 2);//TODO support syntax sugar
	//validate_symbol(argv[0]);//TODO create pair before define (for recursion). create lambda in local
	//TODO error when there is the symbol in modules
#endif
	const cell_t *cell = argv[0];
	switch (tag_of(cell)) {
	case CELL_REFER_INDIRECT: {
			cell_t *pair = (cell_t *) cell;
			pair->rest = argv[1];
		}
		break;
	default:
		throw("not found", cell);
	}

	return unspecified_cell;
}
//TODO move to interpret? referring context
const cell_t *lib_set(const long argc, const cell_t **argv)
{
#ifdef CHECK_SYNTAX_ON_EXECUTING
	validate_length(argc, 2);
#endif
	const cell_t *cell = argv[0];
	switch (tag_of(cell)) {
	case CELL_INTEGER: {
			long offset = integer_value_of(cell);
			uintptr_t base = (uintptr_t) ((offset & RELATIVE_MASK) ?
					(current_context.ep) : (current_context.sp));
			const cell_t **p = (const cell_t **) base;
			p = &(p[offset >> RELATIVE_BITS]);
			if (tag_of(*p) == CELL_REFER_INDIRECT)
				((cell_t *) *p)->rest = argv[1];
			else
				*p = argv[1];
		}
		break;
	case CELL_REFER_INDIRECT: {
			cell_t *pair = (cell_t *) cell;
			if (pair->rest == unspecified_cell)
				throw("not found", pair->first);

			pair->rest = argv[1];
		}
		break;
	default:
		throw("not found", cell);
	}

	return unspecified_cell;
}

const cell_t *lib_import(const long argc, const cell_t **argv)
{
	//TODO define operation
	throw("nop", null_cell);
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

#ifdef USE_ANALYSIS
void *nmalloc(size_t size) {
	void *ptr = malloc(size);
#ifdef USE_ANALYSIS
	size_of_memory += MALLOC_SIZE(ptr);
	if (size_of_memory > max_memory)
		max_memory = size_of_memory;

	num_of_malloc++;
#endif
	return ptr;
}

void nfree(void *ptr) {
#ifdef USE_ANALYSIS
	size_of_memory -= MALLOC_SIZE(ptr);
	num_of_free++;
#endif
	free(ptr);
}

void *nrealloc(void *ptr, size_t size) {
#ifdef USE_ANALYSIS
	size_of_memory -= MALLOC_SIZE(ptr);
	num_of_free++;
#endif
	ptr = realloc(ptr, size);
#ifdef USE_ANALYSIS
	size_of_memory += MALLOC_SIZE(ptr);
	if (size_of_memory > max_memory)
		max_memory = size_of_memory;

	num_of_malloc++;
#endif
	return ptr;
}
#endif
