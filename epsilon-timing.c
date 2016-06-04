#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include <softheap.h>


void time_insert_extract(int tries, int n) {
  int elts[n];

  int max_r = ceil(-log(1./n)/log(2))+5;
  double cumul_insert[max_r+1];
  double cumul_extract[max_r+1];
  for(int i = 0; i < max_r+1; i++)
    cumul_insert[i] = cumul_extract[i] = 0;

  printf("--------------- Insert-Extract: %d ---------------\n", n);

  for(int i = 0; i < tries; i++) {

    for(int j = 0; j < n; j++)
      elts[j] = rand();

    // go over all relevant values of r(epsilon)
    for(int k = 1; k < n; k *= 2) {
      double epsilon = ((double)k)/n;
      int r = ceil(-log(epsilon)/log(2)) + 5;

      softheap *P = makeheap_empty(epsilon);

      clock_t start = clock();
      for(int j = 0; j < n; j++) {
        insert(P, elts[j]);
      }
      clock_t stop = clock();
      double insert_secs = (double)(stop-start) / CLOCKS_PER_SEC;

      start = clock();
      for(int j = 0; j < n; j++) {
        extract_min(P);
      }
      stop = clock();
      double extract_secs = (double)(stop-start) / CLOCKS_PER_SEC;

      destroy_heap(P);

      cumul_insert[r] += insert_secs;
      cumul_extract[r] += extract_secs;
    }
  }

  for(int i = max_r; i > 5; i--) {
    printf("r=%d \t average insert: %f \t average extract: %f\n", i, cumul_insert[i]/tries, cumul_extract[i]/tries);
  }
}

void time_meld(int tries, int n) {
  int elts1[n];
  int elts2[n];

  int max_r = ceil(-log(1./n)/log(2))+5;
  double cumul[max_r+1];
  for(int i = 0; i < max_r+1; i++)
    cumul[i] = 0;

  printf("--------------- Meld: %d (%d tries) ----------\n", n, tries);

  for(int i = 0; i < tries; i++) {

    for(int j = 0; j < n; j++) {
      elts1[j] = rand();
      elts2[i] = rand();
    }

    // go over all relevant values of r(epsilon)
    for(int k = 1; k < n; k *= 2) {
      double epsilon = ((double)k)/n;
      int r = ceil(-log(epsilon)/log(2)) + 5;

      softheap *P = makeheap_empty(epsilon);
      softheap *Q = makeheap_empty(epsilon);

      for(int j = 0; j < n; j++) {
        insert(P, elts1[j]);
        insert(Q, elts2[j]);
      }

      clock_t start = clock();
      P = meld(P, Q);
      clock_t stop = clock();
      double meld_secs = (double)(stop-start) / CLOCKS_PER_SEC;

      cumul[r] += meld_secs;

      destroy_heap(P);
    }
  }

  for(int i = max_r; i > 5; i--) {
    printf("r=%d \t average meld: %f \n", i, cumul[i]/tries);
  }

}

int main(int argc, char *argv[]) {
  int n = 10000;
  int tries = 10;

  srand(time(NULL));

  time_insert_extract(tries, n);
  time_meld(tries, n);

  return 0;
}
