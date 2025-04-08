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
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <libc.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <set/hash.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define ERR (-1)
#define ERR_OK (0)
#define ERR_MEMORY (2)

#define FD_NOT_OPENED (-1)

#define DIRS_DELIMITER ':'
#define PATH_DELIMITER '/'

#define DEFAULT_BUF_SIZE 1024
#define SHIFT_BUF_SIZE 1
#define DEFAULT_ARRAY_SIZE 16
#define SHIFT_ARRAY_SIZE 1

#define MAX_VAR (256)

#define PROMPT "$"
#define DEFAULT_MAIN "/rc/lib/rcmain"

typedef struct {
	int files[3];
	int pipe_file;
	bool background;
} ExecOptions;

typedef struct {
	unsigned char *head;
	size_t len;
	size_t max;
	bool last;
	unsigned char buf[MAXPATHLEN];
} Token;

typedef enum {
	NONE = 0,
	PIPE = 1
} delimiter_type_e;

typedef struct {
	unsigned int pos;
	unsigned int head;
	bool quoting;
	size_t bufsize;
	unsigned char *buf;
	size_t arraysize;
	unsigned char **array;
	struct {
		unsigned int position;
		delimiter_type_e type;
	} delimiter;
	ExecOptions opts;
} Line;

static Line line = {
	0,
	0,
	false,
	DEFAULT_BUF_SIZE >> 1,
	NULL,
	DEFAULT_ARRAY_SIZE >> 1,
	NULL
};
static bool interactive = true;

static int get_path(Token *);
static bool get_size(size_t *, const unsigned char *);
static void execute(unsigned char **, unsigned char **, const unsigned char *,
		const ExecOptions *);
static void line_realloc_buf(Line *);
static void line_destroy(Line *);
static void line_clear(Line *);
static bool line_preprocess(Line *);
static bool line_putc(Line *, const unsigned char);
static void line_realloc_array(Line *);
static bool line_parse(Line *);
static bool line_evaluate(Line *, hash_t *);
static void _options_clear(ExecOptions *, const int);
static int _pipe_create(ExecOptions *);
static unsigned int var_calc_hash(const void *, const size_t);
static int var_compare(const void *, const void *);
static unsigned char *var_get(hash_t *, const unsigned char *);
static bool var_put(hash_t *, const unsigned char *);
static hash_t *var_create(void);
static unsigned char **var_expand(hash_t *);
static void var_show_all(const hash_t *);
static void interpret(hash_t *, FILE *);


static int get_path(Token *token)
{
	unsigned char *head = token->head;
	unsigned char *p;
	for (p = head;; p++) {
		unsigned char c = *p;
		if (c == DIRS_DELIMITER) {
			token->head = p + 1;
			break;
		}

		if (!c) {
			token->head = p;
			token->last = true;
			break;
		}
	}

	token->len = (size_t) p - (size_t) head;
	if (!token->len)
		return 1;

	if (p[-1] == PATH_DELIMITER) {
		if (token->len > token->max)
			return 1;

		memcpy(token->buf, head, token->len);
	} else {
		if (token->len >= token->max)
			return 1;

		memcpy(token->buf, head, token->len);
		token->buf[token->len] = PATH_DELIMITER;
		token->len++;
	}

	return 0;
}

static bool get_size(size_t *len, const unsigned char *head)
{
	bool has_path = false;
	const unsigned char *p;
	for (p = head; *p; p++)
		if (*p == PATH_DELIMITER)
			has_path = true;

	*len = (size_t) p - (size_t) head;
	return has_path;
}

