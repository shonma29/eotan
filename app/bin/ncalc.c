/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define STACK_SIZE (256)

/*
	++, --
TODO	-
TODO	^	right to left
	*, /, %	left to right
	+, -	left to right
	=, +=, -+, *=, /=, %=, ^=	right to left
	==, <=, >=, !=, <, >
*/

typedef struct _parser_t {
	bool has_buffer;
	int ch;
} parser_t;

static parser_t parser = {
	false,
	EOF
};
static int stack[STACK_SIZE];
static int stack_depth = 0;

static int _getc(parser_t *, const bool);
static int _ungetc(parser_t *, const int);
static void add(void);
static void subtract(void);
static void multiply(void);
static void divide(void);
static void modulo(void);
static int push(const int);
static int pop(void);
static void get_factor(parser_t *);
static void get_term(parser_t *);
static void get_expression(parser_t *);


static int _getc(parser_t *parser, const bool skip_space)
{
	if (parser->has_buffer) {
		parser->has_buffer = false;
		return parser->ch;
	}

	do {
		parser->ch = getchar();
		if (parser->ch == EOF)
			break;
	//TODO skip other way
	} while (skip_space && (parser->ch == ' ' || parser->ch == '\t'));
	return parser->ch;
}

static int _ungetc(parser_t *parser, const int ch)
{
	if (parser->has_buffer)
		return (-1);

	parser->has_buffer = true;
	parser->ch = ch;
	return 0;
}

static void add(void)
{
	push(pop() + pop());
	//TODO check overflow
}

static void subtract(void)
{
	int b = pop();
	int a = pop();
	push(a - b);
	//TODO check underflow
}

static void multiply(void)
{
	push(pop() * pop());
	//TODO check overflow
}

static void divide(void)
{
	int b = pop();
	int a = pop();
	//TODO check zero
	push(a / b);
}

static void modulo(void)
{
	int b = pop();
	int a = pop();
	//TODO check zero
	push(a % b);
}

static int push(const int n)
{
	if (stack_depth < STACK_SIZE) {
//		printf("push[%d]\n", n);
		stack[stack_depth++] = n;
		return 0;
	}

	printf("stack overflow\n");
	exit(EXIT_FAILURE);
}

static int pop(void)
{
	if (stack_depth) {
		return stack[--stack_depth];
	}

	printf("stack underflow\n");
	exit(EXIT_FAILURE);
}

static void get_factor(parser_t *parser)
{
	int ch = _getc(parser, true);
	if (ch == '(') {
		get_expression(parser);

		ch = _getc(parser, true);
		if (ch == ')')
			return;
	} else 	if (ch == '-') {
		//TODO support minus
	} else if (isdigit(ch)) {
		//TODO use strtol (parse by line)
		//TODO float
		int v = 0;
		do {
			v = v * 10 + ch - '0';
			ch = _getc(parser, false);
		} while(isdigit(ch));

		if (ch != ' ' && ch != '\t')
			_ungetc(parser, ch);

		push(v);
		return;
	}

	printf("unknown0 %c\n", ch);
	exit(EXIT_FAILURE);
}

static void get_term(parser_t *parser)
{
	get_factor(parser);

	for (;;) {
		int ch = _getc(parser, true);
		switch (ch) {
		case '*':
			get_factor(parser);
			multiply();
			continue;
		case '/':
			get_factor(parser);
			divide();
			continue;
		case '%':
			get_factor(parser);
			modulo();
			continue;
		default:
			break;
		}

		_ungetc(parser, ch);
		break;
	}
}

static void get_expression(parser_t *parser)
{
	get_term(parser);

	for (;;) {
		int ch = _getc(parser, true);
		switch (ch) {
		case '+':
			get_term(parser);
			add();
			continue;
		case '-':
			get_term(parser);
			subtract();
			continue;
		default:
			break;
		}

		_ungetc(parser, ch);
		break;
	}
}

int main(int argc, char **argv)
{
	int ch;
	do {
		//TODO parse by line (support back space)
		//TODO skip empty input (longjmp if only LF)
		//TODO skip error
		//TODO support show value
		get_expression(&parser);

		ch = _getc(&parser, true);
		if (ch != '\n' && ch != EOF) {
			printf("unknown1 %c\n", ch);
			return EXIT_FAILURE;
		}

		printf("%d\n", pop());
	} while (ch != EOF);

	return EXIT_SUCCESS;
}
