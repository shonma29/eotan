#include "nscm.h"

#ifdef USE_REPL

static void display(FILE *, const cell_t *);
static void display_list(FILE *, const cell_t *);


void print(FILE *port, const cell_t *cell)
{
	display(port, cell);
	fputc(LF, port);
}

static void display(FILE *out, const cell_t *cell)
{
	switch (tag_of(cell)) {
	case CELL_SYMBOL:
	case CELL_STRING:
		fputs(cell->name, out);
		break;
	case CELL_PAIR:
		display_list(out, cell);
		break;
	case CELL_SYNTAX:
		fputs("<syntax>", out);
		break;
	case CELL_PROCEDURE:
		fputs("<procedure>", out);
		break;
	case CELL_LAMBDA: {
			fputs("<lambda> ", out);

			const cell_t *lambda = cell->lambda;
			display(out, car(lambda));
			fputc(' ', out);
			display(out, cadr(lambda));
		}
		break;
	case CELL_BOOLEAN:
		fprintf(out, "%s", cell->boolean? "#t" : "#f");
		break;
	case CELL_INTEGER:
		fprintf(out, "%ld", integer_value_of(cell));
		break;
#ifdef USE_FLONUM
	case CELL_FLONUM:
		fprintf(out, "%f", cell->flonum);
		break;
#endif
	case CELL_VECTOR: {
			fputc('{', out);

			for (long i = 0; i < cell->length; i++)
				display(out, cell->vector[i]);

			fputc('}', out);
		}
		break;
	default:
		fputs("<unknown>", out);
		break;
	}
}

static void display_list(FILE *out, const cell_t *list)
{
	fputc('(', out);

	for (const cell_t *pair = list; !is_null(pair);) {
		display(out, car(pair));

		const cell_t *cdr_cell = cdr(pair);
		if (is_null(cdr_cell))
			break;
		else if (tag_of(cdr_cell)== CELL_PAIR) {
			fputc(' ', out);
			pair = cdr_cell;
		} else {
			fputs(" . ", out);
			display(out, cdr_cell);
			break;
		}
	}

	fputc(')', out);
}
#endif
