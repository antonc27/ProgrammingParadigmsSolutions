#include <stdio.h>
#include <stdbool.h>

bool is_little_endian() {
  int i = 1;
  return *(unsigned char *)&i == 1;
}

typedef unsigned char *bytes_pointer;

void print_bytes(bytes_pointer bt, size_t size) {
  size_t i;
  if (is_little_endian()) {
    for (i = size; i > 0; i--) {
      printf(" %.2x", bt[i-1]);
    }
  } else { 
    for (i = 0; i < size; i++) {
      printf(" %.2x", bt[i]);
    }
  }
  printf("\n");
}

void print_short(short x) {
  print_bytes((bytes_pointer) &x, sizeof(short));
}

bool tadd_ok(short x, short y) {
  short sum = x+y;
  if (x > 0 && y > 0 && sum <= 0) {
    return false;
  } else if (x < 0 && y < 0 && sum >= 0) {
    return false;
  }
  return true;
}

void print_short_info(char *desc, short s) {
  printf("%s dec %hd\n", desc, s);
  printf("%s hex", desc);
  print_short(s);
  printf("\n");
}

void test_short_overflow(short s1, short s2) {
  printf("--- Testing shorts s1(%hd)+s2(%hd) for overflow ---\n", s1, s2);
  print_short_info("s1", s1);
  print_short_info("s2", s2);
  print_short_info("s1+s2", s1+s2);
  printf(tadd_ok(s1, s2) ? "Not overflows\n" : "Overflows\n");
  printf("--- ---\n");
}

void OverflowTest() {
  test_short_overflow(20000, 10000);
  test_short_overflow((short)((1<<15)-1), 1);
}

int main() {
  OverflowTest();
  return 0;
}