static void execute(unsigned char **array, unsigned char **env,
		const unsigned char *path, const ExecOptions *opts)
{
	for (int status; waitpid(-1, &status, WNOHANG) > 0;);

	pid_t pid = rfork(RFNOTEG);
	if (pid == 0) {
		// child
		Token token;
		bool has_path = get_size(&(token.len), array[0]);
		//TODO is safe?
		if (++token.len > MAXPATHLEN)
			exit(ENAMETOOLONG);

		for (unsigned int i = 0;
				i < sizeof(opts->files) / sizeof(opts->files[0]);
				i++)
			if (opts->files[i] != FD_NOT_OPENED) {
				close(i);
				dup2(opts->files[i], i);
				close(opts->files[i]);
			}

		if (opts->background
				&& (opts->files[STDIN_FILENO] == FD_NOT_OPENED))
			//TODO use '/dev/null'
			close(STDIN_FILENO);

		if (has_path) {
			if (execve((char *) (array[0]), (char **) array,
					(char **) env) == ERR) {
				if (errno == ENOENT)
					fprintf(stderr, "%s not found\n",
							array[0]);
				else
					fprintf(stderr, "exec error %d\n",
							errno);

				exit(errno);
			}
		} else if (!path)
			exit(ENOENT);
		else {
			token.head = (unsigned char *) path;
			token.max = MAXPATHLEN - token.len;
			token.last = false;

			do {
				if (get_path(&token)) {
					fprintf(stderr, "bad path\n");
					break;
				}

				strcpy((char *) &(token.buf[token.len]),
						(char *) (array[0]));
				if (execve((char *) (token.buf),
						(char **) array,
						(char **) env) == ERR)
					if (errno != ENOENT) {
						fprintf(stderr,
								"exec error %d\n",
								errno);
						exit(errno);
					}
			} while (!(token.last));

			fprintf(stderr, "%s not found\n", array[0]);
			exit(ENOENT);
		}
	} else {
		for (unsigned int i = 0;
				i < sizeof(opts->files) / sizeof(opts->files[0]);
				i++)
			if (opts->files[i] != FD_NOT_OPENED)
				close(opts->files[i]);

		if (pid == ERR)
			// error
			fprintf(stderr, "fork error %d\n", pid);
		else if (!opts->background) {
			// parent
			int status;
			if (waitpid(pid, &status, 0) == ERR)
				fprintf(stderr, "wait error %d\n", errno);
			else
				//TODO omit print, and set $status
				fprintf(stderr, "wait success %d\n", status);
		}
	}
}

static void line_realloc_buf(Line *p)
{
	size_t next_size = p->bufsize << SHIFT_BUF_SIZE;
	unsigned char *buf = (unsigned char *) malloc(
			sizeof(unsigned char) * next_size);
	if (buf) {
		if (p->buf) {
			memcpy(buf, p->buf, p->bufsize);
			free(p->buf);
		}

		p->buf = buf;
		p->bufsize = next_size;
	} else {
		fprintf(stderr, "memory exhausted\n");
		exit(ERR_MEMORY);
	}
}

static void line_destroy(Line *p)
{
	if (p->buf)
		free(p->buf);

	if (p->array)
		free(p->array);
}

static void line_clear(Line *p)
{
	p->head = p->pos = 0;
	p->quoting = false;
	p->delimiter.position = 0;
	p->delimiter.type = NONE;
}

static bool line_preprocess(Line *p)
{
	unsigned char c;
	for (unsigned int i = p->head; (c = p->buf[i]); i++) {
		if (c == '\'')
			p->quoting = !(p->quoting);
		else if ((c == '\\')
				&& (p->buf[i + 1] == '\n')) {
			if (p->quoting)
				p->pos = i + 2;
			else {
				p->buf[i] = ' ';
				p->pos = i + 1;
			}

			p->head = p->pos;
			return false;
		} else if ((c == '\n')
				|| (c == 0x1a)
				|| ((c == '#')
						&& !(p->quoting))) {
			p->buf[i] = '\0';
			p->pos = i + 1;
			break;
		}
	}

	return true;
}

static bool line_putc(Line *p, const unsigned char c)
{
	switch (c) {
	case '\n':
	case 0x1a:
		p->buf[p->pos] = c;
		p->pos++;
		if (p->pos >= p->bufsize)
			line_realloc_buf(p);

		p->buf[p->pos] = '\0';
		p->pos++;
		return line_preprocess(p);
	case 0x08:
		if (p->pos > p->head)
			p->pos--;

		break;
	default:
		p->buf[p->pos] = c;
		p->pos++;
		if (p->pos >= p->bufsize)
			line_realloc_buf(p);

		break;
	}

	return false;
}

static void line_realloc_array(Line *p)
{
	size_t next_size = p->arraysize << SHIFT_ARRAY_SIZE;
	unsigned char **array = (unsigned char **) malloc(sizeof(unsigned char *)
			* next_size);
	if (array) {
		if (p->array) {
			memcpy(array, p->array,
					sizeof(unsigned char *) * p->arraysize);
			free(p->array);
		}

		p->array = array;
		p->arraysize = next_size;
	} else {
		fprintf(stderr, "memory exhausted\n");
		exit(ERR_MEMORY);
	}
}

