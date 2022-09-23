#include "nscm.h"
#include "vm.h"

#ifdef USE_REPL

#ifdef REPL_SHOW_DEBUG
static long code_depth;
#endif

static void display(FILE *, const cell_t *);
static void display_pair(FILE *, const cell_t *);
static void display_module_name(FILE *, const cell_t *);

#ifdef REPL_SHOW_DEBUG
static long display_opecode(FILE *, const cell_t **, long);
#endif

void print(FILE *port, const cell_t *cell)
{
	if (cell != unspecified_cell) {
#ifdef REPL_SHOW_DEBUG
		code_depth = 0;
#endif
		display(port, cell);
		fputc(LF, port);
	}
}

static void display(FILE *out, const cell_t *cell)
{
	switch (tag_of(cell)) {
	case CELL_CODE: {
			fputs("{code ", out);
			display_module_name(out, cell);
			fputc(' ', out);
#ifdef REPL_SHOW_DEBUG
			if (++code_depth == 1)
				for (long i = 0; i < cell->length; i++) {
					if (i)
						fprintf(out, "\n%5.5ld: ", i);

					if (tag_of(cell->vector[i]) == CELL_CODE) {
						fprintf(out, "<code %p ", cell->vector[i]);
						display(out, cell->vector[i]->vector[OFFSET_VARS]);
						fputc('>', out);
					} else if (i >= OFFSET_OPCODE)
						i = display_opecode(out, cell->vector, i);
					else
						display(out, cell->vector[i]);
				}

			code_depth--;
#else
			display(out, cell->vector[OFFSET_VARS]);
#endif
			fputc('}', out);
		}
		break;
	case CELL_INTEGER:
		fprintf(out, "%ld", integer_value_of(cell));
		break;
#ifdef REPL_SHOW_DEBUG
	case CELL_REFER_RELATIVE: {
			long offset = integer_value_of(cell);
			fprintf(out, "%ld(%s)",
					offset >> RELATIVE_BITS,
					(offset & RELATIVE_MASK) ? "%ep" : "%sp");
		}
		break;
#endif
	case CELL_PROCEDURE: {
			fprintf(out, (cell->syntax) ? "<syntax " : "<procedure ");
			display_module_name(out, cell);
			fputc('>', out);
		}
		break;
	case CELL_LAMBDA:
			fputs("<lambda> ", out);
			display(out, cell->code);
			fputc(' ', out);
			display(out, cell->env);
		break;
	case CELL_BOOLEAN:
		fprintf(out, "%s", cell->boolean? "#t" : "#f");
		break;
#ifdef USE_FLONUM
	case CELL_FLONUM:
		fprintf(out, "%f", cell->flonum);
		break;
#endif
	case CELL_STRING:
	case CELL_SYMBOL:
		fputs(cell->name, out);
		break;
	case CELL_VECTOR: {
			fprintf(out, "#(");

			for (long i = 0; i < cell->length; i++) {
				if (i)
					fputc(' ', out);

				display(out, cell->vector[i]);
			}

			fputc(')', out);
		}
		break;
	case CELL_BYTEVECTOR:
		fputs("<bytevector>", out);
		break;
	case CELL_FILE:
		fputs("<file>", out);
		break;
	case CELL_REFER_INDIRECT:
	case CELL_PAIR:
		display_pair(out, cell);
		break;
	default:
#ifdef REPL_SHOW_DEBUG
		fprintf(out, "<unknown %ld:%p>", tag_of(cell), cell);
#else
		fprintf(out, "<unknown>");
#endif
		break;
	}
}

static void display_pair(FILE *out, const cell_t *list)
{
	fputc('(', out);

	if (list_length(list) >= 0)
		for (const cell_t *pair = list; !is_null(pair);) {
			display(out, car(pair));

			const cell_t *cdr_cell = cdr(pair);
			if (is_null(cdr_cell))
				break;
			else if ((tag_of(cdr_cell) == CELL_PAIR)
					|| (tag_of(cdr_cell) == CELL_REFER_INDIRECT)) {
				fputc(' ', out);
				pair = cdr_cell;
			} else {
				fputs(" . ", out);
				display(out, cdr_cell);
				break;
			}
		}
	else {
		display(out, car(list));
		fputs(" . ", out);
		display(out, cdr(list));
	}

	fputc(')', out);
}

static void display_module_name(FILE *out, const cell_t *cell) {
	const cell_t *symbol = find_symbol_by_procedure(cell);
	if (symbol)
		fprintf(out, "\"%s\"", symbol->name);
	else
		fprintf(out, "%p", cell);
}

#ifdef REPL_SHOW_DEBUG
typedef struct op_entry {
	char *name;
	bool has_param;
	bool param_is_cell;
} op_entry_t;

static op_entry_t op_table[] = {
	{ "load", true, true },
	{ "load_relative", true, true },
	{ "store", true, false },
	{ "cdr", true, true },
	{ "bf", true, false },
	{ "bra", true, false },
	{ "create_frame", true, false },
	{ "call", false, false },
	{ "tail", false, false },
	{ "ret", false, false },
	{ "create_lambda", true, true },
	{ "to_list", false, false },
#ifdef USE_BASE
	{ "addi", true, false },
	{ "subi", true, false },
	{ "muli", true, false },
	{ "divi", true, false }
#endif
};
static op_entry_t op_unknown = { "unknown", false, false };

static long display_opecode(FILE *out, const cell_t **code, long index) {
	long op_code = vm_get_opcode(code[index]);
	op_entry_t *entry = (op_code >= 0) ? (&op_table[op_code]) : (&op_unknown);
	if (entry->has_param) {
		index++;
		if (entry->param_is_cell) {
			fprintf(out, "%s ", entry->name);
			display(out, (code[index]));
		} else
			fprintf(out, "%s $%ld", entry->name, (long) (code[index]));
	} else
		fprintf(out, "%s", entry->name);

	return index;
}
#endif
#endif
