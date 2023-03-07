#!/bin/python CLASS STATS
# A class to compute mean, standard deviation, min, and max of a stream
#   of numbers in a single pass via recursion
# hdtodd@gmail.com, May, 2022

# Use:
#   Instantiate:  xstats = stats.stats()
#   Append another sample, x, to the accumulated stats:
#      xstats.append(x)
#   Retrieve results:
#      (sample_count, mean, std_dev, min, max) = xstats.get()

import math

class stats():

   def __init__(self, x):
      self.count = 1
      self.mean  = x
      self.std2  = 0.0
      self.min   = x
      self.max   = x
      
   def append(self,x):
      self.count += 1
      self.mean   = ((self.count-1)*self.mean + x)/self.count
      self.std2   = 0 if self.count<2 else ( (self.count-2)*self.std2 +
                                             (self.count*(self.mean-x)**2)/(self.count-1) )/(self.count-1)
      if self.count > 1:
         self.min    = x if x<self.min else self.min
         self.max    = x if x>self.max else self.max
      return

   def get(self):
      return (self.count, self.mean, math.sqrt(self.std2), self.min, self.max)
