#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <error.h>
#include <stdbool.h>
#include <math.h> // Remember to link math library!
#include <time.h> // to set rand

typedef struct LISTCELL {
  int elem;
  struct LISTCELL *prev, *next;
} cell;

typedef struct TREENODE {
  struct TREENODE *left, *right;
  struct LISTCELL *first, *last;
  int ckey, rank, size, nelems;
} node;

typedef struct TREE {
  struct TREE *prev, *next, *sufmin;
  struct TREENODE *root;
  int rank;
} tree;

typedef struct SOFTHEAP {
  struct TREE *first;
  int rank;
  double epsilon;
  int r;
} softheap;

static inline bool leaf(node *x) {
  return (x->left == NULL && x->right == NULL);
}

static inline void swapLR(node *x) {
  node *tmp = x->left;
  x->left = x->right;
  x->right = tmp;
}

static inline bool empty(softheap *P) {
  return P->first == NULL;
}

static void moveList(node *src, node *dst) {
  assert(src->first != NULL);
  if(dst->last != NULL) dst->last->next = src->first;
  if(dst->first == NULL) dst->first = src->first;
  src->first->prev = dst->last;
  dst->last = src->last;
    
  dst->nelems += src->nelems;
  src->nelems = 0;
  src->first = src->last = NULL;  
}

// make new list cell and concat to end of list
static cell *addcell(int elem, cell *listend) {
  cell *c = malloc(sizeof(cell));
  c->elem = elem;
  c->prev = listend;
  if(listend != NULL) listend->next = c;
  c->next = NULL;
  return c;
}

static node *makenode(int elem) {
  node *x = malloc(sizeof(node));
  x->first = x->last = addcell(elem, NULL);
  x->ckey = elem;
  x->rank = 0;
  x->size = x->nelems = 1;
  x->left = x->right = NULL;
  return x;
}

static tree *maketree(int elem) {
  tree *T = malloc(sizeof(tree));
  T->root = makenode(elem);
  T->prev = T->next = NULL;
  T->rank = 0;
  T->sufmin = T;
  return T;
}

static softheap *makeheap(int elem, double epsilon) {
  softheap *s = malloc(sizeof(softheap));
  s->first = maketree(elem);
  s->rank = 0;
  s->epsilon = epsilon;
  s->r = ceil(log(epsilon)/log(2)) + 5; // max guaranteed uncorrupted rank
  return s;
}

static void sift(node *x) {
  while(x->nelems < x->size && !leaf(x)) {
    if(x->left == NULL || (x->right != NULL && x->left->ckey > x->right->ckey)) swapLR(x);
    moveList(x->left, x); // concat left's list to x's to replenish x
    x->ckey = x->left->ckey;

    // if left was a leaf, it can't be repaired, so destroy it
    if(leaf(x->left)) {
      free(x->left);
      x->left = NULL;
    } else {
      sift(x->left); 
    }   
  }
}

static node *combine(node *x, node *y, int r) {
  node *z = malloc(sizeof(node));
  z->left = x;
  z->right = y;
  z->rank = x->rank + 1;
  z->nelems = 0;
  z->first = z->last = NULL;

  z->size = (z->rank <= r ? 1 : (3 * x->size + 1)/2);
  sift(z);
  return z;
}

static void update_suffix_min(tree *T) {
  while(T != NULL) {
    if(T->next == NULL || T->root->ckey <= T->next->sufmin->root->ckey) T->sufmin = T;
    else T->sufmin = T->next->sufmin;
    T = T->prev;
  }
}

static void insert_tree(softheap *into_heap, tree *inserted, tree *successor) {
  inserted->next = successor;

  // make currP the first tree in Q if currQ was the first, else link Q's prev to currP
  if(successor->prev == NULL) into_heap->first = inserted;
  else successor->prev->next = inserted;
  inserted->prev = successor->prev;
  successor->prev = inserted;
}

static void remove_tree(softheap *outof_heap, tree *removed) {
  if(removed->prev == NULL) outof_heap->first = removed->next;
  else removed->prev->next = removed->next;
  if(removed->next != NULL) removed->next->prev = removed->prev;
}

// Put the trees of soft heap P into soft heap Q. Q->rank >= P->rank.
// On termination, Q will contain all trees from P and Q, in increasing rank order.
static void merge_into(softheap *P, softheap *Q) {
  

  tree *currP = P->first, *currQ = Q->first;
  
  while(currP != NULL) {
    while(currQ->rank < currP->rank) currQ = currQ->next;
    // currQ is now the first tree in Q with rank >= currP. Insert currP before it.
    tree *next = currP->next;
    insert_tree(Q, currP, currQ);
    currP = next;
  }
}

static void repeated_combine(softheap *Q, int smaller_rank, int r) {
  tree *curr = Q->first;

  while(curr->next != NULL) {
    bool two = (curr->rank == curr->next->rank);
    bool three = (two && curr->next->next != NULL && curr->rank == curr->next->next->rank);

    if(!two) { // only one tree of this rank
      if(curr->rank > smaller_rank) break; // no more combines to do and no carries
      else curr = curr->next;
    } else if(!three) { // exactly two trees of this rank
      // combine them to make a carry, then delete curr->next. Do not advance curr yet.
      curr->root = combine(curr->root, curr->next->root, r);
      curr->rank = curr->root->rank;
      tree *tofree = curr->next;
      remove_tree(Q, curr->next); // will change what curr->next points to
      free(tofree);
    } else { // exactly three trees of this rank
      // skip the first so that we can combine the second and third
      curr = curr->next;
    }
  }

  if(curr->rank > Q->rank) Q->rank = curr->rank;
  update_suffix_min(curr); // update minima for curr and all trees before it    
}


