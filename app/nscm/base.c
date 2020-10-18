#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "nscm.h"

#ifdef USE_BASE

static cell_t *create_vector(const long);
#if 0
static bool is_eqv(const cell_t *, const cell_t *);
#endif

static void validate_integer(const cell_t *);
static void validate_string(const cell_t *);
static void validate_vector(const cell_t *);

static const cell_t *lib_set(const cell_t *, const cell_t *);
static const cell_t *lib_begin(const cell_t *, const cell_t *);
static const cell_t *lib_and(const cell_t *, const cell_t *);
static const cell_t *lib_or(const cell_t *, const cell_t *);
#if 0
static const cell_t *lib_is_eqv(const long, const cell_t **);
static const cell_t *lib_is_list(const long, const cell_t **);
#endif
static const cell_t *lib_not(const long, const cell_t **);
static const cell_t *lib_add(const long, const cell_t **);
#ifdef USE_FLONUM
static const cell_t *flonum_add(const long, const cell_t **,
		const double, const long);
#endif
static const cell_t *lib_subtract(const long, const cell_t **);
#ifdef USE_FLONUM
static const cell_t *flonum_subtract(const long, const cell_t **,
		const double, const long);
#endif
static const cell_t *lib_multiply(const long, const cell_t **);//TODO unused
#ifdef USE_FLONUM
static const cell_t *flonum_multiply(const long, const cell_t **,
		const double, const long);
#endif
static const cell_t *lib_divide(const long, const cell_t **);//TODO unused
#ifdef USE_FLONUM
static const cell_t *flonum_divide(const long, const cell_t **,
		const double, const long);
#endif
static const cell_t *lib_truncate_quotient(const long, const cell_t **);//TODO should use in fixnum?
static const cell_t *lib_truncate_remainder(const long, const cell_t **);//TODO might be needless in Dartmouth BASIC
static const cell_t *lib_equal(const long, const cell_t **);
static const cell_t *lib_less(const long, const cell_t **);
static const cell_t *lib_greater(const long, const cell_t **);//TODO unused
static const cell_t *lib_less_or_equal(const long, const cell_t **);
static const cell_t *lib_greater_or_equal(const long, const cell_t **);
static const cell_t *lib_number_string(const long, const cell_t **);
static const cell_t *lib_string_number(const long, const cell_t **);
static const cell_t *lib_string_length(const long, const cell_t **);
static const cell_t *lib_string_equal(const long, const cell_t **);
static const cell_t *lib_string_copy(const long, const cell_t **);
static const cell_t *lib_make_vector(const long, const cell_t **);
static const cell_t *lib_vector_ref(const long, const cell_t **);
static const cell_t *lib_vector_set(const long, const cell_t **);
static const cell_t *lib_read_line(const long, const cell_t **);
static const cell_t *lib_write_string(const long, const cell_t **);
#if 0
static _Noreturn const cell_t *lib_exit(const long, const cell_t **);
#endif
static const cell_t *lib_random_integer(const long, const cell_t **);


static cell_t *create_vector(const long size)//TODO move to 'base.c'
{
	cell_t *cell = create_cell(CELL_VECTOR);
	cell->vector = (const cell_t **) malloc(size * sizeof(cell));
	if (!(cell->vector))
		throw("no memory", NULL);

	cell->length = size;
	for (long i = 0; i < size; i++)
		cell->vector[i] = null_cell;

	return cell;
}
#if 0
static bool is_eqv(const cell_t *cell1, const cell_t *cell2)
{
	long tag = tag_of(cell1);
	switch (tag) {
	case CELL_INTEGER:
		return ((tag == tag_of(cell2))
				&& (integer_value_of(cell1) == integer_value_of(cell2)));
#ifdef USE_FLONUM
	case CELL_FLONUM:
		return ((tag == tag_of(cell2))
				&& (cell1->integer == cell2->integer));
#endif
	default:
		return (cell1 == cell2);
	}
}
#endif
static void validate_integer(const cell_t *arg)
{
	if (tag_of(arg) != CELL_INTEGER)
		throw("not integer", arg);
}

