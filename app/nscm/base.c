#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "nscm.h"
#ifdef USE_FLONUM
#include <math.h>
#endif

#ifdef USE_BASE
#define PORT_INPUT (1)
#define PORT_OUTPUT (2)
#define PORT_TEXT (4)
#define PORT_MASK (PORT_INPUT | PORT_OUTPUT | PORT_TEXT)

static cell_t *eof_object;

static cell_t *create_vector(const long);
static cell_t *create_bytevector(const long);
static cell_t *create_file(const long, const int);
#if 0
static bool is_eqv(const cell_t *, const cell_t *);
#endif

static void validate_integer(const cell_t *);
#ifdef USE_FLONUM
static void validate_flonum(const cell_t *);
#endif
static void validate_string(const cell_t *);
static void validate_vector(const cell_t *);
static void validate_bytevector(const cell_t *);
static void validate_file(const cell_t *);

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
static const cell_t *lib_make_bytevector(const long, const cell_t **);
static const cell_t *lib_bytevector_ref(const long, const cell_t **);
static const cell_t *lib_bytevector_set(const long, const cell_t **);
static const cell_t *lib_close_port(const long, const cell_t **);
static const cell_t *lib_read_line(const long, const cell_t **);
static const cell_t *lib_is_eof_object(const long, const cell_t **);
static const cell_t *lib_read_bytevector(const long, const cell_t **);
static const cell_t *lib_write_string(const long, const cell_t **);
static const cell_t *lib_write_bytevector(const long, const cell_t **);
static const cell_t *lib_command_line(const long, const cell_t **);
#if 0
static _Noreturn const cell_t *lib_exit(const long, const cell_t **);
#endif
static const cell_t *lib_random_integer(const long, const cell_t **);
static const cell_t *lib_abs(const long, const cell_t **);
#ifdef USE_FLONUM
static double integer_to_flonum(const cell_t *);
static const cell_t *lib_floor(const long, const cell_t **);
static const cell_t *lib_expt(const long, const cell_t **);
static const cell_t *lib_exp(const long, const cell_t **);
static const cell_t *lib_log(const long, const cell_t **);
static const cell_t *lib_sin(const long, const cell_t **);
static const cell_t *lib_cos(const long, const cell_t **);
static const cell_t *lib_tan(const long, const cell_t **);
static const cell_t *lib_atan(const long, const cell_t **);
static const cell_t *lib_sqrt(const long, const cell_t **);
#endif
static const cell_t *lib_open_file(const long, const cell_t **);

static cell_t *create_vector(const long size)
{
	cell_t *cell = create_cell(CELL_VECTOR);
	cell->vector = (const cell_t **) malloc(size * sizeof(cell->vector[0]));
	if (!(cell->vector))
		throw("no memory", NULL);

	cell->length = size;
	for (long i = 0; i < size; i++)
		cell->vector[i] = null_cell;

	return cell;
}

static cell_t *create_bytevector(const long size)
{
	cell_t *cell = create_cell(CELL_BYTEVECTOR);
	cell->bytevector = (uint8_t *) malloc(size * sizeof(cell->bytevector[0]));
	if (!(cell->vector))
		throw("no memory", NULL);

	cell->length = size;
	return cell;
}

