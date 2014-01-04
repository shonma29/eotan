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
#include <iso646.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "cunit.h"

#define to_string(x) str(x)
#define str(x) #x

char *test_iso646() {
	assert_eq("and", 0, strcmp(to_string(and), "&&"));
	assert_eq("and_eq", 0, strcmp(to_string(and_eq), "&="));
	assert_eq("bitand", 0, strcmp(to_string(bitand), "&"));
	assert_eq("bitor", 0, strcmp(to_string(bitor), "|"));
	assert_eq("compl", 0, strcmp(to_string(compl), "~"));
	assert_eq("not", 0, strcmp(to_string(not), "!"));
	assert_eq("not_eq", 0, strcmp(to_string(not_eq), "!="));
	assert_eq("or", 0, strcmp(to_string(or), "||"));
	assert_eq("or_eq", 0, strcmp(to_string(or_eq), "|="));
	assert_eq("xor", 0, strcmp(to_string(xor), "^"));
	assert_eq("xor_eq", 0, strcmp(to_string(xor_eq), "^="));

	return NULL;
}

int main(int argc, char **argv)
{
	test(test_iso646);

	return 0;
}
