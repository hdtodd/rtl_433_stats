/* snr.c-- collect basic statistics on signal-to-noise ratio
   data from rtl_433 JSON logs.

   Written by HDTodd@gmail.com, 2022.05.16
   Modified 2026.06.20 to report more than just SNR data values
*/

#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <getopt.h>
#include <math.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

#include "mjson.h"
#include "stats.h"
#include "tree.h"

// These are global variables
time_t dFirst, dLast; // filled by CLI
char inFileName[60];  // filled by CLI
int fnLen = 39;
time_t timestamp;     // timestamp of packet currently being processed

// External procedure to process command line for parameters
extern int processCmdLine(int argc, char* argv[]);

// Update the statistics for this device's snr and freq
bool device_update(NPTR node, time_t timestamp, int thresh, double snr, double freq) {
  APTR attr;
  bool dup;
  
  attr = node->attr;
  dup = (attr->last_xmt_time == (time_t) 0)
    ? false : (timestamp < attr->last_xmt_time + (time_t)thresh);
  attr->pktcount++;
  attr->last_pkt_time = timestamp;
  stats_append(snr, attr->snr);
  if (freq != 0.0)
    stats_append(freq, attr->freq);
  // Do PPT before ITGT since last_xmt_time==0 is a flag 
  if (!dup && (attr->last_xmt_time != 0)) {
    stats_append((double)(attr->pkt_xmt), attr->ppt);
    attr->pkt_xmt = 0;
    };
  if (!dup) {
    if (attr->last_xmt_time != 0) {
      stats_append((double)(timestamp - attr->last_xmt_time), attr->itgt);
      attr->xmtcount++;
      };
    attr->last_xmt_time = timestamp;
  };
  attr->pkt_xmt++;
  return(dup);
}

// Print the statistics for the device pointed to by node 'p'
void node_print(NPTR p) {
  BSPTR s;
  printf("%-30s%6d %6d ", p->key, (p->attr)->pktcount, (p->attr)->xmtcount);
  s = (p->attr)->snr;
  printf("%5.1lf ± %4.1lf %5.1lf  %5.1lf   ",
	 s->mean, stats_stddev(s), s->min, s->max);
  s = (p->attr)->freq;
  printf("%7.3lf ± %5.3lf   %7.3lf  %7.3lf    ",
         s->mean, stats_stddev(s), s->min, s->max);
  s = (p->attr)->itgt;
  printf("%7.1lf ± %6.1lf %7.1lf  %7.1lf   ",
	 s->mean, stats_stddev(s), s->min, s->max);
  s = (p->attr)->ppt;
  printf("%4.1lf ± %3.1lf %3d  %3d",
	 s->mean, stats_stddev(s), (int) (s->min + 0.1), (int)(s->max + 0.1));

  printf("\n");
};

