#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>

#define SHM_KEY  105205673
#define US_ONE_SEC 1000000
#define MAX_CPU 6
#define NSAMPLES 10
#define OFFSET 64
#define SHM_SIZE 8192

void helptext() {
    printf("usage:  tsctest -r                  # start reader\n");
    printf("        tsctest -c cpuiud ./a.out   # start writer\n");
    printf("        tsctest -h                  # help text\n");
    exit(0);
}

uint64_t rdtscp(int *chip, int *core)
{
  uint32_t lo, hi, cpuinfo;
  __asm__ volatile("rdtscp" : "=a"(lo), "=d"(hi), "=c"(cpuinfo));
  *chip = (cpuinfo & 0xFFF000) >> 12;
  *core = (cpuinfo & 0x000FFF);
  return ((uint64_t)lo) | (((uint64_t)hi) << 32);
}

uint64_t setup_shared_memory()
{
  uint64_t shmaddr;
  int shmid;
  key_t shmkey = SHM_KEY;
  if  ((shmid = shmget(shmkey, SHM_SIZE, IPC_CREAT | 0666)) == -1) {
    printf("Error creating shared memory segment\n");
    exit(0);
  }

  if ((shmaddr = (uint64_t)shmat(shmid, 0, 0)) == 0) {
    printf("Invalid memory ptr\n");
    exit(0);
  }
  return shmaddr;
}


void writer(uint64_t addr)
{
  int sock0, core0, sock, core;
  unsigned long tsc = rdtscp(&sock0, &core0);
  printf("Starting tsctest writer\n");
  printf("timer: %" PRIu64 ", (core %d, socket %d)\n", tsc, core0, sock0);
  while (1) {
    tsc = rdtscp(&sock, &core);

    if (core != core0 || sock != sock0) {
      printf("moving between cores not supported\n");
      exit(1);
    }

    volatile uint64_t * addrp = (uint64_t *)(addr + OFFSET * core0);
    *addrp = tsc;
  }
}


void reader(uint64_t addr) {
  uint64_t tsc[MAX_CPU][NSAMPLES];
  int64_t  tdiff[MAX_CPU];

  printf("Starting tsctest reader\n");
  printf("Sampling tsc counters\n");
  for (int n = 0; n < NSAMPLES; n++) {
    printf("."); fflush(stdout);
    for (int i = 0; i < MAX_CPU; i++) {
      tsc[i][n] = *(uint64_t *)(addr + i * OFFSET);
    }
    usleep(US_ONE_SEC);
  }
  printf("done\n");

  printf("Calculating averages\n");
  for (int i = 0; i < MAX_CPU; i++) {
    int64_t diff = 0;
    for (int n = 0; n < NSAMPLES; n++) {
      diff += tsc[i][n] -  tsc[0][n];
    }
    tdiff[i] = diff/NSAMPLES;
    printf("avg tsc diff. 0 - %d: %" PRIi64 "\n", i, tdiff[i]);
  }

  printf("Measuring time\n");
  printf("    dt0    dt1    dt2   dt3   dt4   dt5\n");
  while (1) {
    for (int i = 0; i < MAX_CPU; i++) {
      tsc[i][0] = *(uint64_t *)(addr + i * OFFSET);
    }
    for (int i = 0; i < MAX_CPU; i++) {
      printf("%6" PRIi64 " ", tsc[i][0] - tdiff[i] - tsc[0][0]);
    }
    printf("\n");
    usleep(US_ONE_SEC);
  }
}



int main(int argc, char *argv[]) {
  int cpu1, cpu2;

  if ((argc != 1 && argc != 2) || strcmp(argv[0], "-h") ==0 ) {
    helptext();
  }

  uint64_t addr = setup_shared_memory();

  if (argc == 2 && strcmp(argv[1], "-r") == 0) {
    reader(addr);
  } else if (argc == 1) {
    writer(addr);
  }
  helptext();
}
