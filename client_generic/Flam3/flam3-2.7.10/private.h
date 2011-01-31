/*
    flam3 - cosmic recursive fractal flames
    Copyright (C) 1992-2004  Scott Draves <source@flam3.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#include "flam3.h"

#include <stdlib.h>

#include <math.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <libxml/parser.h>

#ifndef WIN32
#include <unistd.h>
#include <libgen.h>
#else
#define basename(x) strdup(x)
#endif

#include "config.h"

#define EPS (1e-10)
#define CMAP_SIZE 256
#define CMAP_SIZE_M1 255
#define rbit() (flam3_random_bit())
#define flam3_variation_none   (-1)
#define max_specified_vars     (100)
#define vlen(x) (sizeof(x)/sizeof(*x))

extern void rgb2hsv(double *rgb, double *hsv);
extern void hsv2rgb(double *hsv, double *rgb);


#ifdef WIN32
#define M_PI   3.1415926536
#define M_1_PI 0.3183098862
#define M_PI_4 0.7853981634
#define random()  (rand() ^ (rand()<<15))
#define srandom(x)  (srand(x))
extern int getpid();
#define rint(A) floor((A)+(((A) < 0)? -0.5 : 0.5))
#endif

#define argi(s,d)   ((ai = getenv(s)) ? atoi(ai) : (d))
#define argf(s,d)   ((ai = getenv(s)) ? atof(ai) : (d))
#define args(s,d)   ((ai = getenv(s)) ? ai : (d))

/* Spatial Filter Kernels */
#define  hermite_support      (1.0)
#define  box_support      (0.5)
#define  triangle_support   (1.0)
#define  bell_support      (1.5)
#define  b_spline_support   (2.0)
#define  mitchell_support   (2.0)
#define  mitchell_b   (1.0 / 3.0)
#define  mitchell_c   (1.0 / 3.0)
#define  blackman_support  (1.0)
#define  catrom_support    (2.0)
#define  hanning_support   (1.0)
#define  hamming_support   (1.0)
#define  lanczos3_support  (3.0)
#define  lanczos2_support  (2.0)
#define  gaussian_support  (1.8)
#define  quadratic_support (1.5)

double hermite_filter(double t);
double box_filter(double t);
double triangle_filter(double t);
double bell_filter(double t);
double b_spline_filter(double t);
double sinc(double x);
double lanczos3_filter(double t);
double lanczos2_filter(double t);
double mitchell_filter(double t);
double blackman_filter(double x);
double catrom_filter(double x);
double hamming_filter(double x);
double hanning_filter(double x);
double gaussian_filter(double x);
double quadratic_filter(double x);

void docstring();

/* Structures for passing parameters to iteration threads */
typedef struct {
   unsigned short *xform_distrib;    /* Distribution of xforms based on weights */
   flam3_frame *spec; /* Frame contains timing information */
   double bounds[4]; /* Corner coords of viewable area */
   double rot[2][2]; /* Rotation transformation */
   double size[2];
   int width, height; /* buffer width/height */
   double ws0, wb0s0, hs1, hb1s1; /* shortcuts for indexing */
   int fname_specified; /* Set to 1 if there was a filename specified for colormap */
   void *cmap; /* Points to bucket-based cmap if standard, uchar if fname specified */
   double color_scalar; /* <1.0 if non-uniform motion blur is set */
   void *buckets; /* Points to the first accumulator */
   double badvals; /* accumulates all badvalue resets */
   double batch_size;
   int temporal_sample_num,ntemporal_samples;
   int batch_num, nbatches, aborted;
} flam3_iter_constants;

typedef struct {
   double *iter_storage; /* Storage for iteration coordinates */
   randctx rc; /* Thread-unique ISAAC seed */
   flam3_genome cp; /* Full copy of genome for use by the thread */
   int thread_verbose;
   int timer_initialize;
   flam3_iter_constants *fic; /* Constants for render */
} flam3_thread_helper;
