/*
 *	ae.c		Anthony's Editor  IOCCC '91
 *
 *	Public Domain 1991 by Anthony Howe.  All rights released.
 */
#include <ctype.h>
#include "curses.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

static bool done;
static long row;
static long col;
static long index;
static long page;
static long epage;
static char buf[BUF];
static char *ebuf;
static char *gap = buf;
static char *egap;
static char *filename;
static int lastkey;

/*
 *	The following assertions must be maintained.
 *
 *	o  buf <= gap <= egap <= ebuf
 *		If gap == egap then the buffer is full.
 *
 *	o  point = ptr(index) and point < gap or egap <= point
 *
 *	o  page <= index < epage
 *
 *	o  0 <= index <= pos(ebuf) <= BUF
 *
 *
 *	Memory representation of the file:
 *
 *		low	buf  -->+----------+
 *				|  front   |
 *				| of file  |
 *			gap  -->+----------+<-- character not in file
 *				|   hole   |
 *			egap -->+----------+<-- character in file
 *				|   back   |
 *				| of file  |
 *		high	ebuf -->+----------+<-- character not in file
 *
 *
 *	point & gap
 *
 *	The Point is the current cursor position while the Gap is the
 *	position where the last edit operation took place. The Gap is
 *	ment to be the cursor but to avoid shuffling characters while
 *	the cursor moves it is easier to just move a pointer and when
 *	something serious has to be done then you move the Gap to the
 *	Point.
 */

static void to_normal(void);
static void to_esc(void);
static void to_x(void);
static long adjust(long, long);
static long nextline(long);
static long pos(char *);
static long prevline(long);
static char *ptr(long);

static void bottom(void);
static void delete(void);
static void display(void);
static void down(void);
static void file(void);
static void insert(const char);
static void left(void);
static void lnbegin(void);
static void lnend(void);
static void movegap(void);
static void pgdown(void);
static void pgup(void);
static void redraw(void);
static void right(void);
static void quit(void);
static void top(void);
static void up(void);
static void wleft(void);
static void wright(void);

static char *key;
static void (**func)();

static char key_normal[] = "\x02\x0e\x10\x06\x16\x1a\x01\x05\x04\x0c\x1b\x18";
static void (*func_normal[])() = {
	left, down, up, right,
	pgdown, pgup,
	lnbegin, lnend,
	delete, redraw,
	to_esc, to_x
};

static char key_esc[] = "\x07" "bf<>";
static void (*func_esc[])() = {
	to_normal,
	wleft, wright,
	top, bottom
};

static char key_x[] = "\x07\x13\x03";
static void (*func_x[])() = {
	to_normal, file, quit
};

static void to_normal(void)
{
	key = key_normal;
	func = func_normal;
}

static void to_esc(void)
{
	key = key_esc;
	func = func_esc;
}

static void to_x(void)
{
	key = key_x;
	func = func_x;
}

static char *ptr(long offset)
{
	if (offset < 0)
		return buf;

	return (buf + offset + (buf + offset < gap ? 0 : egap - gap));
}

static long pos(char *pointer)
{
	return (pointer - buf - (pointer < egap ? 0 : egap - gap));
}

static void top(void)
{
	index = 0;
	to_normal();
}

static void bottom(void)
{
	epage = index = pos(ebuf);
	to_normal();
}

static void quit(void)
{
	done = true;
	to_normal();
}

static void redraw(void)
{
	clear();
	display();
}

static void movegap(void)
{
	char *p = ptr(index);
	while (p < gap)
		*--egap = *--gap;

	while (egap < p)
		*gap++ = *egap++;

	index = pos(egap);
}

static long prevline(long offset)
{
	char *p;
	while (buf < (p = ptr(--offset)) && *p != '\n')
		;

	return (buf < p ? ++offset : 0);
}

static long nextline(long offset)
{
	char *p;
	while ((p = ptr(offset++)) < ebuf && *p != '\n')
		;

	return (p < ebuf ? offset : pos(ebuf));
}

