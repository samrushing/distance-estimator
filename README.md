distance-estimator
==================

Uses the Distance Estimator method to compute very nice, detailed
images of the Mandelbrot Set.

Note: I originally wrote this program in 1989 or 1990, it took
me a while to find it when I needed it again!  [And when I did
find it, I was horrified by the poor style!] For some reason it
seems to have disappeared from the Usenet archives at Google.  [I
*think* I posted it to either comp.sources or maybe sci.fractals]

In 2011 I used this to generate a bitmap image to laser-engrave my
17" MacBookPro:

![Lasered MacBookPro](http://dark.nightmare.com/rushing/photos/misc/laptop.jpg)

distpbm.c
---------
The original, emits a pbm file.  You want to pipe this into pnmtoxxx
immediately, because its output consists literally of '1' and '0'
characters.

distps.c
--------
[in 1989] I had some trouble getting 11x17 postscript output from pbm,
so I modified the original to emit 'device' postscript (pixel for pixel).

distjules.c
-----------
Don't remember writing this.
