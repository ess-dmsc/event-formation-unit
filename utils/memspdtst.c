#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DCACHE_SIZE (1024 * 15360) // mjc workstation

int dummy(int n) {
  char *array1, *array2;
  size_t index, count;
  int cpys = 0;

  array1 = malloc(5 * DCACHE_SIZE);
  array2 = malloc(5 * DCACHE_SIZE);
  for (index = 0, count = 87654321; count--;
       index = (index + 3) % (5 * DCACHE_SIZE)) {
    memcpy(&array1[index], &array2[index], 1);
    cpys++;
  }
  return cpys;
}

int main(int argc, char *argv[]) {
  int size = 87654321;
  int cpys;
  cpys = dummy(size);
  printf("%d Mb\n", cpys*8/1000000);
}
