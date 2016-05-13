/* File: softheap.h
 * ----------------
 * Header for a soft heap, an approximate min-priority queue that
 * allows amortized O(1) creation, merging, and extract-min
 * in exchange for reduced extract-min accuracy. The heap
 * takes a parameter epsilon and guarantees that in any sequence
 * of operations containing n inserts, there are never more than
 * epsilon * n "corrupted elements" in the heap: elements traveling 
 * with priorities higher than the priorities with which they 
 * were inserted. For a given value of epsilon, insertion into the 
 * soft heap is amortized O(log_2 (1/epsilon)).
 */

#ifndef SOFTHEAP_H
#define SOFTHEAP_H

#include <stdbool.h>

/* Opaque type defining the soft heap data structure. */
typedef struct SOFTHEAP softheap;

/**
 * Function: makeheap
 * ------------------
 * Given an integer elem and an error parameter epsilon,
 * creates a soft heap with parameter epsilon containing
 * only elem.
 */
softheap *makeheap(int elem, double epsilon);

/**
 * Function: makeheap_empty
 * ------------------------
 * Returns an empty soft heap.
 */
softheap *makeheap_empty(double epsilon);

/**
 * Function: destroy_heap
 * ----------------------
 * Destroys this soft heap and deallocates all the memory
 * associated with it.
 */
void destroy_heap(softheap *P);

/**
 * Function: empty
 * ---------------
 * A boolean returning true if and only if the soft heap
 * pointed to by P is empty.
 */
bool empty(softheap *P);

/**
 * Function: insert
 * ----------------
 * Inserts the parameter element into the soft heap pointed
 * to by P.
 */
void insert(softheap *P, int elem);

/**
 * Function: meld
 * --------------
 * Destructively modifies soft heaps P and Q to merge their
 * contents into a new soft heap that is returned.
 */
softheap *meld(softheap *P, softheap *Q);

/**
 * Function: extract_min
 * ---------------------
 * Extracts an element from soft heap P. Owing to the gimmick
 * of the soft heap, this element is not guaranteed to be the
 * element whose original inserted priority is lowest out
 * of all elements currently in the heap. However, the element
 * returned will have the minimum "stored priority" among all
 * those that the heap is using to transport elements.
 * Intuitively, then, this element is "close" to the min-priority
 * element in the heap.
 */
int extract_min(softheap *P);

/**
 * Function: extract_min_with_ckey
 * -------------------------------
 * Extracts an element from soft heap P and stores the ckey of that
 * element in the integer pointed to by ckey_into. The ckey is the 
 * priority with which the returned element was traveling in the
 * heap at the time of its extraction, and is an upper bound on 
 * the true priority of the element.
 */
int extract_min_with_ckey(softheap *P, int *ckey_into);

#endif // SOFTHEAP_H
