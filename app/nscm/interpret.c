#include "nscm.h"
#include "vm.h"

static const cell_t *call_procedure(context_t *, const cell_t *);
static const cell_t *create_closure(context_t *, const cell_t *);
static void to_indirect(context_t *);
static void parameter_to_list(context_t *);
static void push_call(context_t *, const cell_t **, const cell_t **);
#if 0
static void create_frame(const long op, const cell_t *);
#endif
static void add_parameter_stack(context_t *, const unsigned long);
static void pop_parameter(context_t *);

#ifdef USE_BASE
static const cell_t *lib_add_1integer(const cell_t *, const long);
static const cell_t *lib_subtract_1integer(const cell_t *, const long);
static const cell_t *lib_multiply_1integer(const cell_t *, const long);
static const cell_t *lib_divide_1integer(const cell_t *, const long);
#endif

void vm_initialize(context_t *cc)
{
	cc->rp = &(cc->call_stack[CALL_STACK_SIZE - 1]);
	cc->rp[OFFSET_IP] = NULL;
	cc->sp = &(cc->parameter_stack[PARAMETER_STACK_SIZE - MIN_PARAMETER_FRAME_SIZE]);
	cc->sp[OFFSET_CALLEE] = null_cell;
	cc->sp[OFFSET_ARGC] = 0;
#ifdef USE_ANALYSIS
	cc->analysis_max_call = 0;
	cc->analysis_max_param = 0;
#endif
}

static const cell_t *call_procedure(context_t *cc, const cell_t *f)
{
	const cell_t *result = f->procedure(integer_value_of(cc->sp[OFFSET_ARGC]),
			&(cc->sp[OFFSET_ARGV]));
	pop_parameter(cc);
	return result;
}

