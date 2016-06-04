#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "softheap.h"

/* --------------- Metrics ------------------ */

double metric_mispositions(int *output, int n) {
  int mispos = 0;
  for(int i = 0; i < n; i++) {
    if(output[i] != i)
      mispos++;
  }

  return mispos/(double)n;
}

long metric_distance(int *output, int n) {
  long dist = 0;
  for(int i = 0; i < n; i++) {
    dist += (output[i] > i ? output[i] - i : i - output[i]);
  }

  return dist;
}

//Kendall tau distance. Equivalent to number of pairs in wrong order.
double metric_kendall(int *output, int n) {
  long count = 0;
  for(int i = 0; i < n; i++) {
    for(int j = i+1; j < n; j++) {
      if(output[i] > output[j])
        count++;
    }
  }

  return 2*count / (n*(n-1.0));
}

double metric_mispositions_threshold(int *output, int n, int threshold) {
  int mispos = 0;
  for(int i = 0; i < n; i++) {
    if(output[i] > i + threshold || output[i] < i - threshold)
      mispos++;
  }

  return mispos/(double)n;
}


/* --------------- Test -------------------- */

void all_metrics_per_epsilon(int *elts, int n) {
  int output[n];

  for(int k = 1; k < n; k *= 2) {
    double epsilon = ((double)k)/n;
    int r = ceil(-log(epsilon)/log(2)) + 5;

    softheap *P = makeheap_empty(epsilon);

    for(int i = 0; i < n; i++)
      insert(P, elts[i]);

    for(int i = 0; i < n; i++)
      output[i] = extract_min(P);

    destroy_heap(P);

    printf("r=%d \t\t %f \t\t %ld \t\t %f \n", r, metric_mispositions(output,n),metric_distance(output,n),metric_mispositions_threshold(output,n,n/100));
  }
}

//Uniform in [0,k). Not perfect, but should be ok...
int randint(int k) {
  return (int)(rand()/(RAND_MAX + 1.0) * k);
}

//Fisher-Yates uniformly random permutation
void random_permutation(int *elts, int n) {
  for(int i = 0; i < n; i++) {
    int j = randint(i+1);
    elts[i] = elts[j];
    elts[j] = i;
  }
}


int main(int argc, char *argv[]) {
  int n = 1000000;

  int elts[n];

  srand(time(NULL));
  random_permutation(elts, n);

  all_metrics_per_epsilon(elts, n);

  return 0;
}
