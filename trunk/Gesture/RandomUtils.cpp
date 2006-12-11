#include "../precomp.h"
#include "RandomUtils.h"


/**
 * return a random, uniformly distributed int between min and max
 * [min, max)
 */
int RandomInt(int min, int max)
{
  double r = RandomDouble(0, 1);
  return (int)(r * (double)(max - min) + (double)min);
}

/**
 * return a random, uniformy distributed double between min and max
 * [min,max)
 */
double RandomDouble(double min, double max) 
{
  double number;

  number = ((double)rand() / (double)(RAND_MAX+1) * (max - min)) + min;
 

  return number;
}

/**
 * implements the polar form of the Box-Muller Tranformation to generate
 * a random guassian number from a uniform random distribution.
 * see ftp://www.taygeta.com/pub/c/boxmuller.c
 * also see the java docs reference for Random.nextGuassian()
 */
double RandomGaussian(double mean, double variance) {
  double x1, x2, w, y1;
  static double y2;
  static bool use_last = false;
  
  if(use_last) { //already calculated one, so use it
    y1 = y2;
    use_last = false;
  } else {
    do {
      x1 = 2.0 * RandomDouble(0,1) - 1.0; //between -1.0 and 1.0
      x2 = 2.0 * RandomDouble(0,1) - 1.0; //between -1.0 and 1.0
      w = x1 * x1 + x2 * x2;
      
      } while (w >= 1.0);
    
    w = sqrt((-2.0 * log(w)) / w);
    y1 = x1 * w;
    y2 = x2 * w;
    use_last = true;
  }
  
  return(mean + y1 * sqrt(variance));
}