static bool line_parse(Line *p)
{
	unsigned int pos = 0;
	p->delimiter.type = NONE;
	for (unsigned int i = p->delimiter.position;; i++) {
		for (; (p->buf[i] > 0) && (p->buf[i] <= ' '); i++);
		if (!p->buf[i])
			break;

		if (p->buf[i] == '|') {
			if (!pos) {
				fprintf(stderr, "unexpected '|'\n");
				return false;
			}

			p->buf[i] = '\0';
			p->delimiter.position = i + 1;
			p->delimiter.type = PIPE;
			break;
		}
		p->array[pos] = &(p->buf[i]);
		pos++;
		if (pos >= p->arraysize)
			line_realloc_array(p);

		unsigned char c;
		unsigned char *w = &(p->buf[i]);
		bool quoting = false;
		for (; (c = p->buf[i]); i++) {
			if (quoting) {
				if (c == '\'') {
					if (p->buf[i + 1] == '\'')
						i++;
					else {
						quoting = false;
						continue;
					}
				}
			} else if (c == '\'') {
				quoting = true;
				continue;
			} else if (c <= ' ')
				break;

			*w = p->buf[i];
			w++;
		}

		if (quoting) {
			fprintf(stderr, "bad quote\n");
			return false;
		}

		//TODO expand wild card

		c = p->buf[i];
		*w = '\0';

		if (!c)
			break;
	}

	p->array[pos] = NULL;
	return (p->array[0] ? true : false);
}

//TODO close redirect files on error
static bool line_evaluate(Line *p, hash_t *vars)
{
	_options_clear(&(p->opts), FD_NOT_OPENED);

	while (line_parse(p)) {
		unsigned int j = 0;
		for (unsigned int i = 0; p->array[i]; i++) {
			if (!strcmp((char *) (p->array[i]), "<")) {
				//TODO not error? if head of line
				i++;
				if (!p->array[i]) {
					//TODO modify error message. ex. unexpected 'newline'
					fprintf(stderr, "redirect what?\n");
					return false;
				}

				if (p->opts.files[STDIN_FILENO] != FD_NOT_OPENED)
					close(p->opts.files[STDIN_FILENO]);

				p->opts.files[STDIN_FILENO] = open((char *) (p->array[i]),
						O_RDONLY);
				if (p->opts.files[STDIN_FILENO] < 0) {
					fprintf(stderr, "%s cannot open (%d)\n",
							p->array[i], errno);
					return false;
				}
			} else if (!strcmp((char *) (p->array[i]), ">")) {
				//TODO not error? empty file is proper
				i++;
				if (!p->array[i]) {
					//TODO modify error message. ex. unexpected 'newline'
					fprintf(stderr, "redirect what?\n");
					return false;
				}

				if (p->opts.files[STDOUT_FILENO] != FD_NOT_OPENED)
					close(p->opts.files[STDOUT_FILENO]);

				p->opts.files[STDOUT_FILENO] = open((char *) (p->array[i]),
						O_WRONLY | O_CREAT | O_TRUNC,
						//TODO adhoc
						S_IRUSR | S_IWUSR | S_IRGRP
								| S_IROTH);
				if (p->opts.files[STDOUT_FILENO] < 0) {
					fprintf(stderr, "%s cannot open (%d)\n",
							p->array[i], errno);
					return false;
				}
			} else if (!strcmp((char *) (p->array[i]), "&")) {
				//TODO not error? show "`&' unexpected" if head of line
				p->opts.background = true;
				break;
			} else {
				p->array[j] = p->array[i];
				j++;
			}
		}

		p->array[j] = NULL;

		//TODO execute inner commands in background
		if (!strcmp((char *) (p->array[0]), "cd")) {
			if (chdir(p->array[1] ? (char *) (p->array[1]) : "/"))
				fprintf(stderr, "chdir error=%d\n", errno);
		} else if (!strcmp((char *) (p->array[0]), "exit"))
			return true;
		else if (!strcmp((char *) (p->array[0]), "env"))
			var_show_all(vars);
		else if (!strcmp((char *) (p->array[0]), "export"))
			//TODO omit 'export' keyword
			var_put(vars, p->array[1]);
		else {
			if (p->delimiter.type == PIPE)
				if (_pipe_create(&(p->opts)))
					return false;

			unsigned char **envp = var_expand(vars);
			execute(p->array, envp, var_get(
					vars, (unsigned char *) ("PATH")),
					&(p->opts));
			free(envp);

			if (p->delimiter.type == PIPE) {
				_options_clear(&(p->opts), p->opts.pipe_file);
				continue;
			}
		}

		break;
	}

	return false;
}

static void _options_clear(ExecOptions *opts, const int pipe_file)
{
	opts->files[STDIN_FILENO] = pipe_file;
	opts->files[STDOUT_FILENO] = FD_NOT_OPENED;
	opts->files[STDERR_FILENO] = FD_NOT_OPENED;
	opts->pipe_file = FD_NOT_OPENED;
	opts->background = false;
}

