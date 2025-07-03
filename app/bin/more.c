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
#include <unistd.h>
#include <fcntl.h>

#define DEFAULT_COLUMNS (80)
#define DEFAULT_LINES (25)

static int columns;
static int lines;
static char *linebuf;

static bool show(FILE *);
static bool process(const int);
static int _get_env_value(const char *, const int);

extern int rawon(void);
extern int rawoff(void);


int main(int argc, char **argv)
{
	columns = _get_env_value("COLUMNS", DEFAULT_COLUMNS);
	lines = _get_env_value("LINES", DEFAULT_LINES) - 1;

	//TODO UTF-8
	linebuf = malloc(columns + 1);
	if (!linebuf) {
		perror(NULL);
		return EXIT_FAILURE;
	}

	int fd = STDIN_FILENO;
	FILE *fp;
	switch (argc) {
	case 1:
		fp = stdin;
#if 0
		fd = open("/dev/cons", O_RDONLY);
#else
		fd = _get_env_value("CONS_FILENO", -1);
#endif
		if (fd < 0) {
			fprintf(stderr, "failed to open cons\n");
			perror(NULL);
			return EXIT_FAILURE;
		}
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

	if (rawon()) {
		fprintf(stderr, "failed to switch to raw mode\n");
		return EXIT_FAILURE;
	}

	while (!show(fp) && !process(fd));

	if (rawoff()) {
		fprintf(stderr, "failed to switch to cooked mode\n");
		return EXIT_FAILURE;
	}

	fclose(fp);
	return 0;
}

static bool show(FILE *fp)
{
	for (int n = 0; n < lines; n++) {
		if (!fgets(linebuf, columns + 1, fp))
			return true;

		fputs(linebuf, stdout);
	}

	fflush(stdout);
	return false;
}

static bool process(const int fd)
{
	for (;;) {
		char buf[1];
		if (read(fd, buf, sizeof(buf)) != sizeof(buf))
			return true;

		switch (buf[0]) {
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

static int _get_env_value(const char *var_name, const int default_value)
{
	char *value = getenv(var_name);
	if (value) {
		int n = atoi(value);
		if (n > 0)
			return n;
	}

	return default_value;
}
