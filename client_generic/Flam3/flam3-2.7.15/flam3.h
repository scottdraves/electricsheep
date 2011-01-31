/*
    FLAM3 - cosmic recursive fractal flames
    Copyright (C) 1992-2006  Scott Draves <source@flam3.com>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/


#ifndef flam3_included
#define flam3_included

#include <stdio.h>
#include <libxml/parser.h>
#include "isaac.h"


char *flam3_version();

#define flam3_palette_random       (-1)
#define flam3_palette_interpolated (-2)

#define flam3_defaults_on          (1)
#define flam3_defaults_off         (0)

#define flam3_name_len    64

#define flam3_print_edits  (1)
#define flam3_dont_print_edits  (0)

typedef double flam3_palette[256][3];

int flam3_get_palette(int palette_index, flam3_palette p, double hue_rotation);

#define flam3_variation_random (-1)
#define flam3_variation_random_fromspecified (-2)

extern char *flam3_variation_names[];

#define flam3_nvariations 54
#define flam3_nxforms     12

#define flam3_parent_fn_len     30

#define flam3_interpolation_linear 0
#define flam3_interpolation_smooth 1

#define flam3_inttype_linear 0
#define flam3_inttype_log    1
#define flam3_inttype_compat 2 /* Linear and old behaviour */
#define flam3_inttype_older  3 /* rotate padded xforms     */

#define flam3_palette_interpolation_hsv   0
#define flam3_palette_interpolation_sweep 1

#define flam3_max_action_length 10000

#define VAR_LINEAR   0
#define VAR_SINUSOIDAL  1
#define VAR_SPHERICAL  2
#define VAR_SWIRL 3
#define VAR_HORSESHOE  4
#define VAR_POLAR 5
#define VAR_HANDKERCHIEF 6
#define VAR_HEART 7
#define VAR_DISC 8
#define VAR_SPIRAL 9
#define VAR_HYPERBOLIC 10
#define VAR_DIAMOND 11
#define VAR_EX 12
#define VAR_JULIA 13
#define VAR_BENT 14
#define VAR_WAVES 15
#define VAR_FISHEYE 16
#define VAR_POPCORN 17
#define VAR_EXPONENTIAL 18
#define VAR_POWER 19
#define VAR_COSINE 20
#define VAR_RINGS 21
#define VAR_FAN 22
#define VAR_BLOB 23
#define VAR_PDJ 24
#define VAR_FAN2 25
#define VAR_RINGS2 26
#define VAR_EYEFISH 27
#define VAR_BUBBLE 28
#define VAR_CYLINDER 29
#define VAR_PERSPECTIVE 30
#define VAR_NOISE 31
#define VAR_JULIAN 32
#define VAR_JULIASCOPE 33
#define VAR_BLUR 34
#define VAR_GAUSSIAN_BLUR 35
#define VAR_RADIAL_BLUR 36
#define VAR_PIE 37
#define VAR_NGON 38
#define VAR_CURL 39
#define VAR_RECTANGLES 40
#define VAR_ARCH 41
#define VAR_TANGENT 42
#define VAR_SQUARE 43
#define VAR_RAYS 44
#define VAR_BLADE 45
#define VAR_SECANT2 46
#define VAR_TWINTRIAN 47
#define VAR_CROSS 48
#define VAR_DISC2 49
#define VAR_SUPER_SHAPE 50
#define VAR_FLOWER 51
#define VAR_CONIC 52
#define VAR_PARABOLA 53

typedef void (*flam3_iterator)(void *, double);


typedef struct {

   char *genome;
   char *badvals;
   char *numiters;
   char *rtime;

} flam3_img_comments;

typedef struct {

   double badvals;
   long int num_iters;
   int render_seconds;
   
} stat_struct;