static int _pipe_create(ExecOptions *opts)
{
	if (opts->files[STDOUT_FILENO] != FD_NOT_OPENED) {
		//TODO use '/dev/null'
		fprintf(stderr, "already assigned '|'\n");
		return ERR;
	}

	int fds[2];
	if (pipe(fds)) {
		fprintf(stderr, "failed to pipe (%d)\n", errno);
		return ERR;
	}

	opts->pipe_file = fds[STDIN_FILENO];
	opts->files[STDOUT_FILENO] = fds[STDOUT_FILENO];
	opts->background = true;
	return ERR_OK;
}

static unsigned int var_calc_hash(const void *key, const size_t size)
{
	unsigned int v = 0;
	for (unsigned char *p = (unsigned char *) key; *p && (*p != '='); p++)
		v = ((v << CHAR_BIT) | *p) % size;

	return v;
}

static int var_compare(const void *a, const void *b)
{
	unsigned char *x = (unsigned char *) a;
	unsigned char *y = (unsigned char *) b;
	for (; *x && (*x != '='); y++, x++)
		if (*x != *y)
			return 1;

	return ((!(*y) || (*y == '=')) ? 0 : 1);
}

static unsigned char *var_get(hash_t *vars, const unsigned char *key)
{
	if (!key)
		return NULL;

	if (!key[0])
		return NULL;

	unsigned char *p = (unsigned char *) strchr((char *) key, '=');
	if (p)
		return NULL;

	p = hash_get(vars, key);
	if (!p)
		return NULL;

	p = (unsigned char *) strchr((char *) p, '=');
	if (!p)
		return NULL;

	return (p[1] ? &(p[1]) : NULL);
}

static bool var_put(hash_t *vars, const unsigned char *var)
{
	unsigned char *p = hash_get(vars, var);
	if (p) {
		hash_remove(vars, p);
		free(p);
	}

	p = (unsigned char *) strchr((char *) var, '=');
	if (!p)
		return true;

	if (p == var)
		return false;

	if (!p[1])
		return true;

	p = (unsigned char *) malloc(strlen((char *) var) + 1);
	if (!p) {
		fprintf(stderr, "no memory\n");
		return false;
	}

	strcpy((char *) p, (char *) var);

	if (hash_put(vars, p, p)) {
		free(p);
		fprintf(stderr, "no memory\n");
		return false;
	}

	return true;
}

static hash_t *var_create(void)
{
	hash_t *vars = hash_create(MAX_VAR, var_calc_hash, var_compare);
	if (!vars)
		return NULL;

	if (environ)
		for (unsigned char **envp = (unsigned char **) environ;
				*envp; envp++)
			if (!var_put(vars, *envp))
				break;

	return vars;
}

static unsigned char **var_expand(hash_t *vars)
{
	unsigned char **array = (unsigned char **) malloc(
			(vars->num + 1) * sizeof(uintptr_t));
	if (array) {
		unsigned char **p = array;
		for (unsigned int i = 0; i < vars->size; i++) {
			list_t *head = &(vars->tbl[i]);
			for (list_t *entry = list_next(head);
					!list_is_edge(head, entry);
					entry = entry->next) {
				*p = ((hash_entry_t *) entry)->value;
				p++;
			}
		}

		*p = NULL;
	}

	return array;
}

static void var_show_all(const hash_t *vars)
{
	for (int i = 0; i < vars->size; i++) {
		const list_t *head = &(vars->tbl[i]);
		for (const list_t *e = list_next(head); !list_is_edge(head, e);
				e = e->next)
			printf("%s\n", (char *) (((hash_entry_t *) e)->value));
	}
}

static void interpret(hash_t *vars, FILE *in)
{
	do {
		if (interactive) {
			fflush(stderr);
			printf(PROMPT " ");
			fflush(stdout);
		}

		line_clear(&line);

		int c;
		do {
			c = fgetc(in);
			if (c == EOF) {
				if (line_putc(&line, 0x1a))
					line_evaluate(&line, vars);

				return;
			}
		} while (!line_putc(&line, c & 0xff));
	} while (!line_evaluate(&line, vars));
}

int main(int argc, char **argv, char **env)
{
	hash_t *vars = var_create();
	if (!vars) {
		fprintf(stderr, "no memory\n");
		return ERR_MEMORY;
	}

	line_realloc_buf(&line);
	line_realloc_array(&line);

	FILE *in = fopen(DEFAULT_MAIN, "r");
	if (in) {
		interactive = false;
		interpret(vars, in);
		fclose(in);
	}

	in = stdin;
	interactive = true;

	if (argc > 1) {
		FILE *file = fopen(argv[1], "r");
		if (file) {
			in = file;
			interactive = false;
		}
	}

	interpret(vars, in);
	fclose(in);
	line_destroy(&line);
	hash_destroy(vars);
	return ERR_OK;
}
