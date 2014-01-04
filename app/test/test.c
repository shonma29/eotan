#include <stdio.h>

int main(int argc, char *argv[])
{
  int i, j;

  printf("argc = %d\n", argc);
  for(i = 0; i < argc; ++i) {
    printf("%s\n", argv[i]);
  }
  printf("Hello World\n");
#ifdef notdef
  printf("This program intentionally causes page fault.\n");
  i = * ((int *) 0);
  *((int *) 0) = i;
#else
  printf("loop start. wait a moment ...\n");
  for(i = 0; i < 100000000; ++i) ;
  printf("loop end\n");
#endif

  j = 0;
  printf("div %d\n", i / j);

  return 0;
}
