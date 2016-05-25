#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <error.h>
#include <time.h>
#include <float.h>

#include "softheap.h"
#include "binheap.c"

// Defines a function type used to sort integer arrays.
typedef void (*sorter)(int *, size_t);

/* Check whether A is sorted in nondecreasing order. */
static bool sorted(int *A, size_t length) {
  for(int i = 0; i < length - 1; i++) {
    if(A[i] > A[i+1]) return false;
  }
  return true;
}

/******************************************** MERGESORT ****************************************/

/* Call mergesort recursively on two halves of the subarray A[l..r].
 * Then copy the two halves into auxiliary arrays and do a two-way
 * merge of those auxiliary arrays back into A[l..r], so that the 
 * subarray ends up sorted. */
static void mergesort(int *A, int *aux1, int *aux2, int l, int r) {
  if(l >= r) return;
  
  // Recursively sort two halves
  int q = (l+r)/2;
  mergesort(A, aux1, aux2, l, q);
  mergesort(A, aux1, aux2, q+1, r);
  
  // Copy the two sorted halves into aux arrays
  int nlo = q - l + 1, nhi = r - q;
  memcpy(aux1, A + l, nlo * sizeof(int));
  memcpy(aux2, A + q + 1, nhi * sizeof(int));

  // Merge the arrays by filling A[l..r] with the lower current elem
  // of the smallest two unconsidered elements in aux1 and aux2
  int i = 0, j = 0, pos = l;
  while(i < nlo && j < nhi) {
    if(aux1[i] <= aux2[j]) A[pos++] = aux1[i++];
    else A[pos++] = aux2[j++];
  }
  
  // fill the remainder of A[l..r] with whichever subarray isn't done
  while(i < nlo) A[pos++] = aux1[i++];
  while(j < nhi) A[pos++] = aux2[j++];
}


static void mergesort_wrapper(int *A, size_t length) {
  int aux1[(length+1)/2], aux2[(length+1)/2];
  mergesort(A, aux1, aux2, 0, length - 1);
}

/******************************************** HEAPSORT ****************************************/

/* Builds a max-heap out of A in time O(n). Then swaps the max element
 * to the end of the heap and heapifies at index 0 to pull the new max
 * element to the root. Repeats swapping progressively smaller elements
 * to the end of the active subarray until the entire array is sorted. */
static void heapsort(int *A, size_t length) {
  build_maxheap(A, length);
  for(size_t i = length - 1; i >= 1; i--) {
    swap(A, i, 0);
    max_heapify(A, i, 0);
  }
}

/******************************************** QUICKSORT ****************************************/

/* Hoare partitioning algorithm for quicksort. */
static int partition(int *A, int l, int r) {
  int i = l, j = r+1;
  int x = A[l];

  while(true) {
    do { j--; } while(A[j] > x);
    do { i++; } while(A[i] < x && i <= j);

    if(i < j) swap(A, i, j);
    else break;
  }

  // It can be shown that at this point, A[l] = x,
  // A[l+1..i-1] <= x, A[i..r] >= x. Swapping A[l] with A[i-1]
  // completes a partition with the pivot at i-1.
  swap(A, l, i-1); 
  return i-1;
}

/* Implementation of quicksort with Hoare partitioning
 * and randomized pivot selection. */
static void quicksort(int *A, int l, int r) {
  if(l >= r) return;
  int rand_pivot = l + (rand() % (r - l + 1));
  swap(A, l, rand_pivot);

  int q = partition(A, l, r);
  quicksort(A, l, q-1);
  quicksort(A, q+1, r);
}

/* Calls the quicksort subroutine on a subarray consisting of
 * all of A. */
static inline void quicksort_wrapper(int *A, size_t length) {
  quicksort(A, 0, length - 1);
}

/******************************************** GNU QSORT ****************************************/

/* Callback function to compare the values of two integers ala strcmp. */
static int intcmp(const void *a, const void *b) {
  return *(int *)a - *(int *)b;
}

/* Calls the GNU qsort routine on an array A with length elements. */
static inline void gnu_qsort_wrapper(int *A, size_t length) {
  qsort(A, length, sizeof(int), intcmp);
}

/***************************************** SOFTHEAP SORT *************************************/

/* Sorts the input array using a soft heap with epsilon given by
 * a value just under 1/n. This makes it impossible for the soft
 * heap to contain any corrupted elements, guaranteeing that a
 * sequence of extract-mins will pull the elements out of the soft 
 * heap in sorted order. */
