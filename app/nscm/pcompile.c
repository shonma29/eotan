#include <string.h>
#include "nscm.h"
#include "vm.h"

typedef struct codebuf {
	cell_t *cell;
	size_t size;
	size_t len;
} codebuf_t;

typedef struct precontext {
	codebuf_t code;
	codebuf_t data;
	codebuf_t indirect;
} precontext_t;

typedef struct _frame {
	const cell_t *vars;
	size_t offset;//TODO use off_t?
} frame_t;

static frame_t frame_stack[PCOMPILE_STACK_SIZE];
static frame_t *frame_top;
static const void **vm_optable;

#define NUM_OF_FRAME (sizeof(frame_stack) / sizeof(frame_stack[0]))

#define FOUND_IN_PARENT (-2)

static inline cell_t *create_refer_relative(const long offset, const long reg)
{
	return ((cell_t *) ((((offset << RELATIVE_BITS) | reg) << FIX_TAG_BITS)
			| CELL_REFER_RELATIVE));
}

static const long find_local(long *, const cell_t *);
static long get_index(const cell_t *, const cell_t *);

static const cell_t *create_opcode(const long);
static void codebuf_create(codebuf_t *, const size_t);
static const long codebuf_append(codebuf_t *, const cell_t *);
static bool codebuf_exists(codebuf_t *, const cell_t *);
static void codebuf_finish(codebuf_t *);
static bool is_data(const cell_t *);
static void code_initialize(precontext_t *);
static const long code_append(precontext_t *, const cell_t *);
static const long code_append_operand(precontext_t *, const long);
static void indirect_append(precontext_t *, const cell_t *);
static void code_finish(precontext_t *);
static void push_frame(const cell_t *, const size_t);
static void pop_frame(void);
static const cell_t *walk(precontext_t *context, cell_t **, cell_t *, const bool);
static const long get_executable_tag(const cell_t *);
static const cell_t *item(long *, cell_t **, const cell_t *);
static const cell_t *lambda(cell_t **, const cell_t *);
static const cell_t *to_vector(cell_t *);
static const cell_t *to_closure(precontext_t *context, cell_t **, cell_t *);


static const long find_local(long *sp_offset, const cell_t *symbol)
{
	bool is_self = true;
	long top = frame_top->offset;
	for (const frame_t *p = frame_top;; p++) {
		// skip frames for C procedure
		for (; !(p->vars); p++)
			if (!(p->offset))
				return (-1);

		long index = get_index(p->vars, symbol);
		if (index >= 0) {
			*sp_offset = top - p->offset;
			return (is_self ?
					(top - p->offset + index + OFFSET_ARGV) : FOUND_IN_PARENT);
		}

		is_self = false;
	}
}

static long get_index(const cell_t *vars, const cell_t *symbol)
{
	if (vars->tag == CELL_SYMBOL) {
		if (vars == symbol)
			return 0;
	} else {//TODO check if CODE
		for (long index = 0; index < vars->length; index++)
			if (vars->vector[index] == symbol)
				return index;
	}

	return (-1);
}

void pcompile_initialize(const void **optable)
{
	vm_optable = optable;
	frame_stack[NUM_OF_FRAME - 1].vars = NULL;
	frame_stack[NUM_OF_FRAME - 1].offset = 0;
}

static const cell_t *create_opcode(const long opcode)
{
	return ((cell_t *) ((opcode << FIX_TAG_BITS) | CELL_OPCODE));
}

static void codebuf_create(codebuf_t *buf, const size_t size)
{
	cell_t *p = create_vector(size * sizeof(buf->cell->vector[0]));
	if (!p)
		throw("no memory", NULL);

	buf->size = size;
	buf->len = 0;
	buf->cell = p;
}

static const long codebuf_append(codebuf_t *buf, const cell_t *cell)
{
	cell_t *p = buf->cell;

	if (buf->size <= buf->len) {
		buf->size *= 2;//TODO check overflow (or max)
		p->vector = (const cell_t **) nrealloc(p->vector,
				buf->size * sizeof(p->vector[0]));
		if (!(p->vector))
			throw("no memory", NULL);
	}

	const long offset = buf->len;
	p->vector[offset] = cell;
	buf->len++;
	return offset;
}

static bool codebuf_exists(codebuf_t *buf, const cell_t *cell)
{
	cell_t **vector = (cell_t **) (buf->cell->vector);
	long max = buf->len;
	for (long i = 0; i < max; i++)
		if (vector[i] == cell)
			return true;

	return false;
}

