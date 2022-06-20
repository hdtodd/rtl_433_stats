// tree.h
// Include file with definitions used by binary tree code

// Define balance factor for subtree:
//   LH==>left high; EH==>equal height; RH==>right high
typedef enum {LH=-1, EH=0, RH =+1} BALANCEFACTOR;

typedef struct attr {
  int            count;
  double         mean;
  double         std2;
  double         min;
  double         max;
} ATTR, *APTR;

typedef struct node {
  char           *key;
  APTR           attr;
  int            num;
  BALANCEFACTOR  bh;
  struct node   *lptr;
  struct node   *rptr;
} NODE, *NPTR;

NPTR node_find(NPTR p, char *key);
void node_dump(void);
void tree_print(NPTR p);
//void node_print(NPTR p);
void tree_process(NPTR p, void node_print());
