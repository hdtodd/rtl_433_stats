/*  snr-cli.c
    Command-line processor for SNR -- signal-to-noise ratio
    analysis program for RTL_433 packets.

    hdtodd@gmail.com, 2022.05.17
*/

#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

extern time_t dFirst, dLast;
extern int thresh;
extern int noise;
extern bool TPMS
;
extern char* version;
extern char inFileName[60];

int processCmdLine(int argc, char* argv[]) {
  int c, i, j;
  const char* short_opt = "h?vTi:x:w:s:e:";
  struct tm tm;

  struct myOption {
    char* name;
    int   has_arg;
    int*  flag;
    int   val;
    char* desc;
  };

  struct myOption long_opt[] = 
    {  
      {"help",          optional_argument, NULL, 'h', "Print this help message and exit"},
      {"version",       optional_argument, NULL, 'v', "Print version number and exit"},
      {"input",         required_argument, NULL, 'i', "-i <path>: source data file to process"},
      {"exclude_noise", required_argument, NULL, 'x', "-x <minimum # of packets for device to be reported (default: 1)>"},
      {"xmt_window",    required_argument, NULL, 'w', "-w <maximum time in sec for a packet group to be considered one transmission (default: 2)>"},
      {"include_TPMS",  optional_argument, NULL, 'T', "Include tire pressure monitors in the catalog (default: exclude)"},
      {"start",         required_argument, NULL, 's', "-s <date>: date-time of first record to process, in form YYYY-MM-DD HH:MM:SS"},
      {"end",           required_argument, NULL, 'e', "-e <date>: date-time of last record to process, in form YYYY-MM-DD HH:MM:SS"},
      {NULL,            0,                 NULL,  0,  NULL }
    };

  // validate arguments and/or provide help

  // Set default values for options, then check for command-line values
  dFirst     = 0;                 // Process all dates
  dLast      = 0x7FFFFFFF;        // Process all dates; watch for neg 64-bit time
  
  //  If no options provided, simulate "-h"
  c = getopt_long(argc, argv, short_opt, (const struct option *)long_opt, NULL);
  if (c == -1) c = 'h';
  do {
    switch(c) {
      case -1:       /* no more arguments */
      case 0:        /* long options toggles */
	break;

      case '?':
      case 'h':
	printf("SNR: Program to analyze device characteristics from rtl_433 packet logs\n");
	printf("     Usage: %s [OPTIONS]\n", argv[0]);
	printf("     [OPTIONS] are any combination of\n\tLong form       Short\tOption invoked\n");
	for (i=0; long_opt[i].name!=NULL; i++) {
	  printf("\t--%-14s-%c\t%s\n", long_opt[i].name, long_opt[i].val, long_opt[i].desc);
	};
        exit(0);

      case 'v':
	printf("SNR version %s \n", version);
	exit(0);
	
      case 'i':
	if (strlen(optarg)<(sizeof(inFileName)-1)) {
	  strcpy(inFileName, optarg);
	  if (access(inFileName,R_OK) !=0) {
	    printf("Specified input file '%s' not found or not readable\n", inFileName);
	    return(-1);
	  }
	} else {
	  printf("Input data file name '%s' exceeds allocated storage\n", inFileName);
	  return(-1);
	  };
	break;
      
    case 'x':
        noise = atoi(optarg);
	break;

    case 'w':
        thresh = atoi(optarg);
        break;

    case 'T':
        TPMS = true;
        break;

    case 's':
	memset(&tm, 0, sizeof(struct tm));
	strptime(optarg, "%Y-%m-%d %H:%M:%S", &tm);
	tm.tm_isdst = 1;
	dFirst = mktime(&tm);
	break;

      case 'e':
	memset(&tm, 0, sizeof(struct tm));
	strptime(optarg, "%Y-%m-%d %H:%M:%S", &tm);
	tm.tm_isdst = 1;
	dLast = mktime(&tm);
	break;

      default:
	fprintf(stderr, "%s: invalid option -- %c\n", argv[0], c);
	fprintf(stderr, "Try `%s --help' for more information.\n", argv[0]);
	return(-2);
    };
  }
  while ((c = getopt_long(argc, argv, short_opt, (const struct option *)long_opt, NULL)) != -1);;

  return(0);
};

