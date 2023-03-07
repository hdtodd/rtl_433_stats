/* stats.c -- functions to instantiate a basic statistics data node
   (count of records, mean, standard deviation, min valule, max value)
   and to accumulate basic statistics from a stream of data through
   a series of calls to 'append'.  Uses recursive algorithm to
   compute mean and std deviation.

   WARNING: std2, stored internally, is the SQUARE of the std dev.
   Need to perform the 'get' to convert to std dev when all
   stream data have been processed.

   hdtodd@gmail.com, 2022.05.22
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "stats.h"

//  creates a new data node to hold the basic statistics data
bstats *stats_new(void) {
  bstats *data;
  
  data = (bstats *) malloc(sizeof(bstats));
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

void stats_print(bstats *data) {
  //  printf("Count = %d, avg = %lf, std2 dev = %lf, min = %lf, max = %lf\n",
  printf("%6d %6.1lf ± %4.1lf %6.1lf %6.1lf\n",
	 data->count, data->mean, data->std2, data->min, data->max);
  return;
};

bstats *stats_get(bstats* data) {
  data->std2 = sqrt(data->std2);
  return(data);
};

void stats_append(double x, bstats *self) {
  self->count += 1;
  self->mean   = ( (double)(self->count - 1) * self->mean + x)/(double)self->count;
  self->std2   = self->count<2 ? 0.0 :
    ( (self->count-2)*self->std2 +
      (double)(self->count)*(self->mean-x)*(self->mean-x)/(double)(self->count - 1) )/(double)(self->count-1);
  self->min    =  x<self->min ? x : self->min;
  self->max    =  x>self->max ? x : self->max;

  return;
};