int main(int argc, char *argv[])
{
    char model[201];
    char channel[20];
    char id[20];
    char timestring[40];
    double snr;
    double freq;
    double freq1;
    char type[40];

    int option;
    unsigned int lc = 0, pc=0, tc=0; // line count, packet count, transmit count
    int status = 0;
    int errCode = 0;
    struct tm tm;
    NPTR root=NULL, node;
    clock_t tic, toc;
    int thresh=2;  // Time in sec between pkts for pkts to be considered duplicates
    bool dup;

    // Local internal vars
    FILE *fp;
    char *path = "test.json";
    char lbuf[501];
    time_t earliestDTS = (time_t) 0x7FFFFFFF; // watch out for 64-bit time_t, negative times                            
    time_t latestDTS   = (time_t) 0x00000000;
    char ft[80], lt[80];
    struct tm ts;

    // Fields to have mjson parse; values set to 0 or "" if not present
    const struct json_attr_t json_rtl[] = {
	{"time",   t_string,  .addr.string = timestring, .len = sizeof(timestring)},
	{"model",  t_string,  .addr.string = model,      .len = sizeof(model)},
	{"channel",t_string,  .addr.string = channel,    .len = sizeof(channel)},
	{"id",     t_string,  .addr.string = id,         .len = sizeof(id)},
	{"snr",    t_real,    .addr.real   = &snr},
	{"freq",   t_real,    .addr.real   = &freq},
	{"freq1",  t_real,    .addr.real   = &freq1},
	{"type",   t_string,  .addr.string = type,       .len = sizeof(type)},
	{"",       t_ignore},
	{NULL},
    };
  
    printf("\nsnr:\n%s%s%s",
	   "\tAnalyze rtl_433 JSON logs to catalog the devices seen and to characterize\n",
	   "\tstatistically their signal-to-noise ratio (SNR), radio frequency (Freq),\n",
	   "\ttimes between transmissions (ITGT), and packets per transmission (PPT).\n\n");

    // process the command line to retrieve options selected, leave in global vars, open file
    if ( (errCode=processCmdLine(argc, argv)) != 0 ) exit(errCode);
    fp = fopen(inFileName, "r");
    if (!fp) {
      perror(inFileName);
      exit(EXIT_FAILURE);
    };

    // Ready to process input file
    printf("Processing ISM messages recorded by rtl_433 from file %s\n", inFileName);
    tic = clock();
    while (fgets(lbuf, sizeof(lbuf), fp)) {
      lc++;  // count lines read
      status = json_read_object(lbuf, json_rtl, NULL);
      if (status != 0) {
	printf("Error return at line %d reading JSON object %s\n", lc, lbuf);
	puts(json_error_string(status));
      };
      pc++;  // count records processed
      
      // Ignore tire pressure readings, "TPMS"
      if (strcmp(type, "TPMS")==0) continue;

      // Get to work
      strptime(timestring, "%Y-%m-%d %H:%M:%S", &tm);
      tm.tm_isdst = 1;
      timestamp = mktime(&tm);
      if (timestamp<dFirst || timestamp>dLast) continue;  // ignore recs not in date-time range

      // If "freq" is a defined field in the packet, json_read_object() assigns its value to freq
      //   If not, freq is assigned a value of 0
      // So use the value of freq if it is non-zero, otherwise use the value of freq1 (which
      //   may also be zero, implying that there is no frequency value in the JSON record
      freq = (freq != 0.0 ) ? freq : freq1;
      // Statement below makes 'model/channel/id' the key for cataloging and summarizing
      // Change the following statement to experiment with other keys
      strcat(model, "/"); strcat(model, channel); strcat(model, "/"); strcat(model, id);

#ifdef DEBUG
      printf("Line %d: timestamp=%s=%lu, model=%s, snr=%lf, freq=%lf\n",
	     lc, timestring, timestamp, model, snr, freq);
#endif

      // record earliest and last record timestamps
      earliestDTS = (timestamp<earliestDTS) ? timestamp : earliestDTS;
      latestDTS   = (timestamp>latestDTS)   ? timestamp : latestDTS;

      // Find or create the node for this device
      node = node_find(root,model);
      if (root == NULL) root = node;
      if (node == NULL) {
	  fprintf(stderr, "NULL node for %s at line %d\n", model, lc);
	  exit(EXIT_FAILURE);
      };

      //  OK, we have the device node; see if it's a dup packet & update its stats
      dup = device_update(node,timestamp,thresh,snr,freq);
      if (!dup) tc++;
    }; // end while loop
    if (!feof(fp)) {
      fprintf(stderr, "?Error reading at line %d; not at EOF?", lc);
      exit(EXIT_FAILURE);
    };

    // Finished input file; print result summary
    toc = clock();
    ts = *localtime(&earliestDTS);
    strftime(ft,sizeof(ft),"%a %Y-%m-%d %H:%M:%S", &ts);
    ts = *localtime(&latestDTS);
    strftime(lt,sizeof(lt),"%a %Y-%m-%d %H:%M:%S", &ts);
    printf("\nProcessed %d Packets as %d De-Duplicated Transmissions in %7.3lf sec\n",
	   pc, tc, ( (double) (toc-tic) / CLOCKS_PER_SEC) );
    printf("Packets Dated from %s to %s\n\n",ft, lt);
    printf("%-38s              Signal-to-Noise", " ");
    printf("%-15s Frequency (MHz)", " ");
    printf("\n");
    printf("%-38s       ________________________", " ");
    printf("%-2s __________________________________", " ");
    printf("\n");
    printf("%-30s%6s %6s  ", "Device","#Pkts", "#Xmits");
    printf("%-14s %6s %6s   ",  "Mean ±   𝜎", "Min", "Max");
    printf("%14s     %6s  %6s  ", "Mean    ±  𝜎   ", "Min ", "Max");
    printf("\n");
    tree_process(root, &node_print);

    if (fclose(fp)) {
      perror(path);
      exit(EXIT_FAILURE);
    };
    
    exit(EXIT_SUCCESS);
}