static void validate_string(const cell_t *arg)
{
	if (tag_of(arg) != CELL_STRING)
		throw("not string", arg);
}

static void validate_vector(const cell_t *arg)
{
	if (tag_of(arg) != CELL_VECTOR)
		throw("not vector", arg);
}

void import_base(void)
{
	bind_syntax("set!", lib_set, false);
	bind_syntax("begin", lib_begin, true);
	bind_syntax("and", lib_and, true);
	bind_syntax("or", lib_or, true);
#if 0
	bind_procedure("eqv?", lib_is_eqv);
	bind_procedure("list?", lib_is_list);
#endif
	bind_procedure("not", lib_not);
	bind_procedure("+", lib_add);
	bind_procedure("-", lib_subtract);
	bind_procedure("*", lib_multiply);
	bind_procedure("/", lib_divide);
	bind_procedure("truncate-quotient", lib_truncate_quotient);
	bind_procedure("truncate-remainder", lib_truncate_remainder);
	bind_procedure("=", lib_equal);
	bind_procedure("<", lib_less);
	bind_procedure(">", lib_greater);//TODO not(<=)
	bind_procedure("<=", lib_less_or_equal);
	bind_procedure(">=", lib_greater_or_equal);//TODO not(<)
	bind_procedure("number->string", lib_number_string);
	bind_procedure("string->number", lib_string_number);
	bind_procedure("string-length", lib_string_length);
	bind_procedure("string=?", lib_string_equal);
	bind_procedure("string-copy", lib_string_copy);
	bind_procedure("make-vector", lib_make_vector);
	bind_procedure("vector-ref", lib_vector_ref);
	bind_procedure("vector-set!", lib_vector_set);
	bind_procedure("read-line", lib_read_line);
	bind_procedure("write-string", lib_write_string);
#if 0
	bind_procedure("exit", lib_exit);
#endif
	bind_procedure("random-integer", lib_random_integer);
	srand(time(NULL));
}

static const cell_t *lib_set(const cell_t *env, const cell_t *args)
{
	validate_length(length(args), 2);

	const cell_t *symbol = car(args);
	validate_symbol(symbol);//TODO replace to CELL_REFER. but slow in interpreter

	const cell_t **reference = find_env(env, symbol);
	if (reference)
		*reference = eval(env, cadr(args), false);//TODO replace to CELL_REFER. but slow in interpreter
	else {
		cell_t *pair = (cell_t *) find_global(symbol);
		if (!pair)
			throw("not found", symbol);

		pair->rest = eval(env, cadr(args), false);//TODO replace to CELL_REFER. but slow in interpreter
	}

	return unspecified_cell;
}

static const cell_t *lib_begin(const cell_t *env, const cell_t *args)
{
	const cell_t *result = null_cell;//TODO really?
	for (; !is_null(args); args = cdr(args))//TODO check if args >= 1
		result = eval(env, car(args), is_null(cdr(args)));

	return result;
}

static const cell_t *lib_and(const cell_t *env, const cell_t *args)
{
	const cell_t *result = true_cell;
	for (; !is_null(args); args = cdr(args)) {
		result = eval(env, car(args), is_null(cdr(args)));
		if (result == false_cell)
			break;
	}

	return result;
}

static const cell_t *lib_or(const cell_t *env, const cell_t *args)
{
	const cell_t *result = false_cell;
	for (; !is_null(args); args = cdr(args)) {
		result = eval(env, car(args), is_null(cdr(args)));
		if (result != false_cell)
			break;
	}

	return result;
}

#if 0
static const cell_t *lib_is_eqv(const long argc, const cell_t **argv)
{
	validate_length(argc, 2);

	return (is_eqv(argv[0], argv[1]) ? true_cell : false_cell);
}

