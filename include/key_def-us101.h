/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/include/keyboard_def.h,v 1.2 2000/07/02 15:03:09 kishida0 Exp $ */

/*
 * $Log: keyboard_def.h,v $
 * Revision 1.2  2000/07/02 15:03:09  kishida0
 * add #ifndef and #define line
 *
 * Revision 1.1  2000/01/30 18:54:08  kishida0
 * add keyboard keytop define
 *
 *
 *
 */
#ifndef __KEYBOARD_DEF_US101_H__
#define __KEYBOARD_DEF_US101_H__

#include "keycode.h"

static H        key_table[3][128] =
{
{/* us101 - normal */
/*  0*/ NOKEY,   ESC,   '1',   '2',   '3',   '4',   '5',   '6',
/*  8*/   '7',   '8',   '9',   '0',   '-',   '=',    BS,   TAB,
/* 10*/   'q',   'w',   'e',   'r',   't',   'y',   'u',   'i',
/* 18*/   'o',   'p',   '[',   ']',  '\n',  CTRL,   'a',   's',
/* 20*/   'd',   'f',   'g',   'h',   'j',   'k',   'l',   ';',
/* 28*/  '\'',   '`', SHIFT,  '\\',   'z',   'x',   'c',   'v',
/* 30*/   'b',   'n',   'm',   ',',   '.',   '/', SHIFT,   '0',
/* 38*/   ALT, SPACE,     0,  FN01,  FN02,  FN03,  FN04,  FN05,
/* 40*/  FN06,  FN07,  FN08,  FN09,  FN10, PAUSE, SCROL,   '7',
/* 48*/   '8',   '9',  NMIN,   '4',   '5',   '6',   '+',   '1',
/* 50*/   '2',   '3', INSER,   DEL,     0,     0,     0,  FN11,
/* 58*/  FN12,     0,     0, LWinK, RWinK,  MENU,     0,     0,
/* 60*/     0,     0,     0,     0,     0,     0,     0,     0,
/* 68*/     0,     0,     0,     0,     0,     0,     0,     0,
/* 70*/  KANA,     0,     0, NOKEY,     0,     0,     0,     0,
/* 78*/     0,  XFER,     0,  NEXF,     0, NOKEY,     0,     0,
},

{/* us101 - Shift */
/*  0*/ NOKEY,   ESC,   '!',   '@',   '#',   '$',   '%',   '^',
/*  8*/   '&',   '*',   '(',   ')',   '_',   '+',    BS,   TAB,
/* 10*/   'Q',   'W',   'E',   'R',   'T',   'Y',   'U',   'I',
/* 18*/   'O',   'P',   '{',   '}',  '\n',  CTRL,   'A',   'S',
/* 20*/   'D',   'F',   'G',   'H',   'J',   'K',   'L',   ':',
/* 28*/  '\"',   '~', SHIFT,   '|',   'Z',   'X',   'C',   'V',
/* 30*/   'B',   'N',   'M',   '<',   '>',   '?', SHIFT,   '-',
/* 38*/   ALT, SPACE,     0, SFN01, SFN02, SFN03, SFN04, SFN05,
/* 40*/ SFN06, SFN07, SFN08, SFN09, SFN10, PAUSE, SCROL,   '7',
/* 48*/   '8',   '9',  NMIN,   '4',   '5',   '6',   '+',   '1',
/* 50*/   '2',   '3', INSER,   DEL,     0,     0,     0, SFN11,
/* 58*/ SFN12,     0,     0, LWinK, RWinK,  MENU,     0,     0,
/* 60*/     0,     0,     0,     0,     0,     0,     0,     0,
/* 68*/     0,     0,     0,     0,     0,     0,     0,     0,
/* 70*/  KANA,     0,     0, NOKEY,     0,     0,     0,     0,
/* 78*/     0,  XFER,     0,  NEXF,     0, NOKEY,     0,     0,
},

{/* us101 - control */
/*  0*/ NOKEY,   ESC,   '1',   '2',   '3',   '4',   '5',   '6',
/*  8*/   '7',   '8',   '9',   '0',   '-',   '=',    BS,   TAB,
/* 10*/C('q'),C('w'),C('e'),C('r'),C('t'),C('y'),C('u'),C('i'),
/* 18*/C('o'),C('p'),C('['),C(']'),  '\n',  CTRL,C('a'),C('s'),
/* 20*/C('d'),C('f'),C('g'),C('h'),C('j'),C('k'),C('l'),   ';',
/* 28*/  '\'',   '~',SHIFT,C('\\'),C('z'),C('x'),C('c'),C('v'),
/* 30*/C('b'),C('n'),C('m'),   '<',   '>',   '/', SHIFT,   '-',
/* 38*/   ALT,C(' '),     0, CFN01, CFN02, CFN03, CFN04, CFN05,
/* 40*/ CFN06, CFN07, CFN08, CFN09, CFN10, PAUSE, SCROL,   '7',
/* 48*/   '8',   '9',  NMIN,   '4',   '5',   '6',   '+',   '1',
/* 50*/   '2',   '3', INSER,   DEL,     0,     0,     0, CFN11,
/* 58*/ CFN12,     0,     0, LWinK, RWinK,  MENU,     0,     0,
/* 60*/     0,     0,     0,     0,     0,     0,     0,     0,
/* 68*/     0,     0,     0,     0,     0,     0,     0,     0,
/* 70*/  KANA,     0,     0, NOKEY,     0,     0,     0,     0,
/* 78*/     0,  XFER,     0,  NEXF,     0, NOKEY,     0,     0,
},
};


#endif /* __KEYBOARD_DEF_US101_H__ */

