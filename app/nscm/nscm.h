#ifndef _NSCM_H_
#define _NSCM_H_ 1
/* MonoLis (minimam Lisp interpreter)
written by kenichi sasagawa 2016/1
ver 0.01
*/
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define LF '\n'
#define NUL '\0'

#define HASH_SIZE (61)

//TODO split to pcompile. or set same value with vm parameter stack
#define PCOMPILE_STACK_SIZE (64)

#define MAX_SPARE_CELLS (1024)
#define GC_THRESHOLD (1024)
//#define GC_SHOW_MESSAGE (1)

//#define USE_REPL (1)
//#define REPL_SHOW_DEBUG (1)

//#define USE_ANALYSIS (1)

//#define USE_FLONUM (1)

#define USE_DERIVED (1)
#define USE_BASE (1)

#ifdef GC_SHOW_MESSAGE
#define GC_MESSAGE(...) fprintf(__VA_ARGS__)
#else
#define GC_MESSAGE(...) do {} while (0)
#endif

#ifdef USE_REPL
#define PRINT(port, cell) print(port, cell)
#else
#define PRINT(port, cell) do { cell; } while (0)
#endif

#ifdef USE_BASE
#define IMPORT_BASE() import_base()
#else
#define IMPORT_BASE() do {} while (0)
#endif

#define FIX_TAG_BITS (2)
#define MASK_TAG_OF_POINTER ((1 << FIX_TAG_BITS) - 1)

typedef struct _list {
	struct _list *next;
	struct _list *prev;
} list_t;

//TODO char, cc, record, rational, complex
	//TODO char: encoding id, value
	//TODO cc; context, vars
	//TODO record: type id, pointer
	//TODO rational: numerator, denominator
	//TODO complex: real, imaginary

typedef enum {
	CELL_CODE = 0,//TODO need gc(vars/data/indirect)。protect while executing
	CELL_INTEGER = 1,
	CELL_OPCODE = 2,
	CELL_REFER_RELATIVE = 3,
	CELL_REFER_INDIRECT = 4,//TODO global hash
	CELL_PROCEDURE = 8,//TODO module hash
	CELL_LAMBDA = 12,//TODO need gc
	CELL_BOOLEAN = 16,//TODO module hash
#ifdef USE_FLONUM
	CELL_FLONUM = 20,//TODO need gc
#endif
	CELL_STRING = 24,//TODO need gc
	CELL_VECTOR = 28,//TODO need gc
	CELL_BYTEVECTOR = 32,//TODO need gc
	CELL_FILE = 36,//TODO need gc
	CELL_PAIR = 40,//TODO need gc
	CELL_SYMBOL = 44//TODO need gc (excluding hash_symbol)
} cell_tag_e;

typedef struct _cell {
	cell_tag_e tag;//TODO has immutable (read-only) flag
#if 0
	union {
		struct {
			const struct _cell **vector;
			long length;
		} code;//TODO same structure with vector (data)
		struct {
			const struct _cell *first;
			const struct _cell *rest;
		} refer_indirect;
		struct {
			const struct _cell *(*procedure)
					(const long, const struct _cell **);
			bool syntax;
		} procedure;
		struct {
			const struct _cell *code;
			const struct _cell *env;
		} lambda;
		struct {
			bool boolean;
		} boolean;
		struct {
			double flonum;
		} flonum;
		struct {
			char *name;
			long length;
		} string;
		struct {
			const struct _cell **vector;
			long length;
		} vector;
		//TODO define bytevector
		//TODO define file
		struct {
			const struct _cell *first;
			const struct _cell *rest;
		} pair;
		struct {
			char *name;
			unsigned long hash_key;
		} symbol;//TODO can share symbol in global, module, symbol?
	};
#endif
	union {//TODO split per tag
		char *name;
		const struct _cell *first;
		const struct _cell *(*procedure)
				(const long, const struct _cell **);
		const struct _cell *code;
		bool boolean;
		long integer;
		const struct _cell **vector;
		uint8_t *bytevector;
#ifdef USE_FLONUM
		double flonum;
#endif
		long mode;
	};
	union {
		const struct _cell *rest;
		const struct _cell *env;
		long length;
		unsigned long hash_key;
		bool syntax;
		int fd;
	};
	//TODO use higher bits in tag
	long protect_count;//TODO release not referred. unify to tag (add 2 each)?
	//TODO unify hash and cells
	list_t hash;//TODO used by only symbol/pair(env). split or use single list
	list_t cells;//TODO doubly linked list for GC,  or protect.
	//TODO array may be slow
} cell_t;