static const cell_t *lib_is_list(const long argc, const cell_t *args)
{
	validate_length("list?", args, 1);

	return (is_list(car(args)) ? true_cell : false_cell);
}
#endif
static const cell_t *lib_not(const long argc, const cell_t **argv)
{
	validate_length(argc, 1);

	return ((argv[0] == false_cell) ? true_cell : false_cell);
}

static const cell_t *lib_add(const long argc, const cell_t **argv)
{
	long result = 0;
	for (long i = 0; i < argc; i++) {//TODO check if args >= 1
		const cell_t *cell = argv[i];
		switch (tag_of(cell)) {
		case CELL_INTEGER:
			result += integer_value_of(cell);
			break;
#ifdef USE_FLONUM
		case CELL_FLONUM:
			return flonum_add(argc, argv, result, i);
#endif
		default:
			throw("is not number", cell);
			break;
		}
	}

	return create_integer(result);
}
#ifdef USE_FLONUM
static const cell_t *flonum_add(const long argc, const cell_t **argv,
		const double initial, const long from)
{
	double result = initial;
	for (long i = from; i < argc; i++) {
		const cell_t *cell = argv[i];
		switch (tag_of(cell)) {
		case CELL_INTEGER:
			result += integer_value_of(cell);
			break;
		case CELL_FLONUM:
			result += cell->flonum;
			break;
		default:
			throw("is not number", cell);
			break;
		}
	}

	return create_flonum(result);
}
#endif
static const cell_t *lib_subtract(const long argc, const cell_t **argv)
{
	const cell_t *cell = argv[0];
	if (argc == 1) {//TODO check if args >= 1
		switch (tag_of(cell)) {
		case CELL_INTEGER:
			return create_integer(- integer_value_of(cell));
#ifdef USE_FLONUM
		case CELL_FLONUM:
			return create_flonum(- cell->flonum);
#endif
		default:
			throw("is not number", cell);
			break;
		}
	}
#ifdef USE_FLONUM
	if (tag_of(cell) == CELL_FLONUM)
		return flonum_subtract(argc, argv, cell->flonum, 1);
#endif
	long result = integer_value_of(cell);
	for (long i = 1; i < argc; i++) {
		cell = argv[i];
		switch (tag_of(cell)) {
		case CELL_INTEGER:
			result -= integer_value_of(cell);
			break;
#ifdef USE_FLONUM
		case CELL_FLONUM:
			return flonum_subtract(argc, argv, result, i);
#endif
		default:
			throw("is not number", cell);
			break;
		}
	};

	return create_integer(result);
}
#ifdef USE_FLONUM
static const cell_t *flonum_subtract(const long argc, const cell_t **argv,
		const double initial, const long from)
{
	double result = initial;
	for (long i = from; i < argc; i++) {
		const cell_t *cell = argv[i];
		switch (tag_of(cell)) {
		case CELL_INTEGER:
			result -= integer_value_of(cell);
			break;
		case CELL_FLONUM:
			result -= cell->flonum;
			break;
		default:
			throw("is not number", cell);
			break;
		}
	}

	return create_flonum(result);
}
#endif
static const cell_t *lib_multiply(const long argc, const cell_t **argv)
{
	const cell_t *cell = argv[0];
#ifdef USE_FLONUM
	if (tag_of(cell) == CELL_FLONUM)
		return flonum_multiply(argc, argv, cell->flonum, 1);
#endif
	long result = integer_value_of(cell);
	for (long i = 1; i < argc; i++) {//TODO check if args >= 1
		cell = argv[i];
		switch (tag_of(cell)) {
		case CELL_INTEGER:
			result *= integer_value_of(cell);
			break;
#ifdef USE_FLONUM
		case CELL_FLONUM:
			return flonum_multiply(argc, argv, result, i);
#endif
		default:
			throw("is not number", cell);
			break;
		}
	}

	return create_integer(result);
}
#ifdef USE_FLONUM
static const cell_t *flonum_multiply(const long argc, const cell_t **argv,
		const double initial, const long from)
{
	double result = initial;
	for (long i = from; i < argc; i++) {
		const cell_t *cell = argv[i];
		switch (tag_of(cell)) {
		case CELL_INTEGER:
			result *= integer_value_of(cell);
			break;
		case CELL_FLONUM:
			result *= cell->flonum;
			break;
		default:
			throw("is not number", cell);
			break;
		}
	}

	return create_flonum(result);
}
#endif
static const cell_t *lib_divide(const long argc, const cell_t **argv)
{
	const cell_t *cell = argv[0];
	if (argc == 1) {//TODO check if args >= 1
		switch (tag_of(cell)) {
		case CELL_INTEGER:
			return create_integer(1 / integer_value_of(cell));
#ifdef USE_FLONUM
		case CELL_FLONUM:
			return create_flonum(1 / cell->flonum);
#endif
		default:
			throw("is not number", cell);
			break;
		}
	}
#ifdef USE_FLONUM
	if (tag_of(cell) == CELL_FLONUM)
		return flonum_divide(argc, argv, cell->flonum, 1);
#endif
	long result = integer_value_of(cell);
	for (long i = 1; i < argc; i++) {
		cell = argv[i];
		switch (tag_of(cell)) {
		case CELL_INTEGER:
			if (!(integer_value_of(cell)))
				throw("cannot divide", cell);
			result /= integer_value_of(cell);//TODO convert to float if remainder is not 0?
			break;
#ifdef USE_FLONUM
		case CELL_FLONUM:
			return flonum_divide(argc, argv, result, i);
#endif
		default:
			throw("is not number", cell);
			break;
		}
	}

	return create_integer(result);
}
#ifdef USE_FLONUM
static const cell_t *flonum_divide(const long argc, const cell_t **argv,
		const double initial, const long from)
{
	double result = initial;
	for (long i = from; i < argc; i++) {
		const cell_t *cell = argv[i];
		switch (tag_of(cell)) {
		case CELL_INTEGER:
			result /= integer_value_of(cell);
			break;
		case CELL_FLONUM:
			result /= cell->flonum;
			break;
		default:
			throw("is not number", cell);
			break;
		}
	}

	return create_flonum(result);
}
#endif
static const cell_t *lib_truncate_quotient(const long argc, const cell_t **argv)
{
	validate_length(argc, 2);
	validate_integer(argv[0]);//TODO apply float (no fraction)
	validate_integer(argv[1]);

	long n2 = integer_value_of(argv[1]);
	if (!n2)
		throw("cannot divide", argv[1]);

	return create_integer(integer_value_of(argv[0]) / n2);
}

