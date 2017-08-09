#include <stdio.h>
#include <math.h>

int main(int argc, char *argv[])
{
#ifdef notdef
  int *p;
#endif
  int i, j;
  double x = 1.0;

  printf("argc = %d\n", argc);
  for(i = 0; i < argc; ++i) {
    printf("%s\n", argv[i]);
  }
  printf("Hello World\n");

  printf("%f\n", 0.0175);
  printf("%f\n", 0.175);
  printf("%f\n", 1.75);
  printf("%f\n", -17.5);
  printf("%f\n", sin(x));
  printf("%f\n", cos(x));

#ifdef notdef
  printf("This program intentionally causes page fault.\n");
  p = (int *) 0x80000000;
  i = *p;
  *p = i;
#else
  printf("loop start. wait a moment ...\n");
  for(i = 0; i < 100000000; ++i) ;
  printf("loop end\n");
#endif

  j = 0;
  printf("div %d\n", i / j);

  return 0;
}