extern cell_t *null_cell;
extern cell_t *unspecified_cell;
extern cell_t *true_cell;
extern cell_t *false_cell;
extern cell_t *empty_vector_cell;
extern cell_t *command_line;

static inline const cell_t *car(const cell_t *pair)
{
	return pair->first;
}

static inline const cell_t *cdr(const cell_t *pair)
{
	return pair->rest;
}

static inline const cell_t *cadr(const cell_t *pair)
{
	return car(cdr(pair));
}

static inline bool is_null(const cell_t *cell)
{
	return (cell == null_cell);
}

static inline long get_fix_tag(const cell_t *cell)
{
	return (((uintptr_t) cell) & MASK_TAG_OF_POINTER);
}

static inline long tag_of(const cell_t *cell)
{
	long tag = get_fix_tag(cell);
	return (tag ? tag : cell->tag);
}

static inline long integer_value_of(const cell_t *cell)
{
	return ((long) (((intptr_t) cell) >> FIX_TAG_BITS));
}

static inline cell_t *to_integer(const long n)
{
	return ((cell_t *) ((n << FIX_TAG_BITS) | CELL_INTEGER));
}

static inline bool is_procedure(const long tag)
{
	return (tag == CELL_PROCEDURE);
}

extern cell_t *create_cell(const cell_tag_e);
extern void destroy_cell(cell_t *);
extern const cell_t *find_global(const cell_t *);
extern cell_t *set_global(const char *, const cell_t *);
extern cell_t *create_symbol(const char *);
extern const cell_t *create_pair(const cell_t *, const cell_t *);
extern const cell_t *create_lambda(const cell_t *, const cell_t *);
extern cell_t *create_integer(const long);
#ifdef USE_FLONUM
extern cell_t *create_flonum(const double);
#endif
extern cell_t *create_string(const char *);
extern cell_t *create_vector(const long);
extern cell_t *create_refer_indirect(const cell_t *, const cell_t *);
extern long list_length(const cell_t *);
extern void protect_cell(cell_t *);
extern void unprotect_cell(cell_t *);
extern void gc(void);
extern _Noreturn void throw(const char *, const cell_t *);
extern const cell_t *find_symbol_by_procedure(const cell_t *);
extern void validate_symbol(const cell_t *);
extern void validate_length(const long, const long);
extern const cell_t *find_module(const cell_t *);
extern cell_t *set_module(const char *, const cell_t *);
extern void bind_syntax(char *, const cell_t *(*)
		(const long, const cell_t **));
extern void bind_procedure(char *, const cell_t *(*)
		(const long, const cell_t **));
extern void print(FILE *, const cell_t *);

extern const cell_t *lib_import(const long, const cell_t **);
extern const cell_t *lib_quote(const long, const cell_t **);
extern const cell_t *lib_lambda(const long, const cell_t **);
extern const cell_t *lib_if(const long, const cell_t **);
extern const cell_t *lib_define(const long, const cell_t **);
extern const cell_t *lib_set(const long, const cell_t **);

#ifdef USE_ANALYSIS
extern void *nmalloc(size_t);
extern void nfree(void *);
extern void *nrealloc(void *, size_t);
#else
#include <stdlib.h>

static inline void *nmalloc(size_t size)
{
	return malloc(size);
}

static inline void nfree(void *ptr)
{
	free(ptr);
}

static inline void *nrealloc(void *ptr, size_t size)
{
	return realloc(ptr, size);
}
#endif

// pcompile
extern void pcompile_initialize(const void **);
extern const cell_t *pcompile(const cell_t *);

#ifdef USE_BASE
// base
extern void import_base(void);
extern const cell_t *lib_add(const long, const cell_t **);
extern const cell_t *lib_subtract(const long, const cell_t **);
extern const cell_t *lib_multiply(const long, const cell_t **);//TODO unused
extern const cell_t *lib_divide(const long, const cell_t **);//TODO unused
#endif

#endif
