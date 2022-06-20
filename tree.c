// tree.c -- simple binary tree search/insert/update code
//   modeled after Kernighan & Ritchie, "C Programming Language", pg 130
// Accumulates basic statistics (mean, std dev, min, max) of a stream of
//   values, associated with a labeled node in a binary tree
// Data values are stored in a data structure associated with each
//   keyed node.
// hdtodd@gmail.com, 2022.05.22

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "stats.h"
#include "tree.h"

char MODULE[] = "tree -- simple binary tree model";
int node_number = 0;

//  create a new attribute data node 
APTR attr_new(void) {
  APTR p;
#ifdef DEBUG
  printf("entering attr_new\n");
#endif
  p = (APTR) malloc(sizeof(ATTR));
  if (p == NULL) {
    fprintf(stderr, "Really?  Systems says we're out of memory\n",
	   "  while allocating space for a new attribute node in 'tree'\n");
    exit(EXIT_FAILURE);
  };
  p->count = 0;
  p->mean = p->std2 = (double)0.0e0;
  p->min = (double)+INFINITY;
  p->max = (double)-INFINITY;
  return p;
};

//  create a new binary tree node and instantiate its associated attribute node
NPTR node_new(char *key) {
  NPTR p;
  node_number++;
#ifdef DEBUG
  printf("entering node_new with key '%s' to create node # %d\n", key, node_number);
#endif
  p = (NPTR) malloc(sizeof(NODE));
  if (p == NULL) {
    fprintf(stderr, "Really?  Systems says we're out of memory\n",
	   "  while allocating space for a new binary tree  node in 'tree'\n");
    exit(EXIT_FAILURE);
  };
  p->key = (char *) malloc(strlen(key)+1);
  strcpy(p->key,key);
  p->attr = (APTR) attr_new();
  p->num = node_number;
  p->bh = EH;
  p->lptr = p->rptr = NULL;
#ifdef DEBUG
  printf("Node[%2d]: key=%s, balance=%d, lptr=0x%x, rptr=0x%x\n", p->num, p->key, p->bh, p->lptr, p->rptr);
  printf("\tattr address=%x, attr_count = %d\n", p->attr, (p->attr)->count);
#endif
  return p;
};

/*
void node_print(NPTR p) {
  if (p == NULL) printf("NULL pointer!\n");
  else 
    printf("\tNode [%3d] @ 0x%x:%20s, cnt = %d, lptr = 0x%x, rptr = 0x%x\n",
	   p->num, p, p->key, (p->attr)->count, p->lptr, p->rptr);
};
*/

// find the tree node assocated with 'key' or create a new node
//   at the appropriate place in the binary tree
NPTR node_find(NPTR root, char *key) {
  int cond;
  NPTR p = root;;
  if (p == NULL)   return(node_new(key));
  while (p != NULL) {
    if ( (cond=strcmp(key, p->key)) == 0)
      return(p);
    else if (cond<0)
      if (p->lptr == NULL) return (p->lptr = node_new(key));
      else p = p->lptr;
    else
      if (p->rptr == NULL) return (p->rptr = node_new(key));
      else p = p->rptr;
  };
};  

// Not implemented as node_find does this
NPTR node_insert(char *key) {
  printf("Spurious call to 'node_insert: should not have occurred\n");
  return NULL;
};

// In-order printing of the tree with the 'stats_print' function
void tree_print(NPTR p) {
  if (p != NULL) {
    tree_print(p->lptr);
#ifdef DEBUG
    printf("In tree_print, Node[%3d]: %-12s  %2d\n", p->num, p->key, (p->attr)->count);
#endif
    stats_get( (bstats *)p->attr);
    printf("%-30s ", p->key);
    stats_print( (bstats *)p->attr);
    tree_print(p->rptr);
  };
};

// In-order processing of the tree nodes using an external
//   (callback) routine for the actual node processing
void tree_process(NPTR p, void print_node(NPTR p)) {
  if (p != NULL) {
    tree_process(p->lptr, *print_node);
    print_node(p);
    tree_process(p->rptr, *print_node);
  };
};
