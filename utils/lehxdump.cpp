//
//
// Simple utility that reads binary data in 32 bit chunks in little endian
// and prints it as big endian for human parseability

#include <cinttypes>
#include <cstdio>

int main(int argc, char *argv[]) {
  FILE *f = fopen(argv[1], "r");
  if (f == NULL) {
    printf("Cannot find file \'%s\'\n", argv[1]);
    return -1;
  }

  uint32_t data;
  uint32_t addr = 0;

  while (1) {
    if ((addr % 16) == 0)
      printf("\n%08x    ", addr);
    if (fread(&data, sizeof(data), 1, f) <= 0)
      return 0;
    addr += sizeof(data);
    printf("%08x ", data);
  }

  return 0;
}
