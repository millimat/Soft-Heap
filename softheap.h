/* File: softheap.h
 * ----------------
 * Header for a soft heap, an approximate min-priority queue that
 * allows amortized O(1) creation, merging, and extract-min
 * in exchange for reduced extract-min accuracy. The heap
 * takes a parameter epsilon and guarantees that in any sequence
 * of operations containing n inserts, there are never more than
 * (epsilon * n) elements in the heap traveling with priorities
 * higher than the priorities with which they are inserted
 * ("corrupted elements"). For a given value of epsilon,
 * insertion into the soft heap is amortized O(log_2 (1/epsilon)).
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
 * Function: empty
 * ---------------
 * A boolean returning true if and only if the soft heap
 * pointed to by P is empty.
 */
bool empty(softheap *P);

softheap *insert(softheap *P, int elem);
softheap *meld(softheap *P, softheap *Q);
int extract_min(softheap *P);
int extract_min_with_ckey(softheap *P, int *ckey_into);

#endif // SOFTHEAP_H