static void softheap_sort(int *A, size_t length) {
  if(length == 1) return; // already sorted
  double epsilon = (double)1/length;
  if(epsilon <= 0) epsilon = DBL_MIN; // guarantee positive epsilon

  softheap *sh = makeheap_empty(epsilon);
  for(int i = 0; i < length; i++) insert(sh, A[i]);
  for(int i = 0; i < length; i++) A[i] = extract_min(sh);
  destroy_heap(sh);
}

/***************************************** RADIX SORT ****************************************/

/* Do a counting sort over the curr_dig-th least significant digit of the elements
 * of input (in base b), and put the contents into output. */
static void radix_pass(int *input, int *output, size_t length, int b, int divisor) {
  int counter[b];
  memset(counter, 0, sizeof(int) * b); // set digit counts to 0
   
  // Count the number of representatives there are of each digit across all values
  for(int i = 0; i < length; i++) {
    int dig_i = (input[i] / divisor) % b; 
    counter[dig_i]++;
  }

  // Change counter so that counter[i] counts how many elements have appropriate LSD
  // less than or equal to i. This is 1 plus the max index at which an element 
  // with that digit should appear in a zero-indexed array. 
  for(int i = 1; i < b; i++) {
    counter[i] += counter[i-1];
  }
  
  for(int i = length - 1; i >= 0; i--) {
    int elem = input[i];
    int dig_i = (elem / divisor) % b;
    output[counter[dig_i] - 1] = elem;
    counter[dig_i]--; // so that next elem with this digit can be placed before this one
  }
}

/* Perform a base-10 radix sort on the elements of A. */
static void radix_sort(int *A, size_t length) {
  int b = 10; // radix sort base
  int ndigits = ceil(log(RAND_MAX)/log(b));
  int B[length];
  memset(B, 0, sizeof(int) * length); // Valgrind gets pissed if I don't do this

  int divisor = 1; // (x / divisor) % (b) is the log_b(divisor)th least sigdig of x

  for(int i = 0; i < ndigits; i++) {
    // On even-numbered runs, current progress moves A->B; on even, B->A
    if(i % 2 == 0) radix_pass(A, B, length, b, divisor);
    else radix_pass(B, A, length, b, divisor);
    divisor *= b;
  }

  // After the final pass, the sorted array will be B if ndigits was odd.
  // If this is the case, copy B into A to complete the sort.
  if(ndigits % 2 == 1) memcpy(A, B, length * sizeof(int));
}

/******************************************** TIMING ****************************************/

/* Call the sorting algorithm of choice on a copy on the original array of
 * random elements. Report timing results. */
static void time_sort(int *A, size_t length, sorter sort, char *sort_name) {
  //  printf("----------------- Timing %s ------------------\n", sort_name);
  //  printf("Making copy of original array...\n");
  int B[length];
  memcpy(B, A, length * sizeof(int));

  // printf("Timing %s on array...\n", sort_name);

  clock_t tick = clock();
  sort(B, length);
  clock_t tock = clock();
  double elapsed_secs = (double)(tock - tick) / CLOCKS_PER_SEC;
  
  if(!sorted(B, length)) error(1,0, "%s failed", sort_name);
  // printf("Success.\n");
  printf("%s\t %4.6f \n", sort_name, elapsed_secs);  
}


static inline void time_heapsort(int *A, size_t length) {
  time_sort(A, length, heapsort, "heapsort");
}

static inline void time_quicksort(int *A, size_t length) {
  time_sort(A, length, quicksort_wrapper, "quicksort");
}

static inline void time_softheap_sort(int *A, size_t length) {
  time_sort(A, length, softheap_sort, "softheap sort");
}

static inline void time_gnu_qsort(int *A, size_t length) {
  time_sort(A, length, gnu_qsort_wrapper, "GNU qsort");
}

static inline void time_mergesort(int *A, size_t length) {
  time_sort(A, length, mergesort_wrapper, "mergesort");
}

static inline void time_radix_sort(int *A, size_t length) {
  time_sort(A, length, radix_sort, "radix sort");
}



int main(int argc, char *argv[]) {
  if(argc != 2) error(1,0, "usage: ./sorts [nelems]");
  int nelems = atoi(argv[1]);
  if(nelems <= 0) error(1,0, "nelems must be a valid integer greater than or equal to 1");

  long long seed = time(NULL);
  srand(seed);
  //  printf("Random seed: %lld\n", seed);

  //  printf("Creating a template array of %d random elements...\n", nelems);
  int A[nelems];
  for(int i = 0; i < nelems; i++) A[i] = rand();
  //  printf("Done. Starting tests...\n\n");

  time_mergesort(A, nelems);
  time_heapsort(A, nelems);
  time_quicksort(A, nelems);
  time_gnu_qsort(A, nelems);
  time_radix_sort(A, nelems);
  time_softheap_sort(A, nelems);

  return 0;
}
