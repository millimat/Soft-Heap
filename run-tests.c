#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "softheap.h"


#define N_ELEMENTS (1 << 15)
#define SORTED_EPSILON ((double)1 / N_ELEMENTS)
#define EPSILON 0.1
#define MAGIC_PRIME_ONE 1399
#define MAGIC_PRIME_TWO 1093

static inline int intcmp(const void *one, const void *two) {
  return *(int *)one - *(int *)two;
}

static void report_corruptions(int ckey_corruptions, int pos_corruptions, int nelems) {
  printf("\nTotal number of ckey corruptions: %d\nFraction corrupted: %4.3f\n", ckey_corruptions,
         (double)ckey_corruptions/nelems);
  printf("\nTotal number of positional corruptions: %d\nFraction corrupted: %4.3f\n", pos_corruptions,
         (double)pos_corruptions/nelems);
  printf("\n");
}

static void straight_test(int elems[], int results[][2]) {
  printf("----------STRAIGHT TEST----------\n");
  printf("Inserting integers 0 to %d into a soft heap...\n", N_ELEMENTS - 1);
  softheap *P = makeheap_empty(SORTED_EPSILON);
  for(int i = 0; i < N_ELEMENTS; i++) {
    elems[i] = i;
    insert(P, i);
  }
  
  int ckey_corruptions = 0, pos_corruptions = 0;
  printf("Extracting elements with ckeys...\n");
  for(int i = 0; i < N_ELEMENTS; i++) {
    if(results[i][0] < results[i][1]) ckey_corruptions++;
    if(results[i][0] != elems[i]) pos_corruptions++;
  }

  report_corruptions(ckey_corruptions, pos_corruptions, N_ELEMENTS);  
  free(P);
}

static void backwards_test(int elems[], int results[][2]) {
  printf("----------BACKWARDS TEST----------\n");
  printf("Inserting integers 0 to %d into a soft heap in reverse order...\n", N_ELEMENTS - 1);
  softheap *P = makeheap_empty(EPSILON);
  for(int i = N_ELEMENTS - 1; i >= 0; i--) {
    elems[i] = i;
    insert(P, i);
  }
  
  int ckey_corruptions = 0, pos_corruptions = 0;
  printf("Extracting elements with ckeys...\n");
  for(int i = 0; i < N_ELEMENTS; i++) {
    results[i][0] = extract_min_with_ckey(P, &results[i][1]);
    if(results[i][0] < results[i][1]) ckey_corruptions++;
    if(results[i][0] != elems[i]) pos_corruptions++;
  }
  
  report_corruptions(ckey_corruptions, pos_corruptions, N_ELEMENTS);  
  free(P);
}

static void coprime_test(int elems[], int results[][2]) {
  printf("----------COPRIME TEST----------\n");
  printf("Inserting integers (%d * i) mod %d into a soft heap for i = 0 to %d...\n",
         MAGIC_PRIME_ONE, MAGIC_PRIME_TWO, N_ELEMENTS - 1);

  softheap *P = makeheap_empty(EPSILON);
  for(int i = 0; i < N_ELEMENTS; i++) {
    int num = (MAGIC_PRIME_ONE * i) % MAGIC_PRIME_TWO;
    elems[i] = num;
    insert(P, num);
  }

  printf("Sorting correctness array...\n");
  qsort(elems, N_ELEMENTS, sizeof(int), intcmp);
  
  
  int ckey_corruptions = 0, pos_corruptions = 0;
  printf("Extracting elements with ckeys...\n");
  for(int i = 0; i < N_ELEMENTS; i++) {
    results[i][0] = extract_min_with_ckey(P, &results[i][1]);
    if(results[i][0] < results[i][1]) ckey_corruptions++;
    if(results[i][0] != elems[i]) pos_corruptions++;
  }
  
  report_corruptions(ckey_corruptions, pos_corruptions, N_ELEMENTS);  
  free(P);
}

static void random_test(int elems[], int results[][2]) {
  srand(time(NULL));
  printf("----------RANDOM TEST----------\n");
  printf("Inserting %d random integers into a soft heap...\n", N_ELEMENTS);        

  softheap *P = makeheap_empty(EPSILON);
  for(int i = 0; i < N_ELEMENTS; i++) {
    int num = rand() % N_ELEMENTS;
    elems[i] = num;
    insert(P, num);
  }

  printf("Sorting correctness array...\n");
  qsort(elems, N_ELEMENTS, sizeof(int), intcmp);
    
  int ckey_corruptions = 0, pos_corruptions = 0;
  printf("Extracting elements with ckeys...\n");
  for(int i = 0; i < N_ELEMENTS; i++) {
    results[i][0] = extract_min_with_ckey(P, &results[i][1]);
    if(results[i][0] < results[i][1]) ckey_corruptions++;
    if(results[i][0] != elems[i]) pos_corruptions++;
  }
  
  report_corruptions(ckey_corruptions, pos_corruptions, N_ELEMENTS);  
  free(P);
}

static void cleanup_test() {
  printf("----------CLEANUP TEST-----------\n");
  printf("Testing robustness of destroy_heap by creating and destroying "
         "100 soft heaps of increasing size...\n");

  srand(time(NULL));

  for(int i = 0; i < 100; i++) {
    int size = N_ELEMENTS / 100 * i;
    softheap *P = makeheap_empty(SORTED_EPSILON); // make them super branchy
    
    for(int j = 0; j < size; j++) insert(P, rand());

    destroy_heap(P);
    
    // Show progress
    printf(".");
    fflush(stdout);
    if(i % 10 == 9) printf("\n");
  }

  printf("Done.\n\n");
}

int main() {
  int sorted[N_ELEMENTS];
  int results[N_ELEMENTS][2];
  
  straight_test(sorted, results);
  backwards_test(sorted, results);
  coprime_test(sorted, results);
  random_test(sorted, results);
  cleanup_test();
  return 0;
}
