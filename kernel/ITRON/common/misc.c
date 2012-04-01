/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001, Tomohide Naniwa

*/
/* misc.c ---
 *
 */

#include "itron.h"

/*************************************************************************
 * halfword_swap
 *
 * 引数：
 *
 * 返値：
 *
 * 処理：
 *
 */
UH halfword_swap(UH w)
{
    w = ((w & 0xff) << 8) | ((w & 0xff00) >> 8);
    return (w);
}

/**************************************************************************
 *
 *
 */
VP bcopy(VP src, VP dest, W count)
{
    while (count-- > 0) {
	*((B *) dest)++ = *((B *) src)++;
    }
    return (dest);
}

/**************************************************************************
 *
 *
 */
void bzero(VP src, W count)
{
    while (count-- > 0) {
	*((UB *) src)++ = 0;
    }
}

/**************************************************************************
 *
 *
 */
W strlen(B * buf)
{
    int i;

    for (i = 0; *buf++ != '\0'; i++);
    return (i);
}


/**************************************************************************
 *
 *
 */
W strcpy(B * s1, B * s2)
{
    int i;

    for (i = 0; *s2 != NULL; i++) {
	*s1++ = *s2++;
    }
    *s1 = '\0';
    return (i);
}

/**************************************************************************
 *
 *
 */
W strncmp(B * s1, B * s2, W size)
{
    W i;

    for (i = 0; i < size; ++i, ++s1, ++s2) {
	if ((*s1 - *s2 != 0) || (*s1 == '\0') || (*s2 == '\0'))
	    return (*s1 - *s2);
    }
    return (*s1 - *s2);
}

/**************************************************************************
 *
 *
 */
W strcmp(B * s1, B * s2)
{
    while ((*s1 == *s2) && (*s1)) {
	++s1;
	++s2;
    }
    return (*s1 - *s2);
}

W atoi(B * s)
{
    W result;
    BOOL minus = FALSE;

    result = 0;
    if (*s == '-') {
	s++;
	minus = TRUE;
    }

    while (*s != '\0') {
	result = (result * 10) + (*s - '0');
	s++;
    }
    return (minus ? -result : result);
}


char tolower(char c)
{
    if (c >= 'A') {
	if (c <= 'Z') {
	    return c - ('A' - 'a');
	}
    }
    return c;
}

char toupper(char c)
{
    if (c >= 'a') {
	if (c <= 'z') {
	    return c - ('a' - 'A');
	}
    }
    return c;
}
