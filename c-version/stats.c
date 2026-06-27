/* stats.c -- functions to instantiate a basic statistics data node
   (count of records, mean, standard deviation, min valule, max value)
   and to accumulate basic statistics from a stream of data through
   a series of calls to 'append'.  Uses recursive algorithm to
   compute mean and std deviation.

   WARNING: std2, stored internally, is the SQUARE of the std dev.
   Use '(double)stats_stddev(BSPTR data)' to obtain the std dev.

   hdtodd@gmail.com, 2022.05.22
   Modified 2026.06.20
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "stats.h"

//  Creates a new data node to hold the basic statistics data
//  and initializes the field values.
BSPTR stats_new(void) {
  BSPTR data;
  
  data = (BSPTR) malloc(sizeof(BSTATS));
  if (data == NULL) {
      fprintf(stderr, "Unable to allocate space for a stats block!");
      exit(EXIT_FAILURE);
  };

  data->count = 0;
  data->mean  = (double)0.0e0;
  data->std2  = (double)0.0e0;
  data->min   = (double)+INFINITY;
  data->max   = (double)-INFINITY;
  return(data);
};

//  Returns the value of the standard deviation of the stream of values
double stats_stddev(BSPTR data) {
  return( sqrt(data->std2) );
};

//  Prints the statistics for the stream of data values
void stats_print(BSPTR data) {
  printf("%6d %6.1lf ± %4.1lf %6.1lf %6.1lf\n",
	 data->count, data->mean, stats_stddev(data), data->min, data->max);
  return;
};

//  Appends the value x to the accumulating statistics for the stream
//    of data values pointed to by (BSTAT *)self
void stats_append(double x, BSPTR self) {
  self->count++;
  self->mean   = ( (double)(self->count - 1) * self->mean + x)/(double)self->count;
  self->std2   = self->count<2 ? 0.0 :
    ( (self->count-2)*self->std2 +
      (double)(self->count)*(self->mean-x)*(self->mean-x)/(double)(self->count - 1) )/(double)(self->count-1);
  self->min    =  x<self->min ? x : self->min;
  self->max    =  x>self->max ? x : self->max;
  return;
};