const cell_t *vm_start(context_t *cc, const cell_t *cell)
{
	static const void *optable[] = {
		&&P_LOAD, &&P_LOAD_RELATIVE, &&P_STORE, &&P_CDR,
		&&P_BF, &&P_BRA, &&P_CREATE_FRAME, &&P_CALL,
		&&P_TAIL, &&P_RET, &&P_CREATE_LAMBDA, &&P_TO_LIST,
#ifdef USE_BASE
		&&P_ADDI, &&P_SUBI, &&P_MULI, &&P_DIVI
#endif
	};

	if (!cc)
		return ((cell_t *) optable);

	cc->d1 = (long) unspecified_cell;
	//TODO check ip range
	cc->ip = &(cell->vector[OFFSET_OPCODE]);
	goto *((void *) *(cc->ip));

P_LOAD:
	cc->d1 = (long) (cc->ip[1]);
	cc->ip += 2;
	goto *((void *) *(cc->ip));
P_LOAD_RELATIVE:{
		//TODO check parameter stack range
		long offset = integer_value_of(cc->ip[1]);
		const cell_t **base = (offset & RELATIVE_MASK) ? (cc->ep) : (cc->sp);
		cc->d1 = (long) (base[offset >> RELATIVE_BITS]);
		if (tag_of((const cell_t *) (cc->d1)) == CELL_REFER_INDIRECT)
			cc->d1 = (long) cdr((const cell_t *) (cc->d1));
	}
	cc->ip += 2;
	goto *((void *) *(cc->ip));
P_STORE:
	cc->sp[(long) (cc->ip[1])] = (const cell_t *) (cc->d1);
	cc->ip += 2;
	goto *((void *) *(cc->ip));
P_CDR:
	cc->d1 = (long) cdr(cc->ip[1]);
	if ((uintptr_t) (cc->d1) == (uintptr_t) unspecified_cell)
		throw("symbol not defined", car(cc->ip[1]));

	cc->ip += 2;
	goto *((void *) *(cc->ip));
P_BF:
	if ((uintptr_t) (cc->d1) != (uintptr_t) false_cell) {
		cc->ip += 2;
		goto *((void *) *(cc->ip));
	}
P_BRA:
	cc->ip += (long) (cc->ip[1]);
	goto *((void *) *(cc->ip));
P_CREATE_FRAME:
	add_parameter_stack(cc, words_of_frame((long) (cc->ip[1])));
	cc->sp[OFFSET_CALLEE] = (const cell_t *) (cc->d1);
	cc->sp[OFFSET_ARGC] = to_integer((long) (cc->ip[1]));

	for (long i = 0; i < (long) (cc->ip[1]); i++)
		cc->sp[OFFSET_ARGV + i] = to_integer(0);

	cc->ip += 2;
	goto *((void *) *(cc->ip));
P_CALL:
	switch (tag_of(cc->sp[OFFSET_CALLEE])) {
	case CELL_CODE:
		push_call(cc, cc->ip, cc->ep);
		cc->ip = cc->sp[OFFSET_CALLEE]->vector;
		cc->ep = (const cell_t **) null_cell;
		if (cc->ip[OFFSET_VARS] != null_cell)
			validate_length(integer_value_of(cc->sp[OFFSET_ARGC]),
					(*(cc->ip))->length);

		to_indirect(cc);
		cc->ip += OFFSET_OPCODE - 1;
		break;
	case CELL_PROCEDURE:
		cc->d1 = (long) call_procedure(cc, cc->sp[OFFSET_CALLEE]);
		break;
	case CELL_LAMBDA:
		push_call(cc, cc->ip, cc->ep);
		{
			const cell_t *lambda = cc->sp[OFFSET_CALLEE];
			cc->ip = lambda->code->vector;
			cc->ep = lambda->env->vector;
		}

		if (cc->ip[OFFSET_VARS] != null_cell)
			validate_length(integer_value_of(cc->sp[OFFSET_ARGC]),
					(*(cc->ip))->length);

		to_indirect(cc);
		cc->ip += OFFSET_OPCODE - 1;
		break;
	default:
		throw("cannot evaluate", cc->sp[OFFSET_CALLEE]);
	}
	cc->ip++;
	goto *((void *) *(cc->ip));
P_TAIL:
	switch (tag_of(cc->sp[OFFSET_CALLEE])) {
	case CELL_CODE:
		//TODO slow
		if ((uintptr_t) (cc->sp)
				>= (uintptr_t) &(cc->parameter_stack[PARAMETER_STACK_SIZE - 1]))
			throw("param stack underflow", null_cell);
		else {
			const cell_t **current = cc->sp;
			const long len = words_of_frame(integer_value_of(current[OFFSET_ARGC]));
			cc->sp += words_of_frame(
					integer_value_of(current[len + OFFSET_ARGC]));
			for (long i = len - 1; i >= 0; i--)
				cc->sp[i] = current[i];
		}
		cc->ip = cc->sp[OFFSET_CALLEE]->vector;
		if (cc->ip[OFFSET_VARS] != null_cell)
			validate_length(integer_value_of(cc->sp[OFFSET_ARGC]),
					(*(cc->ip))->length);

		to_indirect(cc);
		cc->ip += OFFSET_OPCODE - 1;
		break;
	case CELL_PROCEDURE:
		cc->d1 = (long) call_procedure(cc, cc->sp[OFFSET_CALLEE]);
		break;
	case CELL_LAMBDA:
		if ((uintptr_t) (cc->sp)
				>= (uintptr_t) &(cc->parameter_stack[PARAMETER_STACK_SIZE - 1]))
			throw("param stack underflow", null_cell);
		else {
			const cell_t **current = cc->sp;
			const long len = words_of_frame(integer_value_of(current[OFFSET_ARGC]));
			cc->sp += words_of_frame(
					integer_value_of(current[len + OFFSET_ARGC]));
			for (long i = len - 1; i >= 0; i--)
				cc->sp[i] = current[i];
		}
		{
			const cell_t *lambda = cc->sp[OFFSET_CALLEE];
			cc->ip = lambda->code->vector;
			cc->ep = lambda->env->vector;
		}
		if (cc->ip[OFFSET_VARS] != null_cell)
			validate_length(integer_value_of(cc->sp[OFFSET_ARGC]),
					(*(cc->ip))->length);

		to_indirect(cc);
		cc->ip += OFFSET_OPCODE - 1;
		break;
	default:
		throw("cannot evaluate", cc->sp[OFFSET_CALLEE]);
	}
	cc->ip++;
	goto *((void *) *(cc->ip));
P_RET:
	cc->ip = cc->rp[OFFSET_IP];
	if (!(cc->ip))
		return (cell_t *) (cc->d1);

	pop_parameter(cc);
	cc->ep = cc->rp[OFFSET_EP];
	cc->rp += RETURN_FRAME_SIZE;
	gc();
	cc->ip++;
	goto *((void *) *(cc->ip));
P_CREATE_LAMBDA:{
		const cell_t *code = cc->ip[1];
		cc->d1 = (long) create_lambda(
				create_closure(cc, code->vector[OFFSET_CLOSURE]), code);
				//TODO protect lambda and env?
	}
	cc->ip += 2;
	goto *((void *) *(cc->ip));
P_TO_LIST:
	parameter_to_list(cc);
	cc->ip++;
	goto *((void *) *(cc->ip));
#ifdef USE_BASE
P_ADDI:
	cc->d1 = (long) lib_add_1integer(
			(const cell_t *) (cc->d1), (const long) (cc->ip[1]));
	cc->ip += 2;
	goto *((void *) *(cc->ip));
P_SUBI:
	cc->d1 = (long) lib_subtract_1integer(
			(const cell_t *) (cc->d1), (const long) (cc->ip[1]));
	cc->ip += 2;
	goto *((void *) *(cc->ip));
P_MULI:
	cc->d1 = (long) lib_multiply_1integer(
			(const cell_t *) (cc->d1), (const long) (cc->ip[1]));
	cc->ip += 2;
	goto *((void *) *(cc->ip));
P_DIVI:
	cc->d1 = (long) lib_divide_1integer(
			(const cell_t *) (cc->d1), (const long) (cc->ip[1]));
	cc->ip += 2;
	goto *((void *) *(cc->ip));
#endif
}