static void codebuf_finish(codebuf_t *buf)
{
	if (buf->len) {
		cell_t *p = buf->cell;
		p->length = buf->len;
		p->vector = nrealloc(p->vector, buf->len * sizeof(p->vector[0]));
		if (!(p->vector))
			throw("no memory", NULL);
	} else {
		destroy_cell(buf->cell);
		buf->cell = empty_vector_cell;
	}
}

static bool is_data(const cell_t *cell)
{
	if (cell != null_cell
			&& cell != empty_vector_cell) {
		//TODO ugly。can check via bits? (CELL_LAMBDA<=CELL_BOOLEAN)
		switch (tag_of(cell)) {
		case CELL_INTEGER:
		case CELL_OPCODE:
		case CELL_REFER_RELATIVE:
		case CELL_REFER_INDIRECT:
		case CELL_PROCEDURE:
		case CELL_BOOLEAN:
			break;
		default:
			return true;
		}
	}
	return false;
}

static void code_initialize(precontext_t *context)
{
	codebuf_create(&(context->code), INITIAL_CODEBUF_SIZE);
	context->code.cell->tag = CELL_CODE;
	codebuf_create(&(context->data), INITIAL_CODEBUF_SIZE);
	codebuf_create(&(context->indirect), INITIAL_CODEBUF_SIZE);

	code_append(context, null_cell);
	code_append(context, empty_vector_cell);
	code_append(context, empty_vector_cell);
	code_append(context, empty_vector_cell);
}

static const long code_append(precontext_t *context, const cell_t *cell)
{
	const long offset = codebuf_append(&(context->code),
			(get_fix_tag(cell) == CELL_OPCODE) ?
					vm_optable[integer_value_of(cell)] : cell);

	if (is_data(cell)
			&& !codebuf_exists(&(context->data), cell))
		codebuf_append(&(context->data), cell);

	return offset;
}

static const long code_append_operand(precontext_t *context, const long value)
{
	//TODO check overflow
	return codebuf_append(&(context->code), (cell_t *) value);
}

static void indirect_append(precontext_t *context, const cell_t *cell)
{
	//TODO check overflow
	if (!codebuf_exists(&(context->indirect), cell))
		codebuf_append(&(context->indirect), cell);
}

static void code_finish(precontext_t *context)
{
	//TODO omit 'ret' when tail recursion
	code_append(context, create_opcode(OP_RET));

	codebuf_finish(&(context->code));
	codebuf_finish(&(context->data));
	context->code.cell->vector[OFFSET_DATA] = context->data.cell;
	codebuf_finish(&(context->indirect));
	context->code.cell->vector[OFFSET_INDIRECT] = context->indirect.cell;
}

const cell_t *pcompile(const cell_t *cell)
{
	precontext_t context;
	frame_top = &(frame_stack[NUM_OF_FRAME - 1]);

	//TODO execute 'import' first (without repl)
	//TODO support namespace for 'import'
	code_initialize(&context);

	cell_t *env = null_cell;
	walk(&context, &env, (cell_t *) cell, false);
	if (!is_null(env))
		throw("cannot allocate closure", env);

	code_finish(&context);
	return context.code.cell;
}

static void push_frame(const cell_t *vars, const size_t argc)
{
	if ((uintptr_t) frame_top <= (uintptr_t) frame_stack)
		throw("frame overflow", NULL);

	long offset = frame_top->offset;
	frame_top--;
	frame_top->vars = vars;
	//TODO check overflow
	frame_top->offset = offset + words_of_frame(argc);
}

static void pop_frame(void)
{
	if (!(frame_top->offset))
		throw("frame underflow", NULL);

	frame_top++;
}

