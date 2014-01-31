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
#include <string.h>
#include <unistd.h>
#include "cunit.h"


char *test_memchr() {
	char *txt = "t\x80st";

	assert_eq("1st chr", &txt[0], memchr(txt, 't', 5));
	assert_eq("2nd chr", &txt[1], memchr(txt, '\x80', 5));
	assert_eq("null chr", &txt[4], memchr(txt, '\0', 5));
	assert_eq("empty", NULL, memchr("", 'X', 5));
	assert_eq("not found (empty)", NULL, memchr(txt, 't', 0));
	assert_eq("not found (len)", NULL, memchr(txt, '\0', 4));
	assert_eq("not found (chr)", NULL, memchr(txt, 'X', 5));

	return NULL;
}

char *test_strchr() {
	char *txt = "t\x80st";

	assert_eq("1st chr", &txt[0], strchr(txt, 't'));
	assert_eq("2nd chr", &txt[1], strchr(txt, '\x80'));
	assert_eq("null chr", &txt[4], strchr(txt, '\0'));
	assert_eq("empty", NULL, strchr("", 'X'));
	assert_eq("not found", NULL, strchr(txt, 'X'));

	return NULL;
}

char *test_strrchr() {
	char *txt = "t\x80st";

	assert_eq("1st chr", &txt[3], strrchr(txt, 't'));
	assert_eq("2nd chr", &txt[1], strrchr(txt, '\x80'));
	assert_eq("null chr", &txt[4], strrchr(txt, '\0'));
	assert_eq("empty", NULL, strrchr("", 'X'));
	assert_eq("not found", NULL, strrchr(txt, 'X'));

	return NULL;
}

char *test_strpbrk() {
	char *txt = "t\x80st";

	assert_eq("1st chr", &txt[0], strpbrk(txt, "t"));
	assert_eq("2nd chr", &txt[1], strpbrk(txt, "\x80"));
	assert_eq("group", &txt[0], strpbrk(txt, "\x80st"));
	assert_eq("empty", NULL, strpbrk("", "XYZ"));
	assert_eq("not found (empty)", NULL, strpbrk(txt, ""));
	assert_eq("not found", NULL, strpbrk(txt, "XYZ"));

	return NULL;
}

char *test_strspn() {
	char *txt = "t\x80st";

	assert_eq("1st chr", 1, strspn(txt, "t"));
	assert_eq("2nd chr", 0, strspn(txt, "\x80"));
	assert_eq("group", 4, strspn(txt, "\x80st"));
	assert_eq("empty", 0, strspn("", "XYZ"));
	assert_eq("not found (empty)", 0, strspn(txt, ""));
	assert_eq("not found", 0, strspn(txt, "XYZ"));

	return NULL;
}

char *test_strcspn() {
	char *txt = "t\x80st";

	assert_eq("1st chr", 0, strcspn(txt, "t"));
	assert_eq("2nd chr", 1, strcspn(txt, "\x80"));
	assert_eq("group", 4, strcspn(txt, "XYZ"));
	assert_eq("empty", 0, strcspn("", "XYZ"));
	assert_eq("full", 4, strcspn(txt, ""));
	assert_eq("not found", 0, strcspn(txt, "\x80st"));

	return NULL;
}

int main(int argc, char **argv)
{
extern char *strchr(const char *s, int c);
	//test(test_memcpy);
	//test(test_memset);

	//test(test_strlen);
	//test(test_strcpy);
	//test(test_strcat);

	//test(test_strncpy);
	//test(test_strncat);

	//test(test_memcmp);
	//test(test_strcmp);
	//test(test_strncmp);

	test(test_memchr);
	test(test_strchr);
	test(test_strrchr);

	test(test_strpbrk);
	test(test_strspn);
	test(test_strcspn);

	//test(test_strerror);

	return 0;
}
