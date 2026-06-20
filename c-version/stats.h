//stats.h
//  Data structure to contain basic statistics for
//    a stream of data values:
//    count of # of values, mean, std dev, min, max
//  hdtodd@gmail.com, 2022.05.22
//  Modified 2026.06.20 to add 'stats_stddev' function

typedef struct {
  int    count;
  double mean;
  double std2;
  double min;
  double max;
} BSTATS, *BSPTR;;

BSPTR   stats_new(void);
void    stats_print(BSPTR data);
void    stats_append(double x, BSPTR data);
double  stats_stddev(BSPTR data);