static cell_t *create_file(const long mode, const int fd)
{
	cell_t *cell = create_cell(CELL_FILE);
	cell->mode = mode;
	cell->fd = fd;
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
#ifdef USE_FLONUM
static void validate_flonum(const cell_t *arg)
{
	if (tag_of(arg) != CELL_FLONUM)
		throw("not flonum", arg);
}
#endif
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

static void validate_bytevector(const cell_t *arg)
{
	if (tag_of(arg) != CELL_BYTEVECTOR)
		throw("not bytevector", arg);
}

static void validate_file(const cell_t *arg)
{
	if (tag_of(arg) != CELL_FILE)
		throw("not file", arg);
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
	bind_procedure("truncate-quotient", lib_truncate_quotient);//TODO not tiny
	bind_procedure("truncate-remainder", lib_truncate_remainder);//TODO not tiny
	bind_procedure("=", lib_equal);
	bind_procedure("<", lib_less);
	bind_procedure(">", lib_greater);//TODO not(<=)
	bind_procedure("<=", lib_less_or_equal);
	bind_procedure(">=", lib_greater_or_equal);//TODO not(<)
	bind_procedure("number->string", lib_number_string);
	bind_procedure("string->number", lib_string_number);
	bind_procedure("string-length", lib_string_length);//TODO not tiny
	bind_procedure("string=?", lib_string_equal);
	bind_procedure("string-copy", lib_string_copy);//TODO not tiny
	bind_procedure("make-vector", lib_make_vector);
	bind_procedure("vector-ref", lib_vector_ref);
	bind_procedure("vector-set!", lib_vector_set);
	bind_procedure("make-bytevector", lib_make_bytevector);
	bind_procedure("bytevector-u8-ref", lib_bytevector_ref);
	bind_procedure("bytevector-u8-set!", lib_bytevector_set);
	bind_procedure("close-port", lib_close_port);
	bind_procedure("read-line", lib_read_line);
	eof_object = create_symbol("");
	set_global("#eof", eof_object);
	bind_procedure("eof-object?", lib_is_eof_object);
	bind_procedure("read-bytevector!", lib_read_bytevector);
	bind_procedure("write-string", lib_write_string);
	bind_procedure("write-bytevector", lib_write_bytevector);
	bind_procedure("command-line", lib_command_line);
#if 0
	bind_procedure("exit", lib_exit);
#endif
	bind_procedure("random-integer", lib_random_integer);
	srand(time(NULL));
	bind_procedure("abs", lib_abs);//TODO not tiny
#ifdef USE_FLONUM
	bind_procedure("floor", lib_floor);//TODO not tiny
	bind_procedure("expt", lib_expt);//TODO not tiny
	bind_procedure("exp", lib_exp);//TODO not tiny
	bind_procedure("log", lib_log);//TODO not tiny
	bind_procedure("sin", lib_sin);//TODO not tiny
	bind_procedure("cos", lib_cos);//TODO not tiny
	bind_procedure("tan", lib_tan);//TODO not tiny
	bind_procedure("atan", lib_atan);//TODO not tiny
	bind_procedure("sqrt", lib_sqrt);//TODO not tiny
#endif
	set_global("binary-input", create_integer(PORT_INPUT));
	set_global("textual-input", create_integer(PORT_TEXT | PORT_INPUT));
	set_global("binary-output", create_integer(PORT_OUTPUT));
	set_global("textual-output", create_integer(PORT_TEXT | PORT_OUTPUT));
	set_global("binary-input/output", create_integer(PORT_INPUT | PORT_OUTPUT));
	set_global("open/append", create_integer(O_APPEND));
	set_global("open/create", create_integer(O_CREAT));
	set_global("open/truncate", create_integer(O_TRUNC));
	bind_procedure("open-file", lib_open_file);
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
#ifdef USE_FLONUM
	if (tag_of(cell) == CELL_FLONUM) {
		double prev = cell->flonum;
		for (long i = 1; i < argc; i++) {//TODO check if args >= 1
			cell = argv[i];
			validate_flonum(cell);

			if (prev != cell->flonum)
				return false_cell;

			prev = cell->flonum;
		}

		return true_cell;
	}
#endif
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
#ifdef USE_FLONUM
	if (tag_of(cell) == CELL_FLONUM) {
		double prev = cell->flonum;
		for (long i = 1; i < argc; i++) {//TODO check if args >= 1
			cell = argv[i];
			validate_flonum(cell);

			if (prev >= cell->flonum)
				return false_cell;

			prev = cell->flonum;
		}

		return true_cell;
	}
#endif
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
#ifdef USE_FLONUM
	if (tag_of(cell) == CELL_FLONUM) {
		double prev = cell->flonum;
		for (long i = 1; i < argc; i++) {//TODO check if args >= 1
			cell = argv[i];
			validate_flonum(cell);

			if (prev <= cell->flonum)
				return false_cell;

			prev = cell->flonum;
		}

		return true_cell;
	}
#endif
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
#ifdef USE_FLONUM
	if (tag_of(cell) == CELL_FLONUM) {
		double prev = cell->flonum;
		for (long i = 1; i < argc; i++) {//TODO check if args >= 1
			cell = argv[i];
			validate_flonum(cell);

			if (prev > cell->flonum)
				return false_cell;

			prev = cell->flonum;
		}

		return true_cell;
	}
#endif
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
#ifdef USE_FLONUM
	if (tag_of(cell) == CELL_FLONUM) {
		double prev = cell->flonum;
		for (long i = 1; i < argc; i++) {//TODO check if args >= 1
			cell = argv[i];
			validate_flonum(cell);

			if (prev < cell->flonum)
				return false_cell;

			prev = cell->flonum;
		}

		return true_cell;
	}
#endif
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

static const cell_t *lib_make_bytevector(const long argc, const cell_t **argv)
{
	//TODO support fill
	validate_length(argc, 1);

	const cell_t *size = argv[0];
	validate_integer(size);

	return create_bytevector(integer_value_of(size));
}

static const cell_t *lib_bytevector_ref(const long argc, const cell_t **argv)
{
	validate_length(argc, 2);

	const cell_t *vector = argv[0];
	validate_bytevector(vector);

	const cell_t *index = argv[1];
	validate_integer(index);
	long n = integer_value_of(index);
	if (n < 0 || n >= vector->length)
		throw("out of bound", index);

	return create_integer(vector->bytevector[n]);
}

static const cell_t *lib_bytevector_set(const long argc, const cell_t **argv)
{
	validate_length(argc, 3);

	const cell_t *vector = argv[0];
	validate_bytevector(vector);

	const cell_t *index = argv[1];
	validate_integer(index);
	long n = integer_value_of(index);
	if (n < 0 || n >= vector->length)
		throw("out of bound", index);

	validate_integer(argv[2]);
	vector->bytevector[n] = integer_value_of(argv[2]) & 0xff;//TODO check range?
	return unspecified_cell;
}

static const cell_t *lib_close_port(const long argc, const cell_t **argv)
{
	validate_length(argc, 1);

	cell_t *cell = (cell_t *) (argv[0]);
	validate_file(cell);

	if (cell->mode & (PORT_INPUT | PORT_OUTPUT)) {
		close(cell->fd);
		cell->mode = 0;
		cell->fd = -1;
	}

	return unspecified_cell;
}

static const cell_t *lib_is_eof_object(const long argc, const cell_t **argv)
{
	validate_length(argc, 1);

	return ((argv[0] == eof_object) ? true_cell : false_cell);
}

static const cell_t *lib_read_line(const long argc, const cell_t **argv)
{
	int fd;
	switch (argc) {
	case 0:
		fd = STDIN_FILENO;
		break;
	case 1:
		validate_file(argv[0]);
		if ((argv[0]->mode & (PORT_INPUT | PORT_TEXT))
				!= (PORT_INPUT | PORT_TEXT))
			throw("bad port", argv[0]);

		fd = argv[0]->fd;
		break;
	default:
		throw("too many arguments", NULL);
	}

	size_t size = 32;//TODO define constant
	char *buf = (char *) malloc(size);
	if (!buf)
		throw("no memory", NULL);

	long pos = 0;
	for (;;) {
		char c;
		if (read(fd, &c, 1) != 1) {//TODO throw on error
			if (pos)
				break;
			else {
				free(buf);
				return eof_object;
			}
		}

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

static const cell_t *lib_read_bytevector(const long argc, const cell_t **argv)
{
	validate_length(argc, 4);
	validate_bytevector(argv[0]);

	validate_file(argv[1]);
	if ((argv[1]->mode & (PORT_INPUT | PORT_TEXT)) != PORT_INPUT)
		throw("bad port", argv[1]);

	validate_integer(argv[2]);
	int start = integer_value_of(argv[2]);
	if ((start < 0)
			|| (argv[0]->length <= start))
		throw("invalid start", argv[2]);

	validate_integer(argv[3]);
	int end = integer_value_of(argv[3]);
	if ((end < start)
			|| (argv[0]->length < end))
		throw("invalid end", argv[3]);

	ssize_t len = end - start;
	if (!len)
		return create_integer(0);

	len = read(argv[1]->fd, &(argv[0]->bytevector[start]), len);
	if (len == (-1))
		throw("read error", argv[1]);

	return (len ? create_integer(len) : eof_object);
}

static const cell_t *lib_write_string(const long argc, const cell_t **argv)
{
	int fd;
	const cell_t *cell;

	switch (argc) {
	case 0:
		throw("no argument", NULL);
	case 1:
		fd = STDOUT_FILENO;
		cell = argv[0];
		break;
	case 2:
		validate_file(argv[0]);
		if ((argv[0]->mode & (PORT_OUTPUT | PORT_TEXT))
				!= (PORT_OUTPUT | PORT_TEXT))
			throw("bad port", argv[0]);

		fd = argv[0]->fd;
		cell = argv[1];
		break;
	default:
		throw("too many arguments", NULL);
	}

	validate_string(cell);

	for (char *p = cell->name; *p; p++)
		write(fd, p, 1);//TODO throw on error

	return unspecified_cell;
}

static const cell_t *lib_write_bytevector(const long argc, const cell_t **argv)
{
	validate_length(argc, 4);
	validate_bytevector(argv[0]);

	validate_file(argv[1]);
	if ((argv[1]->mode & (PORT_OUTPUT | PORT_TEXT)) != PORT_OUTPUT)
		throw("bad port", argv[1]);

	validate_integer(argv[2]);
	int start = integer_value_of(argv[2]);
	if ((start < 0)
			|| (argv[0]->length <= start))
		throw("invalid start", argv[2]);

	validate_integer(argv[3]);
	int end = integer_value_of(argv[3]);
	if ((end < start)
			|| (argv[0]->length < end))
		throw("invalid end", argv[3]);

	ssize_t len = end - start;
	if (len) {
		len = write(argv[1]->fd, &(argv[0]->bytevector[start]), len);
		if (len != (end - start))
			throw("write error", argv[1]);
	}

	return unspecified_cell;
}

//TODO process-context library
static const cell_t *lib_command_line(const long argc, const cell_t **argv)
{
	validate_length(argc, 0);
	return command_line;
}
#if 0
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

static const cell_t *lib_abs(const long argc, const cell_t **argv)
{
	validate_length(argc, 1);

	const cell_t *cell = argv[0];
	if (tag_of(cell) == CELL_INTEGER) {
		return create_integer(labs(integer_value_of(cell)));
	} else {
#ifdef USE_FLONUM
		validate_flonum(cell);
		return create_flonum(fabs(cell->flonum));
#else
		throw("not integer", cell);
#endif
	}
}

#ifdef USE_FLONUM
static double integer_to_flonum(const cell_t *cell)
{
	if (tag_of(cell) == CELL_INTEGER) {
		return (double) integer_value_of(cell);
	} else {
		validate_flonum(cell);//TODO support complex
		return cell->flonum;
	}
}

static const cell_t *lib_floor(const long argc, const cell_t **argv)
{
	validate_length(argc, 1);

	const cell_t *cell = argv[0];
	if (tag_of(cell) == CELL_INTEGER)
		return cell;

	validate_flonum(cell);
	return create_integer(floor(cell->flonum));
}

static const cell_t *lib_expt(const long argc, const cell_t **argv)
{
	validate_length(argc, 2);

	double arg1 = integer_to_flonum(argv[0]);
	double arg2 = integer_to_flonum(argv[1]);
	return create_flonum(pow(arg1, arg2));//TODO support integer
}

//TODO inexact library
static const cell_t *lib_exp(const long argc, const cell_t **argv)
{
	validate_length(argc, 1);

	return create_flonum(exp(integer_to_flonum(argv[0])));
}

static const cell_t *lib_log(const long argc, const cell_t **argv)
{
	validate_length(argc, 1);

	return create_flonum(log(integer_to_flonum(argv[0])));
}

static const cell_t *lib_sin(const long argc, const cell_t **argv)
{
	validate_length(argc, 1);

	return create_flonum(sin(integer_to_flonum(argv[0])));
}

static const cell_t *lib_cos(const long argc, const cell_t **argv)
{
	validate_length(argc, 1);

	return create_flonum(cos(integer_to_flonum(argv[0])));
}

static const cell_t *lib_tan(const long argc, const cell_t **argv)
{
	validate_length(argc, 1);

	return create_flonum(tan(integer_to_flonum(argv[0])));
}

static const cell_t *lib_atan(const long argc, const cell_t **argv)
{
	validate_length(argc, 1);

	return create_flonum(atan(integer_to_flonum(argv[0])));
}

static const cell_t *lib_sqrt(const long argc, const cell_t **argv)
{
	validate_length(argc, 1);

	return create_flonum(sqrt(integer_to_flonum(argv[0])));
}
#endif
//TODO SRFI-170
static const cell_t *lib_open_file(const long argc, const cell_t **argv)
{
	//TODO support permission-bits
	//TODO support buffer-mode
	validate_length(argc, 3);
	validate_string(argv[0]);
	validate_integer(argv[1]);
	validate_integer(argv[2]);

	int mode = integer_value_of(argv[1]);
	if ((mode & PORT_MASK) != mode)
		throw("bad mode", NULL);

	int posix_mode;
	switch (mode & (PORT_INPUT | PORT_OUTPUT)) {
	case PORT_INPUT:
		posix_mode = O_RDONLY;
		break;
	case PORT_OUTPUT:
		posix_mode = O_WRONLY;
		break;
	case (PORT_INPUT | PORT_OUTPUT):
		posix_mode = O_RDWR;
		break;
	default:
		throw("bad mode", NULL);
	}

	int fd = open(argv[0]->name, posix_mode | integer_value_of(argv[2]), 0666);
	if (fd == (-1))
		throw("cannot open", argv[0]);

	return create_file(mode, fd);
}
#endif
