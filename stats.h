//stats.h
//  Data structure to contain basic statistics for
//    a stream of data values:
//    count of # of values, mean, std dev, min, max
//  hdtodd@gmail.com, 2022.05.22

typedef struct {
  int    count;
  double mean;
  double std2;
  double min;
  double max;
  } bstats;

bstats  *stats_new(void);
void    stats_print(bstats *data);
bstats  *stats_get(bstats *data);
void    stats_append(double x, bstats *data);

