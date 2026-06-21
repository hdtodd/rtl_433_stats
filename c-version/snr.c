/* snr.c -- collect basic statistics on signal-to-noise ratio
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

char model[201];
char channel[20];
char id[20];
char timestring[40];
double snr;
double freq;
double freq1;

const struct json_attr_t json_rtl[] = {
  {"time",   t_string,  .addr.string = timestring, .len = sizeof(timestring)},
  {"model",  t_string,  .addr.string = model,      .len = sizeof(model)},
  {"channel",t_string,  .addr.string = channel,    .len = sizeof(channel)},
  {"id",     t_string , .addr.string = id,          .len = sizeof(id)},
  {"snr",    t_real,    .addr.real   = &snr},
  {"freq",   t_real,    .addr.real   = &freq},
  {"freq1",  t_real,    .addr.real   = &freq1},
  {"",       t_ignore},
  {NULL},
};;

// Filled in by cli
time_t dFirst, dLast;
char inFileName[60];
int fnLen = 39;

// Print the statistics for the device pointed to by 'p'
void node_print(NPTR p) {
  BSPTR s;
  printf("%-27s", p->key);
  s = (p->attr)->snr;
  printf("%6d %6d  %5.1lf ± %4.1lf %6.1lf %6.1lf    ",
         s->count, s->count, s->mean, stats_stddev(s), s->min, s->max);
  s = (p->attr)->freq;
  printf("%7.3lf ± %5.3lf  %7.3lf  %7.3lf    ",
         s->mean, stats_stddev(s), s->min, s->max);
  printf("\n");
};


// Local internal vars
extern int processCmdLine(int argc, char* argv[]);
FILE *fp;
char *path = "test.json";
char lbuf[501];
time_t timestamp, lasttime = 0;
char lastmodel[201] = "";
time_t earliestDTS = (time_t) 0x7FFFFFFF;           // watch out for 64-bit time_t, negative times                            
time_t latestDTS   = (time_t) 0x00000000;
char ft[80], lt[80];
struct tm ts;

int main(int argc, char *argv[])
{
    int option;
    unsigned int lc = 0, rc=0;
    int status = 0;
    int errCode = 0;
    struct tm tm;
    NPTR  root=NULL, base, node;
    APTR  attr;
    BSPTR snrstats;
    clock_t tic, toc;
    
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
      lc++;
      status = json_read_object(lbuf, json_rtl, NULL);
      strptime(timestring, "%Y-%m-%d %H:%M:%S", &tm);
      tm.tm_isdst = 1;
      timestamp = mktime(&tm);
      if (timestamp<dFirst || timestamp>dLast) continue;  // ignore recs not in date-time range
      // If "freq" is a defined field in the packet, json_read_object() assigns its value to freq
      //   If not, freq is assigned a value of 0
      // So use the value of freq if it is non-zero, otherwise use the value of freq1 (which
      //   may also be zero, implying that there is no frequency value in the JSON record
      freq = (freq != (double)0.0 ) ? freq : freq1;

      // Statement below makes 'model/channel/id' the key for cataloging and summarizing
      // Change the following statement to experiment with other keys
      strcat(model, "/"); strcat(model, channel); strcat(model, "/"); strcat(model, id);
      if ( (strcmp(model, lastmodel) != 0) ||
	   (timestamp > lasttime+2) ) {
#ifdef DEBUG
        printf("Line %d: timestamp=%s=%lu, model=%s, snr=%lf\n", lc, timestring, timestamp, model, snr);
#endif
	rc++;
	node = node_find(root,model);
	if (root == NULL) root = node;
	if (node != NULL)
	  stats_append(snr, (node->attr)->snr);
	if (freq != (double)0.0)
	  stats_append(freq, (node->attr)->freq);
	else {
	  fprintf(stderr, "NULL node for %s at line %d\n", model, lc);
	  exit(EXIT_FAILURE);
	};

	if (timestamp<earliestDTS) earliestDTS = timestamp;
	if (timestamp>latestDTS)   latestDTS  = timestamp;
        strcpy(lastmodel, model);
        lasttime = timestamp;
      };
      if (status != 0) puts(json_error_string(status));
    };
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
	   rc, rc, ( (double) (toc-tic) / CLOCKS_PER_SEC) );
    printf("Packets Dated from %s to %s\n\n",ft, lt);
    printf("%-34s              Signal-to-Noise", " ");
    printf("%-15s Frequency (MHz)", " ");
    printf("\n");
    printf("%-34s         _________________________", " ");
    printf("%-2s  _________________________________", " ");
    printf("\n");
    printf("%-25s  %6s %6s  ", "Device","#Pkts", "#Xmits");
    printf("%-14s  %6s%6s    ",  " Mean ±   𝜎", "Min", "Max");
    printf("%14s%6s  %6s   ", " Mean    ±  𝜎   ", "Min ", "Max");
    printf("\n");
    tree_process(root, &node_print);

    if (fclose(fp)) {
      perror(path);
      exit(EXIT_FAILURE);
    };
    
    exit(EXIT_SUCCESS);
}
