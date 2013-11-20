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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cunit.h"

#define to_string(x) str(x)
#define str(x) #x

char *test_stdbool_undefined() {
#define __bool_true_false_are_defined 1
#undef bool
#define bool int
#undef true
#define true (49)
#undef false
#define false (74)
#include <stdbool.h>
	assert_eq("undefined cond", 1, __bool_true_false_are_defined);
	assert_eq("undefined bool", 0,
			strcmp("int", to_string(bool)));
	assert_eq("undefined true", 49, true);
	assert_eq("undefined false", 74, false);

	return NULL;
}

char *test_stdbool_defined() {
#undef __bool_true_false_are_defined
#undef bool
#undef true
#undef false
#undef _STDBOOL_H_
#include <stdbool.h>
	assert_eq("defined cond", 1, __bool_true_false_are_defined);
	assert_eq("defined bool", 0,
			strcmp("_Bool", to_string(bool)));
	assert_eq("defined true", 1, true);
	assert_eq("defined false", 0, false);

	return NULL;
}

int main(int argc, char **argv)
{
	test(test_stdbool_undefined);
	test(test_stdbool_defined);

	return 0;
}