static const cell_t *lib_truncate_remainder(const long argc, const cell_t **argv)
{
	validate_length(argc, 2);
	validate_integer(argv[0]);//TODO apply float (no fraction)
	validate_integer(argv[1]);

	long n2 = integer_value_of(argv[1]);
	if (!n2)
		throw("cannot divide", argv[1]);

	return create_integer(integer_value_of(argv[0]) % n2);
}

static const cell_t *lib_equal(const long argc, const cell_t **argv)
{
	const cell_t *cell = argv[0];
	validate_integer(cell);

	long prev = integer_value_of(cell);
	for (long i = 1; i < argc; i++) {//TODO check if args >= 1
		cell = argv[i];
		validate_integer(cell);

		if (prev != integer_value_of(cell))
			return false_cell;

		prev = integer_value_of(cell);//TODO is not needed?
	}

	return true_cell;
}

static const cell_t *lib_less(const long argc, const cell_t **argv)
{
	const cell_t *cell = argv[0];
	validate_integer(cell);//TODO apply for float

	long prev = integer_value_of(cell);
	for (long i = 1; i < argc; i++) {//TODO check if args >= 1
		cell = argv[i];
		validate_integer(cell);

		if (prev >= integer_value_of(cell))
			return false_cell;

		prev = integer_value_of(cell);
	}

	return true_cell;
}