static const cell_t *create_closure(context_t *cc, const cell_t *requests)
{
	long len = requests->length;
	cell_t *code = create_vector(len);

	if (len)
		for (long i = 0; i < len; i++) {
			const cell_t *refer = requests->vector[i];
			long offset = integer_value_of(refer);
			const cell_t **base = (offset & RELATIVE_MASK) ? (cc->ep) : (cc->sp);
			code->vector[i] = base[offset >> RELATIVE_BITS];
		}

	return code;
}

static void to_indirect(context_t *cc)
{
	const cell_t *indirect = cc->ip[OFFSET_INDIRECT];
	long max = indirect->length;
	for (long i = 0; i < max; i++) {
		long offset = integer_value_of(indirect->vector[i]) >> RELATIVE_BITS;
		//TODO refer name by cc->ip[OFFSET_VARS]?
		cc->sp[offset] = create_refer_indirect(null_cell, cc->sp[offset]);
	}
}

static void parameter_to_list(context_t *cc)
{
	long len = integer_value_of(cc->sp[OFFSET_ARGC]);
	if (len) {
		const cell_t *head = create_pair(cc->sp[OFFSET_ARGV], null_cell);
		cell_t *y = (cell_t *) head;
		for (int i = 1; i < len; i++) {
			y->rest = create_pair(cc->sp[i + OFFSET_ARGV], null_cell);
			y = (cell_t *) (y->rest);
		}

		cc->sp += len - 1;
		cc->sp[OFFSET_ARGV] = head;
	} else {
		add_parameter_stack(cc, 1);
		cc->sp[OFFSET_ARGV] = null_cell;
	}

	cc->sp[OFFSET_ARGC] = create_integer(1);
}

