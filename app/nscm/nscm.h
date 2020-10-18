#ifndef _MLIS_H_
#define _MLIS_H_ 1
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

#define CALL_STACK_SIZE (1024)
#define PARAMETER_STACK_SIZE (4096)

#define GC_THRESHOLD (1024)
//#define GC_SHOW_MESSAGE (1)

//#define USE_REPL (1)
//#define USE_TRACE (1)

//#define USE_FLONUM (1)

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

typedef enum {
	CELL_SYMBOL = 0,
	CELL_PAIR = 4,
	CELL_SYNTAX = 8,
	CELL_PROCEDURE = 12,
	CELL_LAMBDA = 16,
	CELL_BOOLEAN = 20,
	CELL_INTEGER = 1,
#ifdef USE_FLONUM
	CELL_FLONUM = 24,
#endif
	CELL_STRING = 28,
	CELL_VECTOR = 32,
	CELL_TAIL = 36,
	CELL_REFER_GLOBAL = 40,
	CELL_REFER_LOCAL = 3
} cell_tag_e;

typedef struct _cell {
	cell_tag_e tag;//TODO has immutable (read-only) flag
#if 0
	union {
		struct {
			char *name;
			unsigned long hash_key;
		} symbol;
		struct {
			const struct _cell *first;
			const struct _cell *rest;
		} pair;
		struct {
			const struct _cell *(*syntax)
					(const struct _cell *, const struct _cell *);
			bool inner;
		} syntax;
		struct {
			const struct _cell *(*procedure)
					(const long, const struct _cell **);
		} procedure;
		struct {
			const struct _cell *lambda;//TODO compile P-Code
			const struct _cell *env;
		} lambda;
		struct {
			bool boolean;
		} boolean;
		struct {
			long integer;
		} integer;
		struct {
			char *name;
			long length;
		} string;
		struct {
			const struct _cell **vector;
			long length;
		} vector;
		struct {
			const struct _cell *lambda;
			const struct _cell *rest;
		} tail;
		struct {
			const struct _cell *first;
		} refer_global;
		struct {
			long integer;
		} refer_local;
		struct {
			double flonum;
		} flonum;
	};
#endif
	union {//TODO split per tag
		char *name;
		const struct _cell *first;
		const struct _cell *(*syntax)
				(const struct _cell *, const struct _cell *);
		const struct _cell *(*procedure)
				(const long, const struct _cell **);
		const struct _cell *lambda;//TODO compile P-Code
		bool boolean;
		long integer;
		const struct _cell **vector;
#ifdef USE_FLONUM
		double flonum;
#endif
	};
	union {
		const struct _cell *rest;
		const struct _cell *env;
		long length;
		unsigned long hash_key;
		bool inner;
	};
	long protect_count;//TODO release not referred. unify to tag (add 2 each)?
	list_t hash;//TODO used by only symbol/pair(env). split or use single list
	list_t cells;//TODO sigle list for GC, or dual list for protect.
} cell_t;

extern cell_t *null_cell;
extern cell_t *unspecified_cell;
extern cell_t *true_cell;
extern cell_t *false_cell;

static inline const cell_t *car(const cell_t *pair)
{
	return pair->first;
}

static inline const cell_t *cdr(const cell_t *pair)
{
	return pair->rest;
}

static inline bool is_null(const cell_t *cell)
{
	return (cell == null_cell);//TODO symbol nil or NULL?
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

extern  cell_t *create_cell(const cell_tag_e);
extern const cell_t *find_global(const cell_t *);
extern const cell_t **find_env(const cell_t *, const cell_t *);
extern cell_t *create_integer(const long);
#ifdef USE_FLONUM
extern cell_t *create_flonum(const double);
#endif
extern cell_t *create_string(const char *);
extern const cell_t *cadr(const cell_t *);
extern long length(const cell_t *);
extern bool is_list(const cell_t *);
extern const cell_t *eval(const cell_t *, const cell_t *, const bool);
extern _Noreturn void throw(const char *, const cell_t *);
extern void validate_symbol(const cell_t *);
extern void validate_length(const long, const long);
extern void bind_syntax(char *, const cell_t *(*)
		(const cell_t *, const cell_t *), const bool);
extern void bind_procedure(char *, const cell_t *(*)
		(const long, const cell_t **));
extern void print(FILE *, const cell_t *);

#ifdef USE_BASE
extern void import_base(void);
#endif

#endif