typedef struct {

   unsigned int width, height;
   int version;
   int id;

   /* There are 256 levels of gray to work with */
   double intensity_weight[256];
   unsigned int bin_size[256];
   unsigned int bin_offset[256];

   /* Pointer to newly allocated memory; we will be allocating */
   /* 2*w*h ushorts for this storage.  The bin offset will     */
   /* provide the starting point for a random selection from   */
   /* (bin size) ordered pairs                                 */
   unsigned short *rowcols;

} flam3_image_store;


typedef struct {
   double var[flam3_nvariations];   /* interp coefs between variations */
   double c[3][2];      /* the coefs to the affine part of the function */
   double post[3][2];   /* the post transform */
   double density;      /* probability that this function is chosen. 0 - 1 */
   double color[2];     /* color coords for this function. 0 - 1 */
   double symmetry;     /* 1=this is a symmetry xform, 0=not */
   
   int padding;/* Set to 1 for padding xforms */
   double wind[2]; /* winding numbers */

   int precalc_sqrt_flag;
   int precalc_angles_flag;
   int precalc_atan_xy_flag;
   int precalc_atan_yx_flag;

   /* Params for new parameterized variations */
   /* Blob */
   double blob_low;
   double blob_high;
   double blob_waves;

   /* PDJ */
   double pdj_a;
   double pdj_b;
   double pdj_c;
   double pdj_d;

   /* Fan2 */
   double fan2_x;
   double fan2_y;

   /* Rings2 */
   double rings2_val;

   /* Perspective */
   double perspective_angle;
   double perspective_dist;

   /* Julia_N */
   double juliaN_power;
   double juliaN_dist;

   /* Julia_Scope */
   double juliaScope_power;
   double juliaScope_dist;

   /* Radial_Blur */
   double radialBlur_angle;

   /* Pie */
   double pie_slices;
   double pie_rotation;
   double pie_thickness;

   /* Ngon */
   double ngon_sides;
   double ngon_power;
   double ngon_circle;
   double ngon_corners;

   /* Image */
   /*
   int image_id;
   flam3_image_store *image_storage;
   */

   /* Curl */
   double curl_c1;
   double curl_c2;

   /* Rectangles */
   double rectangles_x;
   double rectangles_y;

   /* AMW */
   double amw_amp;

   /* Disc 2 */
   double disc2_rot;
   double disc2_twist;

   /* Supershape */
   double supershape_rnd;
   double supershape_m;
   double supershape_n1;
   double supershape_n2;
   double supershape_n3;
   double supershape_holes;
   
   /* Flower */
   double flower_petals;
   double flower_holes;
   
   /* Conic */
   double conic_eccen;
   double conic_holes;
   
   /* Parabola */
   double parabola_height;
   double parabola_width;
   
   /* Split */
//   double split_xsize;
//   double split_ysize;
//   double split_shift;
   
   /* Move */
//   double move_x;
//   double move_y;
   
   /* If perspective is used, precalculate these values */
   /* from the _angle and _dist                         */
   double persp_vsin;
   double persp_vfcos;

   /* If Julia_N is used, precalculate these values */
   double juliaN_rN;
   double juliaN_cn;

   /* If Julia_Scope is used, precalculate these values */
   double juliaScope_rN;
   double juliaScope_cn;

   /* If Radial_Blur is used, precalculate these values */
   double radialBlur_spinvar;
   double radialBlur_zoomvar;

   /* Precalculate these values for waves */
   double waves_dx2;
   double waves_dy2;

   /* If disc2 is used, precalculate these values */
   double disc2_sinadd;
   double disc2_cosadd;
   double disc2_timespi;

   /* If supershape is used, precalculate these values */
   double supershape_pm_4;
   double supershape_pneg1_n1;

   /* function pointers for faster iterations */
   int num_active_vars;
   double active_var_weights[flam3_nvariations];
   //flam3_iterator varFunc[flam3_nvariations];
   int varFunc[flam3_nvariations];

} flam3_xform;