static softheap *meld(softheap *P, softheap *Q) {
  if(P->epsilon - Q->epsilon > 0.001) {
    error(1,0, "Tried to combine softheaps with different epsilons");
  }
  
  softheap *result;
  if(P->rank > Q->rank) { // meld Q into P
    merge_into(Q, P);
    repeated_combine(P, Q->rank, P->r);
    free(Q);
    result = P;
  } else { // meld P into Q
    merge_into(P, Q);
    repeated_combine(Q, P->rank, Q->r);
    free(P);
    result = Q;
  }

  return result;
}

static softheap *insert(softheap *P, int elem) {
  return meld(P, makeheap(elem, P->epsilon));
}

// Remove the first element from x's list.
static int extract_elem(node *x) {
  assert(x->first != NULL);
  cell *todelete = x->first;
  int result = todelete->elem;
  
  x->first = todelete->next;
  if(x->first != NULL) {
    if(x->first->next == NULL) x->last = x->first;
    x->first->prev = NULL;
  } else x->last = NULL;

  free(todelete);
  x->nelems--;
  return result;
}

static int extract_min_with_ckey(softheap *P, int *ckey_into) {
  if(empty(P)) error(1,0, "Tried to extract an element from an empty soft heap");
  
  tree *T = P->first->sufmin; // tree with lowest root ckey
  node *x = T->root;
  int e = extract_elem(x);
  *ckey_into = x->ckey;

  if(x->nelems <= x->size / 2) { // x is deficient; rescue it if possible
    if(!leaf(x)) {
      sift(x);
      update_suffix_min(T);
    } else if(x->nelems == 0) { // x is a leaf and empty; it must be destroyed
      free(x);
      remove_tree(P, T);
      if(T->next == NULL) { // we removed the highest-ranked tree; reset rank and clean up
        if(T->prev == NULL) P->rank = -1; // Heap now empty. Rank -1 is sentinel for future melds
        else P->rank = T->prev->rank;
      }
      if(T->prev != NULL) update_suffix_min(T->prev);
      free(T);
    }
  }

  return e;
}

#define N_ELEMENTS 1000000
#define EPSILON 0.125
#define MAGIC_PRIME_ONE 1399
#define MAGIC_PRIME_TWO 1093


static int intcmp(const void *one, const void *two) {
  return *(int *)one - *(int *)two;
}

int main() {
  int sorted[N_ELEMENTS];
  int results[N_ELEMENTS][2];
  softheap *P = makeheap(0, EPSILON);
  
  sorted[0] = 0;
  printf("---------- COPRIME TESTS ----------\n\n");
  for(int i = 1; i < N_ELEMENTS; i++) {
    int num = MAGIC_PRIME_ONE * i % MAGIC_PRIME_TWO;
    sorted[i] = num;
    P = insert(P, num);
  }

  qsort(sorted, N_ELEMENTS, sizeof(int), intcmp);

  int ckey_corruptions = 0, pos_corruptions = 0;
  for(int i = 0; i < N_ELEMENTS; i++) {
    results[i][0] = extract_min_with_ckey(P, &results[i][1]);
    if(results[i][0] != results[i][1]) ckey_corruptions++;
    if(results[i][0] != sorted[i]) pos_corruptions++;
  }

  printf("Results for inserting integers (%d * i %% %d) for i = 0 to to %d:\n", 
         MAGIC_PRIME_ONE, MAGIC_PRIME_TWO, N_ELEMENTS - 1);
  /* for(int i = 0; i < N_ELEMENTS; i++) { */
  /*   printf("Rank %2d:\t sorted elem = %2d, extracted elem = %2d, ckey = %2d\n", i,  */
  /*          sorted[i], results[i][0], results[i][1]);  */
  /* } */

  printf("\nTotal number of ckey corruptions: %d\nFraction corrupted: %4.3f\n", ckey_corruptions, 
         (double)ckey_corruptions/N_ELEMENTS);
  printf("\nTotal number of positional corruptions: %d\nFraction corrupted: %4.3f\n", pos_corruptions, 
         (double)pos_corruptions/N_ELEMENTS);

  printf("\n---------- RANDOM TESTS ----------\n\n");
  long seed = 1462470053; //= time(NULL);
  srand(seed);
  printf("Random seed: %ld\n", seed);
  ckey_corruptions = pos_corruptions = 0;
  for(int i = 0; i < N_ELEMENTS; i++) {
    int num = rand();
    sorted[i] = num;
    P = insert(P, num);
  }

  for(int i = 0; i < N_ELEMENTS; i++) {
    results[i][0] = extract_min_with_ckey(P, &results[i][1]);
    if(results[i][0] != results[i][1]) ckey_corruptions++;
    if(results[i][0] != sorted[i]) pos_corruptions++;
  }

  /* printf("Results for inserting %d random integers:\n", N_ELEMENTS); */
  /* for(int i = 0; i < N_ELEMENTS; i++) { */
  /*   printf("Rank %2d:\t sorted elem = %2d, extracted elem = %2d, ckey = %2d\n", i,  */
  /*          sorted[i], results[i][0], results[i][1]);  */
  /* } */

  printf("\nTotal number of ckey corruptions: %d\nFraction corrupted: %4.3f\n", ckey_corruptions, 
         (double)ckey_corruptions/N_ELEMENTS);
  printf("\nTotal number of positional corruptions: %d\nFraction corrupted: %4.3f\n", pos_corruptions, 
         (double)pos_corruptions/N_ELEMENTS);


  
  free(P);
  return 0;
}
