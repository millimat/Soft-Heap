/* File: binheap.c
 * ---------------
 * Header file for an array-based binary heap of integers.
 * The heap can be set either as a max-heap or a min-heap
 * according to user input.
 */

#ifndef BINHEAP_C
#define BINHEAP_C

#include <stddef.h>
#include <stdbool.h>

/* Return the index of the parent of position i in the heap. */
size_t parent(size_t i) {
  return (i-1)/2;
}

/* Return the index of the left child of i in the heap. */
size_t left(size_t i) {
  return 2*i + 1;
}

/* Return the index of the right child of i in the heap. */
size_t right(size_t i) {
  return 2*i + 2;
}

/* Swap the elements at positions i and j in the heap. */
void swap(int *A, size_t i, size_t j) {
  int tmp = A[i];
  A[i] = A[j];
  A[j] = tmp;
}

/* Performs a max-heapify operation on A[i]. A[i] is swapped with its 
 * larger child if it is less than one of them, restoring the max-heap 
 * property at index i. It is then bubbled down as necessary until it is 
 * greater than or equal to all its children. */
void max_heapify(int *A, size_t heapsize, size_t i) {
  while(true) {
    size_t l = left(i), r = right(i), largest = i;
    // Detect the index of the largest valid element in A between l, r, i
    if(l < heapsize && A[l] > A[i]) largest = l;
    if(r < heapsize && A[r] > A[largest]) largest = r;

    // If i is the largest already, the heap property has been restored; terminate
    if(largest == i) break; 
    
    // Else swap i down and continue
    swap(A, i, largest);
    i = largest;
  }
}

/* Performs a min-heapify operation. The code is symmetric to max-heapify.*/
void min_heapify(int *A, size_t heapsize, int i) {
  while(true) {
    size_t l = left(i), r = right(i), smallest = i;
    // Detect the index of the largest valid element in A between l, r, i
    if(l < heapsize && A[l] < A[i]) smallest = l;
    if(r < heapsize && A[r] < A[smallest]) smallest = r;

    // If i is the largest already, the heap property has been restored; terminate
    if(smallest == i) break; 
    
    // Else swap i down and continue
    swap(A, i, smallest);
    i = smallest;
  }
}

/* Convert A to a maxheap by calling max_heapify on all nonleaf indices. */
void build_maxheap(int *A, size_t length) {
  for(int i = length/2 - 1; i >= 0; i--) max_heapify(A, length, i);
}

/* Convert a to a minheap. */
void build_minheap(int *A, size_t length) {
  for(int i = length/2 - 1; i >= 0; i--) min_heapify(A, length, i);
}









#endif // BINHEAP_C
