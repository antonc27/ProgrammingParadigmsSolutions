#include <stdio.h>
#include <stdbool.h>

int remainder_devided_by_4(int x) {
  return x & ~((x>>2)<<2);
}

void TestBitwiseDevisionBy4(int x) {
  printf("--- Testing bitwise devision remainder of %d %% %d ---\n", x, 4);
  printf("%d %% %d (built-in) = %d\n", x, 4, x % 4);
  printf("%d %% %d ( bitwise) = %d\n", x, 4, remainder_devided_by_4(x));
  printf("--- ---\n");
} 

bool will_loose_when_assigned_to_short(int x) {
  return x ^ (x<<16)>>16;
}

void TestLoseDataWhenCastingToShort(int x) {
  printf("--- Testing lose data when casting to short for %d ---\n", x);
  printf(will_loose_when_assigned_to_short(x) ? "Will loose data\n" : "Will not loose data\n");
  printf("Getting after casting to short: %hd\n", (short)x);
  printf("--- ---\n");
}

int negate(int x) {
  return ~x + 1;
}

void TestNegation(int x) {
  printf("--- Testing negation for %d ---\n", x);
  int nx = negate(x);
  printf("Negation: %d\n", nx);
  printf("Checking %d + %d = %d\n", x, nx, x + nx);
  printf("--- ---\n");
}

void encrypt_file(char key, char *src_name, char *dest_name) {
  FILE *src_f = fopen(src_name, "r");
  FILE *dest_f = fopen(dest_name, "w");
  int buf = 0;
  while (!feof(src_f) || !ferror(src_f)) {
    if (fread(&buf, sizeof(char), 1, src_f) == 0) {
      break;
    }
    buf ^= key;
    printf("%c", (char)buf);
    fwrite(&buf, sizeof(char), 1, dest_f);
  }
  printf("\n");
  fclose(src_f);
  fclose(dest_f);
}


int main() {
  TestBitwiseDevisionBy4(33);
  TestBitwiseDevisionBy4(24);
  
  TestLoseDataWhenCastingToShort(32000);
  TestLoseDataWhenCastingToShort(650000);
  
  TestNegation(1);
  TestNegation(0);
  TestNegation(-1);
  
  encrypt_file('f', "./test.txt", "./test_enc.txt");
  encrypt_file('f', "./test_enc.txt", "./test_dec.txt");

  return 0;
}