//TODO use 'const cell_t *' if not replace
static const cell_t *walk(precontext_t *context, cell_t **env, cell_t *cell,
		const bool tail)
{
	if (cell == null_cell)
		throw("cannot evaluate", cell);

	if (tag_of(cell) == CELL_PAIR) {
		if (list_length(cell) > 0) {
			//TODO inline frame
			const cell_t *callee = walk(context, env, (cell_t *) car(cell), false);
			const long callee_tag = get_executable_tag(callee);
			const cell_t *args = cdr(cell);
			if (is_procedure(callee_tag)) {
				//TODO execute by handler
				// pure syntax
				if (callee->procedure == lib_quote) {
					context->code.len -= 2;//TODO ugly
					validate_length(list_length(args), 1);
					code_append(context, create_opcode(OP_LOAD));
					code_append(context, car(args));
					return car(args);
				}
				if (callee->procedure == lib_import) {
					context->code.len -= 2;//TODO ugly
					return NULL;
				}
				if (callee->procedure == lib_if) {
					//TODO support 'if without else'
					//TODO support 'cond'
					context->code.len -= 2;//TODO ugly
					validate_length(list_length(args), 3);//TODO allow 2 parameters?
					walk(context, env, (cell_t *) car(args), false);
					code_append(context, create_opcode(OP_BF));

					long bfOffset = code_append_operand(context, 0);
					args = (cell_t *) cdr(args);
					walk(context, env, (cell_t *) car(args), tail);
					code_append(context, create_opcode(OP_BRA));

					long braOffset = code_append_operand(context, 0);
					walk(context, env, (cell_t *) cadr(args), tail);
					context->code.cell->vector[bfOffset] =
							(const cell_t *) (braOffset - bfOffset + 2);
					//TODO replace 'bra' to 'ret' if tail
					context->code.cell->vector[braOffset] =
							(const cell_t *) (context->code.len - braOffset + 1);
					return NULL;
				}
				if (callee->procedure == lib_lambda) {
					long argc = list_length(args);
					if (argc < 2) {
						//TODO unify validate_length. or define common message
						char buf[256];//TODO ugly
						sprintf(buf, "requires %ld argument, but %ld", (long) 2, argc);
						throw(buf, cell);
					}

					cell_t *closure_request = null_cell;
					cell = (cell_t *) lambda(&closure_request, (cell_t *) args);
					context->code.len -= 2;//TODO ugly
					if (is_null(closure_request))
						code_append(context, create_opcode(OP_LOAD));
					else {
						cell->vector[OFFSET_CLOSURE] =
								to_closure(context, env, closure_request);
						code_append(context, create_opcode(OP_CREATE_LAMBDA));
					}

					code_append(context, cell);
					return cell;
				}
#ifdef USE_BASE
				if (callee->procedure == lib_add) {
					if ((list_length(args) == 2)
							&& tag_of(cadr(args)) == CELL_INTEGER) {
						walk(context, env, (cell_t *) car(args), false);
						code_append(context, create_opcode(OP_ADDI));
						code_append_operand(context, integer_value_of(cadr(args)));
						return NULL;
					}
				}
				if (callee->procedure == lib_subtract) {
					if ((list_length(args) == 2)
							&& tag_of(cadr(args)) == CELL_INTEGER) {
						walk(context, env, (cell_t *) car(args), false);
						code_append(context, create_opcode(OP_SUBI));
						code_append_operand(context, integer_value_of(cadr(args)));
						return NULL;
					}
				}
				if (callee->procedure == lib_multiply) {
					if ((list_length(args) == 2)
							&& tag_of(cadr(args)) == CELL_INTEGER) {
						walk(context, env, (cell_t *) car(args), false);
						code_append(context, create_opcode(OP_MULI));
						code_append_operand(context, integer_value_of(cadr(args)));
						return NULL;
					}
				}
				if (callee->procedure == lib_divide) {
					if ((list_length(args) == 2)
							&& tag_of(cadr(args)) == CELL_INTEGER) {
						walk(context, env, (cell_t *) car(args), false);
						code_append(context, create_opcode(OP_DIVI));
						code_append_operand(context, integer_value_of(cadr(args)));
						return NULL;
					}
				}
#endif
			}

			long offset = 0;
			const long argc = list_length(args);
			push_frame(NULL, argc);
			code_append(context, create_opcode(OP_CREATE_FRAME));
			code_append_operand(context, argc);

			if (is_procedure(callee_tag)) {
				// library functions
				if (callee->procedure == lib_define) {
					validate_length(argc, 2);
					const cell_t *symbol = car(args);
					validate_symbol(symbol);
					//TODO split syntax map, and check here
					//TODO can set value here?
					const cell_t *p = item(NULL, env, (cell_t *) symbol);
					code_append(context, create_opcode(OP_LOAD));
					code_append(context, p);
					code_append(context, create_opcode(OP_STORE));//TODO op fusion
					code_append_operand(context, 2);
					offset = 1;//TODO ugly
					args = cdr(args);
				} else if (callee->procedure == lib_set) {
					validate_length(argc, 2);
					const cell_t *symbol = car(args);
					validate_symbol(symbol);

					const cell_t *p = item(NULL, env, (cell_t *) symbol);
					code_append(context, create_opcode(OP_LOAD));
					code_append(context,
							(tag_of(p) == CELL_REFER_RELATIVE) ?
									(const cell_t *) ((((uintptr_t) p) & (~MASK_TAG_OF_POINTER)) | CELL_INTEGER) : p);
					code_append(context, create_opcode(OP_STORE));//TODO op fusion
					code_append_operand(context, 2);
					offset = 1;//TODO ugly
					args = cdr(args);
				}
			}

			for (cell_t *a = (cell_t *) args; !is_null(a); a = (cell_t *) cdr(a)) {
				walk(context, env, (cell_t *) car(a), false);
				code_append(context, create_opcode(OP_STORE));
				//TODO return what is done for op fusion
				code_append_operand(context, 2 + offset);
				offset++;
			}

			code_append(context, create_opcode(tail ? OP_TAIL : OP_CALL));
			pop_frame();
			return NULL;
		} else
			throw("cannot evaluate", cell);
	} else {
		const cell_t *value = item(NULL, env, cell);
		long opcode;
		switch (tag_of(value)) {
		case CELL_REFER_RELATIVE:
			opcode = OP_LOAD_RELATIVE;
			break;
		case CELL_REFER_INDIRECT:
			opcode = OP_CDR;
			break;
		default:
			opcode = OP_LOAD;
			break;
		}
		code_append(context, create_opcode(opcode));
		code_append(context, value);
		return value;
	}
}

