#include <stdio.h>

void PrintByte(long int num, int size) {
  int n = size*8;
  char str[n+1];
  str[n] = '\0';
  int count = 0;
  while (count < n) {
    str[n - count - 1] = (num & 1) ? '1' : '0';
    num = num >> 1;
    count++;
  }
  printf("%s", str);
}

void OverflowTest() {
  //  printf("%lu %lu\n", sizeof(short), sizeof(unsigned short));

  short minShort = -(((unsigned short)~0) / 2 + 1);
  printf("%hd %hd\n", minShort, (short)(minShort-1));

  printf("%hd\n", (short)(~0));

  short s = (short)2;//minShort;
  /*
  int n = sizeof(s)*8;
  char str[n+1];
  str[n] = '\0';
  int count = 0;
  while (count < n) {
    str[n - count - 1] = (s & 1) ? '1' : '0';
    s = s >> 1;
    count++;
  }
  printf("%s", str);
  */
  PrintByte(s, sizeof(s));
  printf("\n");

  /*
  short s = 0;
  while ((short)(s-1) < s) {
    printf("%hd %hd %hd\n", s, (short)(s-1), (short)(s-2));
    //    printf("%d\n", s);
    s--;
  }
  */

  /*
  int i;
  for (i = 0; i <= maxShort; i++)
    printf("%d\n", i);
  */
}

int main() {
  OverflowTest();
  return 0;
}
