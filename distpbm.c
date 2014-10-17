/* This program uses the Distance Estimator to calculate a  detailed
 * image of the Mandelbrot set, with output in pbm.
 *
 * Reference:
 * H. Peitgen and D. Saupe, eds., _The Science of Fractal Images_, Springer-
 * Verlag Inc., New York, 1988.  ISBN 0-387-96608-0.
 * 
 * WARNING: this is a no-frills program.  Here are a few suggestions:
 *
 * o Use a nice fast xaos-like program to find a good area, *then*
 *   use distpbm to get a beautiful hardcopy.
 * o Find a fast machine, this takes forever.
 * o Find the lowest value for maxiter that works for your image.
 *
 * To compile and link:
 *
 * gcc -O3 distpbm.c -o distps -lm
 *
 * To run:
 *            x          y      range   W   H  maxiter threshold (in pixels) 
 * distpbm -0.800049 -0.167122 0.010537 640 640   256       1        > image.pbm
 *
 * A more modern usage:
 * distpbm -1.26541935 -.41499480 .000005438 9000 6000 1500 1000 | pnmtopng > image.png
 * 
 * Author: Sam Rushing
 * Contact: http://www.nightmare.com/~rushing/
 * License: public domain
 *
 * 2011 Note: I originally wrote this program in 1989 or 1990, it took
 *   me a while to find it when I needed it again!  [And when I did
 *   find it, I was horrified by the poor style!] For some reason it
 *   seems to have disappeared from the Usenet archives at Google.  [I
 *   *think* I posted it to either comp.sources or maybe sci.fractals]
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double overflow;
double * xorbits, * yorbits;

double huge;

void 
usage(char * argv[])
{ 
  fprintf (stderr, "Usage: \n  %s x y range width height maxiter threshold\n", argv[0]);
  exit(-1);
}

void
barf(char *what, int which)
{
  fprintf (stderr, "You hosed argument %d, \"%s\"\n",which,what);
  exit(-1);
}

/* estimate the distance from (cx, cy) to the mandelbrot set. */

double
estimate_distance (double cx, double cy, long maxiter)
{
  long iter, i;
  double x,y,x2,y2,temp,dist;
  double xder,yder;
  int flag = 0;
  
  x = y = x2 = y2 = dist = xorbits[0] = yorbits[0] = 0.0;
  iter = 0;
  huge = 100000.0;
  
  while ((iter < maxiter) && ((x2 + y2) < huge)) {
    temp = x2 - y2 + cx;
    y = 2 * x * y + cy;
    x = temp;
    x2 = x * x;
    y2 = y * y;
    xorbits[iter] = x;
    yorbits[iter] = y;
    iter++;
  }
  if ((x2 + y2) > huge) {
    xder = yder = 0.0;
    i = 0;
    flag = 0;
    while ((i < (iter - 1)) && (!flag)) {
	temp = 2 * (xorbits[i] * xder - yorbits[i] * yder) + 1;
	yder = 2 * (yorbits[i] * xder + xorbits[i] * yder);
	xder = temp;
	if ((fabs (xder) > overflow) || (fabs (yder) > overflow)) {
	  flag = 1;
	}
	i++;
      }
    if (!flag) {
      dist = (log (x2 + y2) * sqrt (x2 + y2) / sqrt (xder * xder + yder * yder));
    } else {
      dist = huge;
    }
  }
  return (dist);
}


/* generate a Portable BitMap image of the mandelbrot set */

int
main(int argc, char * argv[])
{
  double x,y,range,xmin,xmax,ymin,ymax,delta,width_half,height_half;
  long width,height,maxiter,threshold,hundredth;
  long ix, iy;
  int linepos;
  double dist,cx,cy;
  int arg = 1;
  int data = 0, power = 0;
  
  if (argc < 8) 
    usage(argv); 
  
  if (!(sscanf (argv[arg++], "%lf",&x))) barf ("x",arg);
  if (!(sscanf (argv[arg++], "%lf",&y))) barf ("y",arg);
  if (!(sscanf (argv[arg++], "%lf",&range))) barf ("range",arg);
  if (!(sscanf (argv[arg++], "%ld",&width))) barf ("width",arg);
  if (!(sscanf (argv[arg++], "%ld",&height))) barf ("height",arg);
  if (!(sscanf (argv[arg++], "%ld",&maxiter))) barf ("maxiter",arg);
  if (!(sscanf (argv[arg++], "%ld",&threshold))) barf ("threshold",arg);

  printf ("P1\n%ld %ld\n", width, height);

  /* figure out aspect ratios */

  if (width > height) {
    width_half = range / 2.0;
    height_half = ((range * ((double)height / (double)width)) / 2.0);
  } else {
    width_half = ((range * ((double)width / (double)height)) / 2.0);
    height_half = range / 2.0;
  }

  xmin = x - width_half;
  ymin = y - height_half;
  xmax = x + width_half;
  ymax = y + height_half;
  
  linepos = 0;

  delta = threshold * (xmax - xmin) / (width - 1);

  overflow = pow (delta,-2);

  fprintf (stderr, "overflow = %f\n", overflow);

  /* allocate memory for orbit arrays */

  xorbits = (double *) malloc (maxiter * sizeof (double));
  yorbits = (double *) malloc (maxiter * sizeof (double));  
  
  /* hurl if malloc failed */

  if (!xorbits || !yorbits) {
    return (-1);
  }

  hundredth = height/100;
  
  for (iy = 0; iy < height ; iy++) {
    cy = ymin + iy * (ymax - ymin) / (height - 1);
    /* print out '.'s to let the user see progress */
    if (iy > hundredth) {
      fprintf (stderr, ".");
      hundredth += height/100;
    }
    for (ix = 0; ix < width; ix++) {
      cx = xmin + ix * (xmax - xmin) / (width - 1);
      dist = estimate_distance (cx, cy, maxiter);
      if ((linepos += 2) > 70) {
	linepos = 0; printf ("\n"); }
      if (dist < delta) {
	printf("1 ");
      } else {
	printf("0 ");
      }
    }
  }
  fprintf (stderr, "done.\n\n");
}