static const cell_t *lib_greater(const long argc, const cell_t **argv)
{
	const cell_t *cell = argv[0];
	validate_integer(cell);//TODO apply for float

	long prev = integer_value_of(cell);
	for (long i = 1; i < argc; i++) {//TODO check if args >= 1
		cell = argv[i];
		validate_integer(cell);

		if (prev <= integer_value_of(cell))
			return false_cell;

		prev = integer_value_of(cell);
	}

	return true_cell;
}

static const cell_t *lib_less_or_equal(const long argc, const cell_t **argv)
{
	const cell_t *cell = argv[0];
	validate_integer(cell);//TODO apply for float

	long prev = integer_value_of(cell);
	for (long i = 1; i < argc; i++) {//TODO check if args >= 1
		cell = argv[i];
		validate_integer(cell);

		if (prev > integer_value_of(cell))
			return false_cell;

		prev = integer_value_of(cell);
	}

	return true_cell;
}

static const cell_t *lib_greater_or_equal(const long argc, const cell_t **argv)
{
	const cell_t *cell = argv[0];
	validate_integer(cell);//TODO apply for float

	long prev = integer_value_of(cell);
	for (long i = 1; i < argc; i++) {//TODO check if args >= 1
		cell = argv[i];
		validate_integer(cell);

		if (prev < integer_value_of(cell))
			return false_cell;

		prev = integer_value_of(cell);
	}

	return true_cell;
}

static const cell_t *lib_number_string(const long argc, const cell_t **argv)
{
	//TODO support radix
	validate_length(argc, 1);

	char buf[32];//TODO ad-hoc
	const cell_t *cell = argv[0];
	switch (tag_of(cell)) {
	case CELL_INTEGER:
		sprintf(buf, "%ld", integer_value_of(cell));
		break;
#ifdef USE_FLONUM
	case CELL_FLONUM:
		sprintf(buf, "%f", cell->flonum);
		break;
#endif
	default:
		throw("is not number", cell);
		break;
	}

	return create_string(buf);
}

static const cell_t *lib_string_number(const long argc, const cell_t **argv)
{
	//TODO support radix
	validate_length(argc, 1);

	const cell_t *cell = argv[0];
	validate_string(cell);

	char *end;
	long integer = strtol(cell->name, &end, 10);
	if (cell->name[0] && !(*end))
		return create_integer(integer);
#ifdef USE_FLONUM
	double flonum = strtod(cell->name, &end);
	if (cell->name[0] && !(*end))
		return create_flonum(flonum);
#endif
	return false_cell;
}

static const cell_t *lib_string_length(const long argc, const cell_t **argv)
{
	validate_length(argc, 1);

	const cell_t *str = argv[0];
	validate_string(str);

	return create_integer(strlen(str->name));
}

static const cell_t *lib_string_equal(const long argc, const cell_t **argv)
{
	const cell_t *cell = argv[0];
	validate_string(cell);

	const cell_t *prev = cell;
	for (long i = 1; i < argc; i++) {//TODO check if args >= 1
		cell = argv[i];
		validate_string(cell);

		if ((prev->length != cell->length)
				|| strcmp(prev->name, cell->name))
			return false_cell;

		prev = cell;
	}

	return true_cell;
}

static const cell_t *lib_string_copy(const long argc, const cell_t **argv)
{
	//TODO support other parameters
	validate_length(argc, 3);

	const cell_t *str = argv[0];
	validate_string(str);
	long len = strlen(str->name);

	const cell_t *start = argv[1];
	validate_integer(start);
	if ((integer_value_of(start) < 0)
			|| (integer_value_of(start) >= len))
		throw("invalid start", start);

	const cell_t *end = argv[2];
	validate_integer(end);
	if ((integer_value_of(end) < integer_value_of(start))
			|| (integer_value_of(end) > len))
		throw("invalid end", end);

	size_t size = integer_value_of(end) - integer_value_of(start);
	char *buf = (char *) malloc(size + 1);
	if (!buf)
		throw("no memory", NULL);

	memcpy(buf, (void *) ((uintptr_t) (str->name) + integer_value_of(start)), size);
	buf[size] = NUL;
	const cell_t *cell = create_string(buf);
	free(buf);
	return cell;
}

