
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define ARR_SIZEBIG 2 * 1024
#define ARR_SIZESMALL 256
#define LOOP 3000 * 1000
// For 64 KB cache
typedef struct cacheblock {
  uint8_t line[64];
} cacheblock;

int main()
{
// register volatile cacheblock* V_small asm("rax");
//   register volatile cacheblock* V_big asm("rbx");
  // register int i asm("edx");

  // V_small = malloc(ARR_SIZESMALL*sizeof(cacheblock));
  // V_big = malloc(ARR_SIZEBIG*sizeof(cacheblock));
  volatile cacheblock *V_small = malloc(ARR_SIZESMALL*sizeof(cacheblock));
  volatile cacheblock *V_big = malloc(ARR_SIZEBIG*sizeof(cacheblock));
  // i = 0;

  //  volatile cacheblock* V_small = new cacheblock[ARR_SIZESMALL];
  //  volatile cacheblock* V_big = new cacheblock[ARR_SIZEBIG];
  volatile int i = 0;

  while (1) {
    V_small[i & (ARR_SIZESMALL - 1)].line[0] = 2;
    V_big[i & (ARR_SIZEBIG - 1)].line[0] = 5;
    i++; // To get 64 Bytes
    // printf("Address : %p \n",&i);
    if (i > LOOP) {
      // std::cout << "Code Done \n";
      break;
    }
  }
  // for (i = 0; i < 100; i++) {
  //   int temp = V_small[i & (ARR_SIZESMALL - 1)].line[0];
  //   printf("Small : %d \n",temp);
  // }
  return 0;
}