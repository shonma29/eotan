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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <set/hash.h>
#include <sys/param.h>

#define ERR (-1)
#define ERR_OK (0)
#define ERR_MEMORY (-2)

#define DIRS_DELIMITER ':'
#define PATH_DELIMITER '/'

#define DEFAULT_BUF_SIZE 1024
#define SHIFT_BUF_SIZE 1
#define DEFAULT_ARRAY_SIZE 16
#define SHIFT_ARRAY_SIZE 1

#define MAX_ENV (256)

typedef struct {
	char *head;
	size_t len;
	size_t max;
	bool last;
	char buf[MAXPATHLEN];
} Token;

static size_t bufsize = DEFAULT_BUF_SIZE >> 1;
static size_t arraysize = DEFAULT_ARRAY_SIZE >> 1;
static char *buf = NULL;
static char **array = NULL;

static int get_path(Token *s);
static bool get_size(size_t *len, const char *head);
static void execute(char **env, const char *path);
static unsigned int env_calc_hash(const void *key, const size_t size);
static int env_compare(const void *a, const void *b);
static char *env_get(hash_t *hash, const char *key);
static bool env_put(hash_t *hash, const char *envp);
static void env_store(hash_t *hash);
static char **env_expand(hash_t *hash);


static int get_path(Token *token)
{
	char *head = token->head;
	char *p;

	for (p = head;; p++) {
		char c = *p;

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

	token->len = (size_t)p - (size_t)head;
	if (!token->len)
		return 1;

	if (p[-1] == PATH_DELIMITER) {
		if (token->len > token->max)
			return 1;

		memcpy(token->buf, head, token->len);

	} else {
		if (token->len + 1 > token->max)
			return 1;

		memcpy(token->buf, head, token->len);
		token->buf[token->len++] = PATH_DELIMITER;
	}

	return 0;
}

static bool get_size(size_t *len, const char *head)
{
	bool has_path = false;
	const char *p;

	for (p = head; *p; p++)
		if (*p == PATH_DELIMITER)
			has_path = true;

	*len = (size_t)p - (size_t)head;

	return has_path;
}

static void execute(char **env, const char *path)
{
	pid_t pid = fork();

	/* child */
	if (pid == 0) {
		Token token;
		bool has_path = get_size(&(token.len), array[0]);

		if (++token.len > MAXPATHLEN)
			exit(ENAMETOOLONG);

		if (has_path) {
			if (execve(array[0], array, env) == ERR) {
				if (errno == ENOENT)
					printf("%s not found\n", array[0]);
				else
					printf("exec error %d\n", errno);

				exit(errno);
			}

		} else if (!path)
			exit(ENOENT);

		else {
			token.head = (char*)path;
			token.max = MAXPATHLEN - token.len;
			token.last = false;

			do {
				if (get_path(&token)) {
					printf("bad path\n");
					break;
				}

				strcpy(&(token.buf[token.len]), array[0]);
				if (execve(token.buf, array, env) == ERR)
					if (errno != ENOENT) {
						printf("exec error %d\n",
								errno);
						exit(errno);
					}

			} while (!(token.last));

			printf("%s not found\n", array[0]);
			exit(ENOENT);
		}
	}

	/* error */
	else if (pid == ERR)
		printf("fork error %d\n", pid);

	/* parent */
	else {
		int status;

		if (waitpid(pid, &status, 0) != pid)
			printf("waitpid error %d\n", errno);
		else
			printf("waitpid success %d\n", status);
	}
}

static void get_buf()
{
	char *buf2 = (char*)malloc(sizeof(char) * (bufsize << SHIFT_BUF_SIZE));

	if (buf2) {
		if (buf) {
			memcpy(buf2, buf, bufsize);
			free(buf);
		}

		buf = buf2;
		bufsize <<= SHIFT_BUF_SIZE;
	}
	else {
		printf("memory exhausted\n");
		exit(ERR_MEMORY);
	}
}

static void get_array()
{
	char **array2 = (char**)malloc(sizeof(char*)
			* (arraysize << SHIFT_ARRAY_SIZE));

	if (array2) {
		if (array) {
			memcpy(array2, array, sizeof(char*) * arraysize);
			free(array);
		}

		array = array2;
		arraysize <<= SHIFT_ARRAY_SIZE;
	}
	else {
		printf("memory exhausted\n");
		exit(ERR_MEMORY);
	}
}

static void parse()
{
	size_t i = 0;
	size_t pos = 0;

	for (;;) {
		for (; (buf[i] > 0) && (buf[i] <= ' '); i++);
		if (!buf[i])
			break;

		array[pos++] = &buf[i];
		if (pos >= arraysize)
			get_array();

		for (; buf[i] > ' '; i++);
		if (!buf[i])
			break;

		buf[i++] = '\0';
	}

	array[pos] = NULL;
}

static unsigned int env_calc_hash(const void *key, const size_t size)
{
	unsigned int v = 0;
	unsigned char *p;

	for (p = (unsigned char*)key; *p && (*p != '='); p++)
		v = ((v << CHAR_BIT) | *p) % size;

	return v;
}

static int env_compare(const void *a, const void *b)
{
	unsigned char *x = (unsigned char*)a;
	unsigned char *y = (unsigned char*)b;

	for (; *x && (*x != '='); y++, x++)
		if (*x != *y)
			return 1;

	return (!(*y) || (*y == '='))? 0:1;
}

static char *env_get(hash_t *hash, const char *key)
{
	char *p;

	if (!key)
		return NULL;

	if (!key[0])
		return NULL;

	p = strchr(key, '=');
	if (p)
		return NULL;

	p = hash_get(hash, key);
	if (!p)
		return NULL;

	p = strchr(p, '=');
	if (!p)
		return NULL;

	return p[1]? &(p[1]):NULL;
}

static bool env_put(hash_t *hash, const char *env)
{
	char *p = hash_get(hash, env);

	if (p) {
		hash_remove(hash, p);
		free(p);
	}

	p = strchr(env, '=');

	if (!p)
		return true;
	if (p == env)
		return false;
	if (!p[1])
		return true;

	p = (char*)malloc(strlen(env));
	if (!p) {
		printf("no memory\n");
		return false;
	}

	strcpy(p, env);

	if (hash_put(hash, p, p)) {
		free(p);
		printf("no memory\n");
		return false;
	}

	return true;
}

static void env_store(hash_t *hash)
{
	if (environ) {
		char **envp;

		for (envp = environ; *envp; envp++)
			if (!env_put(hash, *envp))
				break;
	}
}

static char **env_expand(hash_t *hash)
{
	char **array = (char**)malloc((hash->num + 1) * sizeof(uintptr_t));

	if (array) {
		size_t i;
		char **p = array;

		for (i = 0; i < hash->size; i++) {
			list_t *head = &(hash->tbl[i]);
			list_t *entry;

			for (entry = list_next(head);
					!list_is_edge(head, entry);
					entry = entry->next)
				*p++ = ((hash_entry_t*)entry)->value;
		}

		*p = NULL;
	}

	return array;
}

int main(int argc, char **argv, char **env)
{
	hash_t *hash = hash_create(MAX_ENV, env_calc_hash, env_compare);

	if (hash)
		env_store(hash);
	else {
		printf("no memory\n");
		return ERR_MEMORY;
	}

	get_buf();
	get_array();

	for (;;) {
		size_t pos;

		printf("$ ");

		for (pos = 0;;) {
			char c = fgetc(stdin);

			putchar(c);

			if (c == '\n') {
				buf[pos] = '\0';
				break;

			} else if (c == 0x08) {
				if (pos)
					pos--;

			} else {
				buf[pos++] = c;

				if (pos >= bufsize)
					get_buf();
			}
		}

		parse();
		if (array[0]) {
			if (!strcmp(array[0], "cd")) {
				if (chdir(array[1]? array[1]:"/"))
					printf("chdir error=%d\n", errno);

			} else if (!strcmp(array[0], "exit"))
				break;

			else if (!strcmp(array[0], "env")) {
				if (environ) {
					char **envp;

					for (envp = environ; *envp; envp++)
						printf("%s\n", *envp);
				}

			} else if (!strcmp(array[0], "export"))
				env_put(hash, array[1]);

			else {
				char **envp = env_expand(hash);

				execute(envp, env_get(hash, "PATH"));
				free(envp);
			}
		}
	}

	if (buf)
		free(buf);

	if (array)
		free(array);

	return ERR_OK;
}
