/*
   FoRTh Modoki
   frtm.c ver 1.4.3 EOTA/init 1.14
   Copyright (C) 1999-2002 Tomohide Naniwa. All rights reserved.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Last modified: Sep. 23, 2002.
*/
#define GREETING "FoRTh Modoki Ver. 1.4.3 for INIT 1.14"
#define GREETING2 "Copyright (C) 1999-2002 Tomohide Naniwa\nFoRTh Modoki comes with NO WARRANTY."

#define EOTA
#ifndef EOTA
#include <stdio.h>
#else
#include <stdlib.h>
#endif

#define MAIN
#include "frtm.h"
#include "errmes.h"

#define PROMPT "init> "
#define PROMPT2 "dict> "

#ifndef WCONSOLE
frtm_main()
{
  static char line[LINE];
  long int read_line_edit();
  static int first = 1;
  int pid, res;

  printf("%s\n", GREETING);
  printf("%s\n", GREETING2);
  init_hash();
  error_no = E_OK;
  while(1) {
    if (first) {
      eval("(/init.fm) load");
      first = 0;
    }
    else {
      pid = waitpid(-1, &res, WNOHANG);
      if (pid > 0) {
	printf("child process exited pid=%d stat=%d\n", pid, res);
	continue;
      }
      if (eval_mode == DIC1 || eval_mode == DIC2) {
	printf("%s", PROMPT2);
      }
      else 
	printf("%s", PROMPT);
#ifdef EOTA
      read_line_edit(line, (long int) sizeof(line));
#else
      fgets(line, LINE, stdin);
#endif

      eval(line);
    }

    /* エラーリカバリー */
    if (error_no == E_QUT) {
      break;
    }
    else if (error_no != E_OK) {
      printf("ERROR: %s\n", error_mess[error_no]);
      error_no = E_OK;
      reload_old_csp();
      if (eval_mode == DIC2) {
	/* 未完辞書の破棄 */
	drop_dict();
      }
      eval_mode = RUN;
      memory = OFF;
      clear_stack();
      clear_rstack();
      clear_sstack();
    }
  }
#ifdef EOTA
  return(0);
#endif
}
#else
#endif

eval(line)
     char *line;
{
  int i, j, len, quote;
  static char subst[LINE];

  i = 0; len = strlen(line);
  while(i < len) {
    j = 0;
    if (isspace(line[i])) {
      ++i; continue;
    }
    quote = 0;
    if (line[i] == '(') quote = 1;
    while(((! isspace(line[i])) || (quote == 1)) && line[i]) {
      if (line[i] == ')') {
	quote = 2;
	subst[j++] = line[i++];
      }
      else if (line[i] == '\\') {
        switch(line[i+1]) {
        case 'n':
          subst[j++] = '\n';
          i += 2;
          break;
        case 'b':
          subst[j++] = '\b';
          i += 2;
          break;
        case 't':
          subst[j++] = '\t';
          i += 2;
          break;
        case '0':
          if ((line[i+2] >= '0' && line[i+2] <= '7') &&
	      (line[i+3] >= '0' && line[i+3] <= '7')) {
            subst[j++] = (line[i+2]-'0') * 8 + (line[i+3] - '0');
	    i += 4;
	  }
          else {
            subst[j++] = line[i+1];
	    i += 2;
          }
          break;
        default:
          if (line[i+1]) {
            subst[j++] = line[i+1];
            i += 2;
          }
          else
            subst[j++] = line[i++];
          break;
        }
      }
      else
        subst[j++] = line[i++];
    }
    if (quote == 1) {
      error_no = E_UCB;
      break;
    }
    else if (quote == 2) {
      if (line[i-1] != ')') {
	error_no = E_UCB;
	break;
      }
    }
    subst[j] = 0;
    eval_word(subst);
    if (error_no != E_OK) break;
  }
}

