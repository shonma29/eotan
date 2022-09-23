#ifndef _NSCM_VM_H_
#define _NSCM_VM_H_ 1

#include "nscm.h"

#define CALL_STACK_SIZE (1024)
#define PARAMETER_STACK_SIZE (4096)

// parameter stack
#define OFFSET_CALLEE (0)
#define OFFSET_ARGC (1)
#define OFFSET_ARGV (2)
#define MIN_PARAMETER_FRAME_SIZE (OFFSET_ARGV)

// return stack
#define OFFSET_IP (0)
#define OFFSET_EP (1)
#define RETURN_FRAME_SIZE (2)

// code block
#define OFFSET_VARS (0)
#define OFFSET_CLOSURE (1)
#define OFFSET_INDIRECT (2)
#define OFFSET_DATA (3)
#define OFFSET_OPCODE (4)

#define VM_OP_BITS (4)

#define RELATIVE_BITS (1)
#define RELATIVE_MASK (1)
#define RELATIVE_SP (0)
#define RELATIVE_EP (1)

typedef enum {
	OP_LOAD = 0,
	OP_LOAD_RELATIVE = 1,
	OP_STORE = 2,
	OP_CDR = 3,
	OP_BF = 4,//TODO BT may be needed(if + not, and, or)
	OP_BRA = 5,
	OP_CREATE_FRAME = 6,
	OP_CALL = 7,
	OP_TAIL = 8,
	OP_RET = 9,
	OP_CREATE_LAMBDA = 10,
	OP_TO_LIST = 11,
	OP_ADDI = 12,
	OP_SUBI = 13,
	OP_MULI = 14,
	OP_DIVI = 15
} vm_op_e;

#ifdef USE_BASE
#define NUM_OF_OP (16)
#else
#define NUM_OF_OP (12)
#endif

//TODO split pcompile.h
//TODO too large?
#define INITIAL_CODEBUF_SIZE (64)

typedef struct context {
	long d1;
	const cell_t **ip;//TODO union registers to refer as array
	const cell_t **sp;
	void **rp;
	const cell_t **ep;
#ifdef USE_ANALYSIS
	long analysis_max_call;
	long analysis_max_param;
#endif
	void *call_stack[CALL_STACK_SIZE];
	const cell_t *parameter_stack[PARAMETER_STACK_SIZE];
} context_t;

static inline long words_of_frame(const long argc)
{
	return (argc + 2);
}

extern void vm_initialize(context_t *);
extern const cell_t *vm_start(context_t *, const cell_t *);
extern long vm_get_opcode(const void *);

#endif