static const cell_t *lib_make_vector(const long argc, const cell_t **argv)
{
	//TODO support fill
	validate_length(argc, 1);

	const cell_t *size = argv[0];
	validate_integer(size);

	return create_vector(integer_value_of(size));
}

static const cell_t *lib_vector_ref(const long argc, const cell_t **argv)
{
	validate_length(argc, 2);

	const cell_t *vector = argv[0];
	validate_vector(vector);

	const cell_t *index = argv[1];
	validate_integer(index);
	long n = integer_value_of(index);
	if (n < 0 || n >= vector->length)
		throw("out of bound", index);

	return vector->vector[n];
}

static const cell_t *lib_vector_set(const long argc, const cell_t **argv)
{
	validate_length(argc, 3);

	const cell_t *vector = argv[0];
	validate_vector(vector);

	const cell_t *index = argv[1];
	validate_integer(index);
	long n = integer_value_of(index);
	if (n < 0 || n >= vector->length)
		throw("out of bound", index);

	vector->vector[n] = argv[2];
	return unspecified_cell;
}

static const cell_t *lib_read_line(const long argc, const cell_t **argv)
{
	//TODO support port
	validate_length(argc, 0);

	size_t size = 32;//TODO define constant
	char *buf = (char *) malloc(size);
	if (!buf)
		throw("no memory", NULL);

	long pos = 0;
	for (;;) {
		int c = fgetc(stdin);
		if (c == EOF)
			//TODO return EOF object
			break;

		if (c == LF) {
			//TODO support CR
			break;
		}

		buf[pos++] = c & 0xff;
		if (pos >= size) {
			size <<= 2;
			char *next_buf = (char *) malloc(size);
			if (!next_buf)
				throw("no memory", NULL);

			memcpy(next_buf, buf, pos);
			free(buf);
			buf = next_buf;
		}
	}

	buf[pos] = NUL;
	const cell_t *cell = create_string(buf);
	free(buf);
	return cell;
}

static const cell_t *lib_write_string(const long argc, const cell_t **argv)
{
	//TODO support port
	validate_length(argc, 1);

	const cell_t *cell = argv[0];
	validate_string(cell);

	fputs(cell->name, stdout);
	return unspecified_cell;
}
#if 0
//TODO process-context library
static _Noreturn const cell_t *lib_exit(const long argc, const cell_t **argv)
{
	switch (argc) {
	case 0:
		exit(EXIT_SUCCESS);
	case 1: {
			const cell_t *cell = argv[0];
			validate_integer(cell);
			exit(integer_value_of(cell));
		}
	default: {
			char buf[256];//TODO ugly
			sprintf(buf, "requires 0 or 1 argument, but %ld", argc);
			throw(buf, null_cell);
		}
	}
}
#endif
//TODO SRFI-27
static const cell_t *lib_random_integer(const long argc, const cell_t **argv)
{
	validate_length(argc, 1);
	validate_integer(argv[0]);//TODO apply for float

	long n2 = integer_value_of(argv[0]);
	if (n2 <= 0)
		throw("requires positive integer", argv[0]);

	int n1;
	if (INT_MAX < n2)//TODO is error? use another generator
		n1 = rand();
	else {
		int max = (INT_MAX / n2) * n2;
		if ((INT_MAX - max) == (n2 - 1))
			n1 = rand();
		else
			do {
				n1 = rand();
			} while (max <= n1);
	}

	return create_integer(n1 % n2);
}
#endif