typedef struct {
   char flame_name[flam3_name_len+1]; /* 64 chars plus a null */
   double time;
   int interpolation;
   int interpolation_type;
   int palette_interpolation;
   int num_xforms;
   int final_xform_index;
   int final_xform_enable;
   flam3_xform *xform;
   int genome_index;                   /* index into source file */
   char parent_fname[flam3_parent_fn_len];   /* base filename where parent was located */
   int symmetry;                /* 0 means none */
   flam3_palette palette;
   char *input_image;           /* preview/temporary! */
   int  palette_index;
   double brightness;           /* 1.0 = normal */
   double contrast;             /* 1.0 = normal */
   double gamma;
   int  width, height;          /* of the final image */
   int  spatial_oversample;
   double center[2];             /* of camera */
   double rot_center[2];         /* really the center */
   double rotate;                /* camera */
   double vibrancy;              /* blend between color algs (0=old,1=new) */
   double hue_rotation;          /* applies to cmap, 0-1 */
   double background[3];
   double zoom;                  /* effects ppu, sample density, scale */
   double pixels_per_unit;       /* vertically */
   double spatial_filter_radius; /* radius of spatial filter */
   int spatial_filter_select; /* selected spatial filter */
//   double (*spatial_filter_func)(double); /* spatial filter kernel function */
//   double spatial_filter_support; /* size of standard kernel for specific function */
   double sample_density;        /* samples per pixel (not bucket) */
   /* in order to motion blur more accurately we compute the logs of the
   sample density many times and average the results. */
   /* nbatches is the number of times the buckets are filtered into
   the abucket log accumulator */
   /* ntemporal_samples is the number of time steps per batch.  this many
   interpolated control points are used per batch and accumulated */
   int nbatches;
   int ntemporal_samples;

   /* Density estimation parameters for blurring low density hits */
   double estimator;             /* Filter width for bin with one hit */
   double estimator_curve;              /* Exponent on decay function ( MAX / a^(k-1) ) */
   double estimator_minimum;         /* Minimum filter width used -
                                    forces filter to be used of at least this width on all pts */

   /* XML Edit structure */
   xmlDocPtr edits;

   /* Small-gamma linearization threshold */
   double gam_lin_thresh;

   /* for cmap_interpolated hack */
   int palette_index0;
   double hue_rotation0;
   int palette_index1;
   double hue_rotation1;
   double palette_blend;

//   double motion_exp; /* Motion blur parameter that controls how the colors are scaled */

   int temporal_filter_type; /* Temporal filters */
   double temporal_filter_width, temporal_filter_exp;


} flam3_genome;


/* xform manipulation */
void flam3_add_xforms(flam3_genome *cp, int num_to_add, int interp_padding);
void flam3_delete_xform(flam3_genome *thiscp, int idx_to_delete);
void flam3_copy(flam3_genome *dest, flam3_genome *src);
void flam3_copyx(flam3_genome *dest, flam3_genome *src, int num_std, int num_final);
void flam3_copy_params(flam3_xform *dest, flam3_xform *src, int varn);

void flam3_create_xform_distrib(flam3_genome *cp, unsigned short *xform_distrib);

/* samples is array nsamples*4 long of x,y,color triples.
   using (samples[0], samples[1]) as starting XY point and
   (samples[2], samples[3]) as starting color coordinate,
   perform fuse iterations and throw them away, then perform
   nsamples iterations and save them in the samples array */
int flam3_iterate(flam3_genome *g, int nsamples, int fuse, double *samples,
                     unsigned short *xform_distrib, randctx *rc);

/* genomes is array ngenomes long, with times set and in ascending order.
   interpolate to the requested time and return in result */
void flam3_interpolate(flam3_genome *genomes, int ngenomes, double time, flam3_genome *result);

/* barycentric coordinates in c */
void flam3_interpolate_n(flam3_genome *result, int ncp, flam3_genome *cpi, double *c);

