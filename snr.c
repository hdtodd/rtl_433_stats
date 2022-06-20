/* snr.c -- collect basic statistics on signal-to-noise ratio
   data from rtl_433 JSON logs.

   Written by HDTodd@gmail.com, 2022.05.16
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
char timestring[40];
char id[20];
double snr;

const struct json_attr_t json_rtl[] = {
  {"time",   t_string,  .addr.string = timestring, .len = sizeof(timestring)},
  {"model",  t_string,  .addr.string = model,      .len = sizeof(model)},
  {"id",     t_string , .addr.string = id,          .len = sizeof(id)},
  {"snr",    t_real,    .addr.real    = &snr},
  {"",       t_ignore},
  {NULL},
};;

// Filled in by cli
time_t dFirst, dLast;
char inFileName[40];
int fnLen = 39;

void node_print(NPTR p) {
    stats_get( (bstats *)p->attr);
    printf("%-27s", p->key);
    stats_print( (bstats *)p->attr);
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

void main(int argc, char *argv[])
{
    int option;
    unsigned int lc = 0, rc=0;
    int status = 0;
    int errCode = 0;
    struct tm tm;
    NPTR root=NULL, base, node;
    APTR attr;
    bstats *snrstats;

    printf("\nsnr: Analyze rtl_433 json log files\n");
    // process command line to retrieve options selected, leave in global vars
    if ( (errCode=processCmdLine(argc, argv)) != 0 ) exit(errCode);
    fp = fopen(inFileName, "r");
    if (!fp) {
      perror(inFileName);
      exit(EXIT_FAILURE);
    };

    // Ready to process input file
    printf("Processing ISM 433MHz messages from file %s\n", inFileName);
    while (fgets(lbuf, sizeof(lbuf), fp)) {
      lc++;
      status = json_read_object(lbuf, json_rtl, NULL);
      strptime(timestring, "%Y-%m-%d %H:%M:%S", &tm);
      tm.tm_isdst = 1;
      timestamp = mktime(&tm);
      if (timestamp<dFirst || timestamp>dLast) continue;  // ignore recs not in date-time range
      strcat(model, " "); strcat(model, id);
      if ( (strcmp(model, lastmodel) != 0) ||
	   (timestamp > lasttime+2) ) {
#ifdef DEBUG
        printf("Line %d: timestamp=%s=%lu, model=%s, snr=%lf\n", lc, timestring, timestamp, model, snr);
#endif
	rc++;
	node = node_find(root,model);
	if (root == NULL) root = node;
	if (node != NULL)
	  stats_append(snr, (bstats *)node->attr);
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
    ts = *localtime(&earliestDTS);
    strftime(ft,sizeof(ft),"%a %Y-%m-%d %H:%M:%S", &ts);
    ts = *localtime(&latestDTS);
    strftime(lt,sizeof(lt),"%a %Y-%m-%d %H:%M:%S", &ts);
    printf("\nProcessed %d de-duplicated records\nDated from %s to %s\n\n", rc, ft, lt);
    printf("%-25s  %6s  %14s %6s %6s\n",
	   "Device","#Recs", "Mean SNR ± 𝜎", "Min", "Max");
    tree_process(root, &node_print);

    if (fclose(fp)) {
      perror(path);
      exit(EXIT_FAILURE);
    };

}
