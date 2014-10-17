/* This program uses the Distance Estimator to calculate a  detailed
 * image of the Mandelbrot set, with output in pbm.
 *
 * Reference:
 * H. Peitgen and D. Saupe, eds., _The Science of Fractal Images_, Springer-
 * Verlag Inc., New York, 1988.  ISBN 0-387-96608-0.
 * 
 * WARNING: this is a no-frills program.  Here are a few suggestions:
 *
 * o Use a nice fast fractint-like program to find a good area, *then*
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
 * Author: Sam Rushing
 * Contact: rushing@titan.ksc.nasa.gov, hoggle!hoggle2!rushing@peora.sdc.ccur.com
 * Conditions: None.  Do with this what you will.  Enjoy!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

/*based on function  mndlbrot::dist  from  mndlbrot.cpp
 from program mandel by Wolf Jung (GNU GPL )
 http://www.mndynamics.com/indexp.html  */
double
jdist (double Zx, double Zy, double Cx, double Cy ,  int iter_max)
{ 
  int i;
  double x = Zx; /* Z = x+y*i */
  double y = Zy; /* Zp = xp+yp*1 = 1  */
  double xp = 1;
  double yp = 0;
  double nz;  /* temporary */
  double nzp;
  double a;   /* a = abs(z) */

  for (i = 1; i <= iter_max; i++) {
    /* first derivative   zp = 2*z*zp  = xp + yp*i; */
    nz = 2*(x*xp - y*yp) ; 
    yp = 2*(x*yp + y*xp); 
    xp = nz;
    /* z = z*z + c = x+y*i */
    nz = x*x - y*y + Cx; 
    y = 2*x*y + Cy; 
    x = nz; 
    /* */
    nz = x*x + y*y; 
    nzp = xp*xp + yp*yp;
    if (nzp > 1e60 || nz > 1e60) break;
  }
  a=sqrt(nz);
  /* distance = 2 * |Zn| * log|Zn| / |dZn| */
  return 2* a*log(a)/sqrt(nzp); 
}


/* generate a Portable BitMap image of the mandelbrot set */

int
main(int argc, char * argv[])
{
  double x,y,range,xmin,xmax,ymin,ymax,delta,width_half,height_half, threshold;
  long width,height,maxiter,hundredth;
  long ix, iy;
  int linepos;
  double dist,cx,cy,zx,zy;
  int arg = 1;
  int data = 0, power = 0;
  double mindist = 1234234234.0;

  if (argc < 8) 
    usage(argv); 
  
  // usually with the julia set, you want the whole thing, not some tiny sub-image.
  // we need cx/cy, we need w/h, and we need range.
  // +0.301813, -0.022009
  // 3000, 2000
  // 1.6, 1.6

  if (!(sscanf (argv[arg++], "%lf",&x))) barf ("x",arg);
  if (!(sscanf (argv[arg++], "%lf",&y))) barf ("y",arg);
  if (!(sscanf (argv[arg++], "%lf",&range))) barf ("range",arg);
  if (!(sscanf (argv[arg++], "%ld",&width))) barf ("width",arg);
  if (!(sscanf (argv[arg++], "%ld",&height))) barf ("height",arg);
  if (!(sscanf (argv[arg++], "%ld",&maxiter))) barf ("maxiter",arg);
  if (!(sscanf (argv[arg++], "%lf",&threshold))) barf ("threshold",arg);

  printf ("P2\n%ld %ld 256\n", width, height);

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

  delta = threshold * range / width;
  fprintf (stderr, "delta= %f\n", delta);
  fprintf (stderr, "threshold= %f\n", threshold);

  hundredth = height/100;
  
  for (iy = 0; iy < height ; iy++) {
    zy = ymin + iy * (ymax - ymin) / (height - 1);
    /* print out '.'s to let the user see progress */
    if (iy > hundredth) {
      fprintf (stderr, ".");
      hundredth += height/100;
    }
    for (ix = 0; ix < width; ix++) {
      zx = xmin + ix * (xmax - xmin) / (width - 1);
      // dist = estimate_distance (cx, cy, maxiter);
      dist = jdist (zx, zy, cx, cy, maxiter);
      if (dist < mindist) {
	mindist = dist;
      }
      fprintf (stderr, "[%f]", dist);
      if ((linepos += 2) > 70) {
	linepos = 0; printf ("\n"); }
      if (dist < delta) {
	printf ("%d ", (int) (dist / (threshold/256)));
	//printf("1 ");
      } else {
	printf("0 ");
      }
    }
  }
  fprintf (stderr, "mindist=%lf\n", mindist);
  fprintf (stderr, "done.\n\n");
}
