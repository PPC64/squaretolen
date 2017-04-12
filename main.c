#include <stdlib.h>  // for strtol
#include "squaretolen.h"

#ifdef TIME
#include <time.h>
#endif

void print_result(int *x, int len)
{
  for(size_t i=0; i < len; i++){
    if (i > 0) {
      if (i%16 == 0)
        printf("\n");
    }
    printf("%08x ",x[i]);
  }
  printf("\n");
}

int main (int argc, char** argv) {
  int size = 32;
  if (argc > 1)
    size = atoi(argv[1]);

  int* in;
  if (size == 0) {
    // Easter egg?!
    size = 34;
    in = malloc(size * sizeof(int));
    in[ 0] = 0x0006e55e; in[ 1] = 0x1603ecab; in[ 2] = 0x494bcac2;
    in[ 3] = 0x7d2e91d5; in[ 4] = 0x79ffc9b6; in[ 5] = 0xc4e88cb2;
    in[ 6] = 0xe4aee2c4; in[ 7] = 0x0588c3bb; in[ 8] = 0x28d2d344;
    in[ 9] = 0xcf649c89; in[10] = 0x9c147afa; in[11] = 0xed1c0f6e;
    in[12] = 0xb644b286; in[13] = 0xf7b7d8c0; in[14] = 0x5d48c108;
    in[15] = 0x6bb70e4a; in[16] = 0x9c9534d0; in[17] = 0xcf92da5f;
    in[18] = 0x0609e2c5; in[19] = 0x9bfb20dd; in[20] = 0x25ed56d9;
    in[21] = 0x500b1638; in[22] = 0xcc11727f; in[23] = 0x6e3d6f82;
    in[24] = 0xb5f8e403; in[25] = 0x80d72ac8; in[26] = 0x3b6c18cd;
    in[27] = 0xf5b5a28a; in[28] = 0x04a5423b; in[29] = 0x918f39a5;
    in[30] = 0xeb63c980; in[31] = 0xfe77a79e; in[32] = 0x16240468;
    in[33] = 0x385884f3;
  }
  else {
    in = malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
      in[i] = 1; //*i*(in[i-1]+1);
    }
  }

#ifdef TIME
  clock_t t = clock();
#else
  printf("in:%d\n", size);
  print_result(in, size);
#endif

  int* out = malloc(size*2 * sizeof(int));
  out = SquareToLen(in, size, out, size*2);

#ifdef TIME
  t = clock() - t;
  double time_taken = ((double)t)/CLOCKS_PER_SEC;
  printf("%d %f\n", size, time_taken);
#else
  printf("\nout:%d\n", size*2);
  print_result(out, size*2);
#endif

  free(in);
  free(out);
  return 0;
}
