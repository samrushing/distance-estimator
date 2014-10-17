/* This program uses the Distance Estimator to calculate a  detailed
 * image of the Mandelbrot set, with output in PostScript.
 *
 * Reference:
 * H. Peitgen and D. Saupe, eds., _The Science of Fractal Images_, Springer-
 * Verlag Inc., New York, 1988.  ISBN 0-387-96608-0.
 * 
 * WARNING: this is a no-frills program.  Here are a few suggestions:
 *
 * o 'width' must be a multiple of 8.  (the ps program reads in bytes)
 * o Use a nice fast fractint-like program to find a good area, *then*
 *   use distps to get a beautiful hardcopy.
 * o Find a fast machine, this takes forever.
 * o Find the lowest value for maxiter that works for your image.
 *
 * To compile and link:
 *
 * gcc -O2 distps.c -o distps -lm
 *
 * To run:
 *            x          y      range   W   H  maxiter threshold (in pixels) 
 * distps -0.800049 -0.167122 0.010537 640 640   256       1        > image.ps
 * 
 * Author: Sam Rushing
 * Contact: rushing@titan.ksc.nasa.gov, hoggle!hoggle2!rushing@peora.sdc.ccur.com
 * Conditions: None.  Do with this what you will.  Enjoy!
 *
 * Comments: I have a 'distpnm' (outputs Portable Anymap).  I couldn't
 * get pnmtops to generate a full-resolution 11x17 image.  Therefore this.
 *
 * 2400x3150 = 8" x 10.5"
 * 3200x5000 = 11" x 17"
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double overflow;
double * xorbits, * yorbits;

/* Change this for your printer.  For example, a NeXT printer is a */
/* 400dpi machine  */

#define DPI 300.0

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
estimate_distance (double cx,
		   double cy,
		   long maxiter)
{
  long iter, i;
  double x,y,x2,y2,temp,dist;
  double xder,yder;
  int flag = 0;
  
  x = y = x2 = y2 = dist = xorbits[0] = yorbits[0] = 0.0;
  iter = 0;
  huge = 100000.0;
  
  while ((iter < maxiter) && ((x2 + y2) < huge)) 
    {
      temp = x2 - y2 + cx;
      y = 2 * x * y + cy;
      x = temp;
      x2 = x * x;
      y2 = y * y;
      xorbits[iter] = x;
      yorbits[iter] = y;
      iter++;
    }
  if ((x2 + y2) > huge)
    {
      xder = yder = 0.0;
      i = 0;
      flag = 0;
      
      while ((i < (iter - 1)) && (!flag))
	{
	  temp = 2 * (xorbits[i] * xder - yorbits[i] * yder) + 1;
	  yder = 2 * (yorbits[i] * xder + xorbits[i] * yder);
	  xder = temp;
	  if ((fabs (xder) > overflow) || (fabs (yder) > overflow))
	    flag = 1;
	  i++;
	}
      if (!flag) {
	dist = (log (x2 + y2) * sqrt (x2 + y2) / 
		sqrt (xder * xder + yder * yder));
      }
      else
	dist = huge;
    }
  return (dist);
}


/* generate a PostScript image of the mandelbrot set */

int
main(int argc, char * argv[])
{
  double x,y,range,xmin,xmax,ymin,ymax,delta,width_half,height_half;
  long width,height,maxiter,threshold,tenth;
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

  printf ("/picstr %ld string def\n",width);
  printf ("10 10 translate \n");
  printf ("%f %f scale\n",
	  (float) ((width / DPI) * 72.0),
	  (float) ((height / DPI) * 72.0));
  printf ("%ld %ld 1 [ %ld 0 0 %ld 0 0 ]\n", width, height, width, height);
  printf ("{ currentfile picstr readhexstring pop } image\n");

  /* figure out aspect ratios */

  if (width > height)
    {
      width_half = range / 2.0;
      height_half = ((range * (height / width)) / 2.0);
    }
  else
    {
      width_half = ((range * (height / width)) / 2.0);
      height_half = range / 2.0;
    }

  xmin = x - width_half;
  ymin = y - width_half;
  xmax = x + height_half;
  ymax = y + height_half;
  
  linepos = 0;

  delta = threshold * (xmax - xmin) / (width - 1);

  overflow = pow (delta,-2);

  fprintf (stderr, "overflow = %f\n", overflow);

  /* allocate memory for orbit arrays */

  xorbits = (double *) malloc (maxiter * sizeof (double));
  yorbits = (double *) malloc (maxiter * sizeof (double));  
  
  /* hurl if malloc failed */

  if (!xorbits || !yorbits)
    return (-1);

  tenth = height/10;
  
  for (iy = 0; iy < height ; iy++)
    {
      cy = ymin + iy * (ymax - ymin) / (height - 1);
      
      /* print out '*'s to let the user see progress */

      if (iy > tenth) {
	fprintf (stderr, "*");
	tenth += height/10;
      }

      for (ix = 0; ix < width; ix++) 
	{
	  /* output a byte of image data, in hex */

	  if (power == 8) {
	    power = 0;
	    printf ("%02x", data);
	    data = 0;
	    
	    /* each line 70 chars */
	    if ((linepos += 2) > 70)
	      { linepos = 0; printf ("\n"); }
	  }
	  cx = xmin + ix * (xmax - xmin) / (width - 1);
	  dist = estimate_distance (cx, cy, maxiter);

	  if (dist >= delta)
	    data |= (1<<(7-power));
	  power++;

	}
    }
  printf ("%02x\nshowpage\n",data);
  return (0);

}	  
	  
