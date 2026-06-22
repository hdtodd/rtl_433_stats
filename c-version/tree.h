// tree.h
// Include file with definitions used by binary tree code
//   for processing data into per-device records
//   and with structures to contain the data for that device

#include <time.h>

// The ATTR structure holds the data associated with each device
typedef struct attr {
  int           pktcount;        // # of packets for this device
  int           xmtcount;        // # of transmissions for this device
  int           pkt_xmt;         // # of packets for this transmission
  time_t        last_pkt_time;   // Unix Epoch time for last packet
  time_t        last_xmt_time;   // Unix Epoch time for last transmission
  BSPTR         snr;             // Stats for signal-to-noise for this device
  BSPTR         freq;            // Stats for frequency for this device
  BSPTR         itgt;            // Stats for Inter-Transmit Gap Time
  BSPTR         ppt;             // Stats for Packet Per Transmission
} ATTR, *APTR;

// The NODE structure holds the name of each device,
//   a pointer to its data attributes, a unique number for the
//   node (for debugging), and the tree pointer for processing
typedef struct node {
  char           *key;
  APTR           attr;
  int            num;
  struct node   *lptr;
  struct node   *rptr;
} NODE, *NPTR;

// Find/create a tree node for device named by "key"
NPTR node_find(NPTR p, char *key);
// Dump the tree
void node_dump(void);
// Print the tree structure
void tree_print(NPTR p);
// Process the tree via in-order traversal
//   and print the data associated with each node
void tree_process(NPTR p, void node_print());
