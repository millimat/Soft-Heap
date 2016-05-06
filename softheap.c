/* File: softheap.c
 * ----------------
 * Implementation of a soft heap. Rather than the version using binomial trees
 * introduced in Chazelle's original paper, this soft heap uses binary trees
 * according to the strategy outlined in Kaplan/Zwick.
 */

#include "softheap.h"

#include <stdlib.h>
#include <assert.h> // for assert
#include <error.h> // for error
#include <math.h> // For log and ceil. Remember to link math library!

/* Structure representing a soft heap. The soft heap object has access
 * to the first tree in its root list, the rank of the highest-order tree
 * in its root list, its error parameter epsilon, and the parameter
 * r(epsilon) that defines the maximum node rank for which a node 
 * is guaranteed to contain only uncorrupted elements. */
typedef struct SOFTHEAP {
  struct TREE *first;
  int rank;
  double epsilon;
  int r;
} softheap;

/* Structure representing a binary tree in a soft heap's rootlist. The tree stores
 * its rank, which is the maximum possible height of its root (although the
 * root is not guaranteed to have that height at all times). The tree
 * is wired to its predecessor and successor in the rootlist, which have
 * rank less than and greater than this tree's rank, respectively. 
 * The tree also has a pointer to its own root.
 * 
 * Binary trees in a soft-heap are heap-ordered according to the "ckeys" of the nodes
 * in the trees. Each node stores a list of items under one ckey; the ckey is 
 * an upper bound on the original priorities of all items in the node's list.
 * The final element of a softheap tree is a pointer "sufmin" to the tree of minimum
 * root ckey in the segment of the rootlist beginning at this tree.
 */
typedef struct TREE {
  struct TREE *prev, *next, *sufmin;
  struct TREENODE *root;
  int rank;
} tree;

/* A node in a tree in a soft heap. The node has access to its left and right children,
 * but does not need access to its parent. It contains a ckey (its priority), its rank,
 * the number of elements in its list, and its "size": a parameter defined such that
 * its list always contains Theta(size) elements so long as the node is not a leaf. 
 * Its list is stored as a doubly linked list. */
typedef struct TREENODE {
  struct TREENODE *left, *right;
  struct LISTCELL *first, *last;
  int ckey, rank, size, nelems;
} node;

/* An item in a soft heap tree node's list. */
typedef struct LISTCELL {
  int elem;
  struct LISTCELL *prev, *next;
} cell;

/************************************** HEAP & ITEM CREATION ***********************************/

/* Function: makeheap
 * ------------------
 * Construct a soft heap with error parameter epsilon containing element elem.
 * This is done by constructing a tree of rank 0 containing a single rank-0
 * node. The node has one item in its item list, which is the item inserted.
 */
softheap *makeheap(int elem, double epsilon) {
  softheap *s = makeheap_empty(epsilon);
  s->first = maketree(elem);
  s->rank = 0;
  return s;
}

/* Function: makeheap_empty
 * ------------------------
 * Constructs an empty soft heap with the provided error parameter.
 */
softheap *makeheap_empty(double epsilon) {
  // Ensure error parameter is valid
  if(epsilon <= 0 || epsilon >= 1) error(1,0, "Soft heap error parameter must fall in (0,1)");
  
  softheap *s = malloc(sizeof(softheap));
  s->first = NULL;
  s->rank = -1; // Ensures that any insertion will just return the SH containing the inserted elem
  s->epsilon = epsilon;
  s->r = get_r(epsilon);
  return s;
}

/* Function: maketree
 * ------------------
 * Constructs a soft heap binary tree consisting of exactly one node
 * housing the parameter element.
 */
static tree *maketree(int elem) {
  tree *T = malloc(sizeof(tree));
  T->root = makenode(elem);
  T->prev = T->next = NULL;
  T->rank = 0;
  T->sufmin = T;
  return T;
}

/* Function: makenode
 * ------------------
 * Constructs a rank-0 soft heap binary tree node containing just the parameter
 * element. Its ckey matches the element, since that element is the only
 * object in its list.
 */
static node *makenode(int elem) {
  node *x = malloc(sizeof(node));
  x->first = x->last = addcell(elem, NULL);
  x->ckey = elem;
  x->rank = 0;
  x->size = x->nelems = 1;
  x->left = x->right = NULL;
  return x;
}


/* Function: addcell
 * -----------------
 * Creates a list cell containing the parameter element
 * and concatenates it to the end of the linked list pointed
 * to by listend.
 */
static cell *addcell(int elem, cell *listend) {
  cell *c = malloc(sizeof(cell));
  c->elem = elem;
  c->prev = listend;
  if(listend != NULL) listend->next = c;
  c->next = NULL;
  return c;
}

 
/************************************** UTILITY FUNCTIONS ***********************************/

/* Function: leaf
 * --------------
 * Return true if and only if this soft heap tree node
 * has no children. 
 */
static inline bool leaf(node *x) {
  return (x->left == NULL && x->right == NULL);
}

/* Function: swapLR 
 * ----------------
 * Swap the left and right children of this node.
 */
static inline void swapLR(node *x) {
  node *tmp = x->left;
  x->left = x->right;
  x->right = tmp;
}

/* Function: get_r
 * ---------------
 * Return the parameter r(epsilon) for this soft heap.
 * r is the largest integer such that a node of that rank
 * contains only uncorrupted elements.
 */
static inline int get_r(double epsilon) {
  return ceil(log(epsilon)/log(2)) + 5;
}

bool empty(softheap *P) {
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


softheap *meld(softheap *P, softheap *Q) {
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


// make void by setting *P = meld? 
softheap *insert(softheap *P, int elem) {
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

int extract_min(softheap *P) {
  int filler;
  return extract_min_with_ckey(P, &filler);
}

int extract_min_with_ckey(softheap *P, int *ckey_into) {
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

int main() {}