static long adjust(long offset, long column)
{
	long i = 0;
	for (char *p; (p = ptr(offset)) < ebuf && *p != '\n' && i < column;) {
		i += *p == '\t' ? 8 - (i & 7) : 1;
		++offset;
	}

	return offset;
}

static void left(void)
{
	if (0 < index)
		--index;
}

static void right(void)
{
	if (index < pos(ebuf))
		++index;
}

static void up(void)
{
	index = adjust(prevline(prevline(index) - 1), col);
}

static void down(void)
{
	index = adjust(nextline(index), col);
}

static void lnbegin(void)
{
	index = prevline(index);
}

static void lnend(void)
{
	index = nextline(index);
	left();
}

static void wleft(void)
{
	for (char *p; !isspace(*(p = ptr(index))) && buf < p;)
		--index;

	for (char *p; isspace(*(p = ptr(index))) && buf < p;)
		--index;

	to_normal();
}

static void pgdown(void)
{
	page = index = prevline(epage - 1);

	while (0 < row--)
		down();

	epage = pos(ebuf);
}

static void pgup(void)
{
	for (long i = LINES - 2; 0 < --i;) {
		page = prevline(page - 1);
		up();
	}
}

static void wright(void)
{
	for (char *p; !isspace(*(p = ptr(index))) && p < ebuf;)
		++index;

	for (char *p; isspace(*(p = ptr(index))) && p < ebuf;)
		++index;

	to_normal();
}

static void insert(const char ch)
{
	movegap();

	if (ch == '\b') {
		if (buf < gap)
			--gap;
	} else if (gap < egap) {
		*gap++ = ch == '\r' ? '\n' : ch;
	}

	index = pos(egap);
	display();
}

static void delete(void)
{
	movegap();

	if (egap < ebuf)
		index = pos(++egap);
}

static void file(void)
{
	int fd;
	long j = index;

	index = 0;
	movegap();
	write(fd = creat(filename, MODE), egap, (int) (ebuf - egap));
	close(fd);
	index = j;
	to_normal();
}

static void display(void)
{
	long i;
	long j;

	if (index < page)
		page = prevline(index);

	if (epage <= index) {
		page = nextline(index);
		i = ((page == pos(ebuf)) ? (LINES - 4) : (LINES - 2));
		while (0 < i--)
			page = prevline(page - 1);
	}

	move(0, 0);
	i = j = 0;
	epage = page;

	while (true) {
		if (index == epage) {
			row = i;
			col = j;
		}

		char *p = ptr(epage);
		if (LINES - 2 <= i || ebuf <= p)
			break;

		if (*p != '\r') {
			addch(*p);
			j += *p == '\t' ? 8 - (j & 7) : 1;
		}

		if (*p == '\n' || COLS <= j) {
			++i;
			j = 0;
		}

		++epage;
	}

	clrtobot();

	if (++i < LINES - 2)
		mvaddstr(i, 0, "<< EOF >>");

	// mode line
	char strbuf[128];
	j = snprintf(strbuf, COLS, "%ld, %ld <%d> %s", row, col, lastkey, filename);
	attron(A_REVERSE);
	mvaddstr(LINES - 2, 0, strbuf);
	for (; j < COLS; j++)
		addch(' ');

	attroff(A_REVERSE);

	// mini buffer
	sprintf(strbuf, "%ld, %ld", i, j);
	mvaddstr(LINES - 1, 0, strbuf);

	move(row, col);
	refresh();
}

int main(int argc, char **argv)
{
	egap = ebuf = buf + BUF;

	if (argc < 2)
		return 2;

	initscr();
	raw();
	noecho();
	idlok(stdscr, 1);

	int fd;
	if (0 < (fd = open(filename = *++argv, 0))) {
		gap += read(fd, buf, BUF);
		if (gap < buf)
			gap = buf;

		close(fd);
	}

	top();
	to_normal();

	while (!done) {
		display();

		long i = 0;
		int ch = getch();
		lastkey = ch;
		while (key[i] != '\0' && ch != key[i])
			++i;

		if (key[i])
			(*func[i])();
		else
			insert(ch);
	}

	endwin();
	return 0;
}