eval_word(str)
     char *str;
{
  int val, i, sgn = 1;
  char *ostr;

  ostr = str;
  if (*str == '+') ++str;
  else if (*str == '-') {sgn = -1; ++str;}
  if (isdigit(*str)) {
    if (*str == '0' && str[1] == 'x') {
      /* 16 進数の読み込み */
      for(i = 2; str[i]; ++i) {
	if (! isxdigit(str[i])) break;
      }
      if (! str[i]) {
#ifdef EOTA
	val = xtoi(&(str[2]));
#else
	sscanf(&(str[2]), "%x", &val);
#endif
	val *= sgn;
	switch(eval_mode) {
	case RUN:
	  push(val); EC;
	case SKIP:
	  if (memory == ON) {
	    cpush_int(val); EC;
	  }
	  return;
	case FGT:
	case DEF:
	case VAR:
	case DIC1:
	  error_no = E_IVN;
	  return;
	case DIC2:
	  /* 辞書登録 */
	  cpush_int(val); EC;
	  return;
	}
      }
    }
    else if (*str == '0' && isdigit(str[1])) {
      /* 8 進数の読み込み */
      for(i = 1; str[i]; ++i) {
	if (str[i] < '0' || str[i] > '7') break;
      }
      if (! str[i]) {
#ifdef EOTA
	val = otoi(&(str[1]));
#else
	sscanf(str, "%o", &val);
#endif
	val *= sgn;
	switch(eval_mode) {
	case RUN:
	  push(val); EC;
	case SKIP:
	  if (memory == ON) {
	    cpush_int(val); EC;
	  }
	  return;
	case FGT:
	case DEF:
	case VAR:
	case DIC1:
	  error_no = E_IVN;
	  return;
	case DIC2:
	  /* 辞書登録 */
	  cpush_int(val); EC;
	  return;
	}
      }
    }
    else {
      /* 10 進数の読み込み */
      for(i = 1; str[i]; ++i) {
	if (! isdigit(str[i])) break;
      }
      if (! str[i]) {
#ifdef EOTA
	val = atoi(str);
#else
	sscanf(str, "%d", &val);
#endif
	val *= sgn;
	switch(eval_mode) {
	case RUN:
	  push(val); EC;
	case SKIP:
	  if (memory == ON) {
	    cpush_int(val); EC;
	  }
	  return;
	case FGT:
	case DEF:
	case DIC1:
	  error_no = E_IVN;
	  return;
	case DIC2:
	  /* 辞書登録 */
	  cpush_int(val); EC;
	  return;
	}
      }
    }
  }
  interp(ostr); EC;
}

#ifdef EOTA
int isxdigit(c)
     char c;
{
  if (c >= 'A' && c <= 'Z') return 1;
  else if (c >= 'a' && c <= 'z') return 1;
  else if (c >= '0' && c <= '9') return 1;
  else return 0;
}

int xtoi(str)
     char *str;
{
  int i, v;
  char c;
  for (i = 0, v = 0; (c = str[i]) != 0; ++i) {
    v <<= 4;
    if (c >= 'A' && c <= 'Z') v += c-'A'+10;
    else if (c >= 'a' && c <= 'z') v += c-'a'+10;
    else if (c >= '0' && c <= '9') v += c-'0';
  }
  return v;
}

int otoi(str)
     char *str;
{
  int i, v;
  char c;
  for (i = 0, v = 0; (c = str[i]) != 0; ++i) {
    if (c < '0' || c > '9') break;
    v = (v*8) + c - '0';
  }
  return v;
}

#define C(x)            (x & 0x1f)

#if 0
int read_line_edit(char *line, int length)
{
  int i;
  int ch;
  
  for (i = 0; i < length-1;) {
    ch = getc(stdin);
    if ((ch == C('m')) || (ch == C('j'))) {
      __putc(ch, stdout);
      break;
    }
    else if (ch == '\b') {
      if (i > 0) {
        --i; line[i] = 0;
	__putc(ch, stdout);
      }
    }
    else if (isprint (ch)) {
      __putc(ch, stdout);
      line[i++] = ch;
    }
  }
  line[i] = '\0';
  return (i);
}
#endif
#endif