static const long get_executable_tag(const cell_t *callee)
{
	const long tag = callee ? tag_of(callee) : CELL_CODE;
	switch (tag) {
	case CELL_CODE:
	case CELL_REFER_RELATIVE:
	case CELL_REFER_INDIRECT:
	case CELL_PROCEDURE:
		break;
	default:
		throw("cannot evaluate", callee);
	}

	return tag;
}

static const cell_t *item(long *sp_offset, cell_t **env, const cell_t *cell)
{
	if (is_null(cell))
		return cell;

	if (tag_of(cell) == CELL_SYMBOL) {
		long sub_offset;
		const long offset = find_local(&sub_offset, cell);
		if (offset >= 0) {
			if (sp_offset)
				*sp_offset = sub_offset;

			return create_refer_relative(offset, RELATIVE_SP);
		}

		if (offset == FOUND_IN_PARENT) {
			long index = 0;
			cell_t *p = *env;
			if (is_null(p))
				*env = (cell_t *) create_pair(cell, null_cell);
			else
				for (; car(p) != cell; p = (cell_t *) cdr(p)) {
					index++;
					if (is_null(cdr(p))) {
						p->rest = create_pair(cell, null_cell);
						break;
					}
				}

			return create_refer_relative(index, RELATIVE_EP);
		}

		const cell_t *module = find_module(cell);
		if (module)
			return cdr(module);

		const cell_t *pair = find_global(cell);
		if (!pair)
			pair = set_global(cell->name, unspecified_cell);//TODO ugly

		if (pair)
			return pair;
	}

	return cell;
}

static const cell_t *lambda(cell_t **env, const cell_t *cell)
{
	precontext_t context;
	code_initialize(&context);

	const cell_t *vars = car(cell);
	if (is_null(vars) || tag_of(vars) == CELL_PAIR) {
		//TODO split to another function
		long len = 0;
		for (const cell_t *p = vars; !is_null(p); p = cdr(p))
			if (tag_of(car(p)) == CELL_SYMBOL)
				len++;
			else
				throw("not symbol", car(p));

		vars = to_vector((cell_t *) vars);
		//TODO store only length if not display vars in stack trace
		context.code.cell->vector[OFFSET_VARS] = vars;
		push_frame(vars, len);
	} else if (tag_of(vars) == CELL_SYMBOL) {
		code_append(&context, create_opcode(OP_TO_LIST));
		push_frame(vars, 1);
	} else
		throw("not symbol and list", vars);

	for (const cell_t *p = cdr(cell); !is_null(p); p = cdr(p))
		walk(&context, env, (cell_t *) car(p), is_null(cdr(p)));

	pop_frame();
	code_finish(&context);
	return context.code.cell;
}

static const cell_t *to_vector(cell_t *cell)
{
	long len = list_length(cell);
	cell_t *code = create_vector(len);

	if (len) {
		const cell_t *p = cell;
		for (long i = 0; i < len; i++) {
			code->vector[i] = car(p);
			p = cdr(p);
		}
	}

	return code;
}

static const cell_t *to_closure(precontext_t *context, cell_t **env, cell_t *requests)
{
	long len = list_length(requests);
	cell_t *code = create_vector(len);

	if (len) {
		const cell_t *p = requests;
		for (long i = 0; i < len; i++) {
			long sp_offset;
			const cell_t *refer = item(&sp_offset, env, car(p));
			code->vector[i] = refer;

			if (tag_of(refer) == CELL_REFER_RELATIVE) {
				long offset = integer_value_of(refer);
				if ((offset & RELATIVE_MASK) == RELATIVE_SP)
					indirect_append(context,
							create_refer_relative(
									(offset >> RELATIVE_BITS) - sp_offset,
									RELATIVE_SP));
			}

			p = cdr(p);
		}
	}

	return code;
}
