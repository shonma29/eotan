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
#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_COLUMNS (80)
#define DEFAULT_LINES (25)

static int columns;
static int lines;
static char *linebuf;

static int get_columns(void);
static int get_lines(void);
static bool show(FILE *);
static bool process(void);

extern int rawon(void);
extern int rawoff(void);


int main(int argc, char **argv)
{
	FILE *fp;

	columns = get_columns() + 1;
	lines = get_lines() - 1;

	//TODO UTF-8
	linebuf = malloc(columns);
	if (!linebuf) {
		perror(NULL);
		return EXIT_FAILURE;
	}

	switch (argc) {
	case 1:
		fp = stdin;
		break;
	case 2:
		argv++;
		fp = fopen(*argv, "r");
		if (!fp) {
			perror(*argv);
			return EXIT_FAILURE;
		}

		break;
	default:
		fprintf(stderr, "Missing filename\n");
		return EXIT_FAILURE;
	}

	rawon();

	for (;;) {
		bool done = show(fp);

		fputs("--More--", stdout);
		done |= process();
		putchar('\n');

		if (done)
			break;
	}

	rawoff();
	fclose(fp);
	return 0;
}

static int get_columns(void)
{
	//TODO getenv -> atoi
	return DEFAULT_COLUMNS;
}

static int get_lines(void)
{
	//TODO getenv -> atoi
	return DEFAULT_LINES;
}

static bool show(FILE *fp)
{
	for (int n = 0; n < lines; n++) {
//TODO cut LF when maximum length
		if (!fgets(linebuf, columns, fp))
			return true;

		fputs(linebuf, stdout);
	}

	return false;
}

static bool process(void)
{
	for (;;) {
		switch (getchar()) {
		case 'q':
		case 'Q':
			return true;
		case ' ':
			return false;
		default:
			break;
		}
	}
}
