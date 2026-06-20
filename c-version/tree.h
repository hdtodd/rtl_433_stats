// tree.h
// Include file with definitions used by binary tree code
//   for processing data into per-device records
//   and with structures to contain the data for that device

typedef struct attr {
  int           pktcount;
  int           xmitcount;
  BSPTR         snr;
  BSPTR         itgt;
  BSPTR         freq;
  BSPTR         ppt;
} ATTR, *APTR;

typedef struct node {
  char           *key;
  APTR           attr;
  int            num;
  struct node   *lptr;
  struct node   *rptr;
} NODE, *NPTR;

NPTR node_find(NPTR p, char *key);
void node_dump(void);
void tree_print(NPTR p);
//void node_print(NPTR p);
void tree_process(NPTR p, void node_print());