/* print genome to given file with extra_attributes if not NULL */
void flam3_print(FILE *f, flam3_genome *g, char *extra_attributes, int print_edits);
char *flam3_print_to_string(flam3_genome *cp);

/* ivars is a list of variations to use, or flam3_variation_random     */
/* ivars_n is the number of values in ivars to select from.            */
/* sym is either a symmetry group or 0 meaning random or no symmetry   */
/* spec_xforms specifies the number of xforms to use, setting to 0 makes the number random. */
void flam3_random(flam3_genome *g, int *ivars, int ivars_n, int sym, int spec_xforms);

/* return NULL in case of error */
flam3_genome *flam3_parse(char *s, int *ncps);
flam3_genome *flam3_parse_xml2(char *s, char *fn, int default_flag, int *ncps);
flam3_genome *flam3_parse_from_file(FILE *f, char *fn, int default_flag, int *ncps);

void flam3_add_symmetry(flam3_genome *g, int sym);
void flam3_parse_hexformat_colors(char *colstr, flam3_genome *cp, int numcolors, int chan);

void flam3_estimate_bounding_box(flam3_genome *g, double eps, int nsamples,
             double *bmin, double *bmax, randctx *rc);
void flam3_rotate(flam3_genome *g, double angle); /* angle in degrees */
void flam3_align(flam3_genome *dst, flam3_genome *src, int nsrc);
void establish_asymmetric_refangles(flam3_genome *cp, int ncps);

double flam3_dimension(flam3_genome *g, int ntries, int clip_to_camera);
double flam3_lyapunov(flam3_genome *g, int ntries);

void flam3_apply_template(flam3_genome *cp, flam3_genome *templ);

int flam3_count_nthreads(void);

typedef struct {
//   double         temporal_filter_radius;
   double         pixel_aspect_ratio;    /* width over height of each pixel */
   flam3_genome  *genomes;
   int            ngenomes;
   int            verbose;
   int            bits;
   int            bytes_per_channel;
   double         time;
   int            (*progress)(void *, double, int, double);
   void          *progress_parameter;
   randctx       rc;
   int           nthreads;
} flam3_frame;


#define flam3_field_both  0
#define flam3_field_even  1
#define flam3_field_odd   2

/* out is pixel array with stride of out_width.
   pixels are rgb or rgba if nchan is 3 or 4. */
void flam3_render(flam3_frame *f, void *out, int out_width, int field, int nchan, int transp, stat_struct *stats);

double flam3_render_memory_required(flam3_frame *f);


double flam3_random01();
double flam3_random11();
int flam3_random_bit();

/* ISAAC random numbers */
double flam3_random_isaac_01(randctx *);
double flam3_random_isaac_11(randctx *);
int flam3_random_isaac_bit(randctx *);

void flam3_init_frame(flam3_frame *f);

/* External memory helpers */
void *flam3_malloc(size_t size);
void flam3_free(void *ptr);

/* AE Plugin helper functions */
size_t flam3_size_flattened_genome(flam3_genome *cp);
void flam3_flatten_genome(flam3_genome *cp, void *buf);
void flam3_unflatten_genome(void *buf, flam3_genome *cp);

void flam3_srandom();

/* Spatial filter kernels */
#define flam3_gaussian_kernel 0
#define flam3_hermite_kernel 1
#define flam3_box_kernel 2
#define flam3_triangle_kernel 3
#define flam3_bell_kernel 4
#define flam3_b_spline_kernel 5
#define flam3_lanczos3_kernel 6
#define flam3_lanczos2_kernel 7
#define flam3_mitchell_kernel 8
#define flam3_blackman_kernel 9
#define flam3_catrom_kernel 10
#define flam3_hamming_kernel 11
#define flam3_hanning_kernel 12
#define flam3_quadratic_kernel 13

/* Temporal filters */
#define flam3_temporal_box 0
#define flam3_temporal_gaussian 1
#define flam3_temporal_exp 2

#endif