static void push_call(context_t *cc, const cell_t **ip, const cell_t **ep)
{
	cc->rp -= RETURN_FRAME_SIZE;
	if ((uintptr_t) (cc->rp) < (uintptr_t) (cc->call_stack))//TODO slow. but stack may be filled with 0
		throw("call stack overflow", null_cell);
#ifdef USE_ANALYSIS
	long depth = ((uintptr_t) &(cc->call_stack[CALL_STACK_SIZE - 1])
					- (uintptr_t) (cc->rp))
			/ sizeof(uintptr_t);
	if (depth > cc->analysis_max_call)
		cc->analysis_max_call = depth;
#endif
	cc->rp[OFFSET_IP] = ip;
	cc->rp[OFFSET_EP] = ep;
}
#if 0
//TODO slow as function?
static void create_frame(const long op, const cell_t *callee)
{
	long argc = op >> VM_OP_BITS;
	add_parameter_stack(words_of_frame(argc));
	sp[OFFSET_CALLEE] = callee;
	sp[OFFSET_ARGC] = create_integer(argc);

	const cell_t *zero_cell = to_integer(0);
	for (long i = 0; i < argc; i++)
		sp[OFFSET_ARGV + i] = zero_cell;
}
#endif
static void add_parameter_stack(context_t *cc, const unsigned long n)
{
	uintptr_t rest = ((uintptr_t) (cc->sp) - (uintptr_t) (cc->parameter_stack))
			/ sizeof(uintptr_t);
	if (n > rest)
		throw("param stack overflow", null_cell);

	cc->sp -= n;
#ifdef USE_ANALYSIS
	long depth = ((uintptr_t) &(cc->parameter_stack[PARAMETER_STACK_SIZE - 1])
					- (uintptr_t) (cc->sp))
			/ sizeof(uintptr_t);
	if (depth > cc->analysis_max_param)
		cc->analysis_max_param = depth;
#endif
}

static void pop_parameter(context_t *cc)
{
	if ((uintptr_t) (cc->sp)
			>= (uintptr_t) &(cc->parameter_stack[PARAMETER_STACK_SIZE - 1]))//TODO slow
		throw("param stack underflow", null_cell);

	cc->sp += words_of_frame(integer_value_of(cc->sp[OFFSET_ARGC]));
}
#ifdef USE_BASE
static const cell_t *lib_add_1integer(const cell_t *dest, const long operand)
{
	switch (tag_of(dest)) {
	case CELL_INTEGER:
		return create_integer(integer_value_of(dest) + operand);
#ifdef USE_FLONUM
	case CELL_FLONUM:
		return create_flonum(dest->flonum + operand);
#endif
	default:
		throw("is not number", dest);
	}
}

static const cell_t *lib_subtract_1integer(const cell_t *dest, const long operand)
{
	switch (tag_of(dest)) {
	case CELL_INTEGER:
		return create_integer(integer_value_of(dest) - operand);
#ifdef USE_FLONUM
	case CELL_FLONUM:
		//TODO reuse cell (or use d1) for acceleration
		return create_flonum(dest->flonum - operand);
#endif
	default:
		throw("is not number", dest);
	}
}

static const cell_t *lib_multiply_1integer(const cell_t *dest, const long operand)
{
	switch (tag_of(dest)) {
	case CELL_INTEGER:
		return create_integer(integer_value_of(dest) * operand);
#ifdef USE_FLONUM
	case CELL_FLONUM:
		return create_flonum(dest->flonum * operand);
#endif
	default:
		throw("is not number", dest);
	}
}

static const cell_t *lib_divide_1integer(const cell_t *dest, const long operand)
{
	if (!operand)
		throw("cannot divide", create_integer(0));

	switch (tag_of(dest)) {
	case CELL_INTEGER:
		return create_integer(integer_value_of(dest) / operand);
#ifdef USE_FLONUM
	case CELL_FLONUM:
		return create_flonum(dest->flonum / operand);
#endif
	default:
		throw("is not number", dest);
	}
}
#endif

#ifdef REPL_SHOW_DEBUG
long vm_get_opcode(const void *p)
{
	const void **vm_optable = (const void **) vm_start(NULL, NULL);
	for (long i = 0; i < NUM_OF_OP; i++)
		if (vm_optable[i] == p)
			return i;

	return (-1);
}
#endif