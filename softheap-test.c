#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "softheap.h"


#define N_ELEMENTS 1 << 10
#define EPSILON 0.01
#define MAGIC_PRIME_ONE 1399
#define MAGIC_PRIME_TWO 1093

/* static int intcmp(const void *one, const void *two) { */
/*   return *(int *)one - *(int *)two; */
/* } */

int main() {
  // int results[N_ELEMENTS];

  srand(time(NULL));

  softheap *P = makeheap_empty(EPSILON);
  for(int i = 0; i < N_ELEMENTS; i++) insert(P, rand() % 100);  //insert(P, N_ELEMENTS - i);
  //  while(!empty(P)) extract_min(P);
  for(int i = 0; i < N_ELEMENTS; i++) printf("%d ", extract_min(P));
  printf("\n");

  free(P);
  return 0;
}
