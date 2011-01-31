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

#include "private.h"
#include "img.h"
#include "config.h"
#include <limits.h>
#include <math.h>
#include <stdint.h>

#ifdef HAVE_LIBPTHREAD
#include <pthread.h>
#endif

#ifdef __APPLE__
#include <mach/mach.h>
#include <mach/mach_error.h>
#endif

#ifdef WIN32
#define WINVER 0x0500
#include <windows.h>
#endif


char *flam3_version() {

//  if (strcmp(SVN_REV, "exported"))
//    return VERSION "." SVN_REV;
  return VERSION;
}


#define SUB_BATCH_SIZE     10000
#define CHOOSE_XFORM_GRAIN 10000

#define random_distrib(v) ((v)[random()%vlen(v)])

#define badvalue(x) (((x)!=(x))||((x)>1e10)||((x)<-1e10))


typedef struct {

   double tx,ty; /* Starting coordinates */

   double precalc_atan, precalc_sina;  /* Precalculated, if needed */
   double precalc_cosa, precalc_sqrt;
   double precalc_atanyx;

   flam3_xform *xform; /* For the important values */

   /* Output Coords */

   double p0, p1;

   /* Pointer to the isaac RNG state */
   randctx *rc;

} flam3_iter_helper;



/* Variation functions */
static void var0_linear(void *, double);
static void var1_sinusoidal(void *, double);
static void var2_spherical(void *, double);
static void var3_swirl(void *, double);
static void var4_horseshoe(void *, double);
static void var5_polar(void *, double);
static void var6_handkerchief(void *, double);
static void var7_heart(void *, double);
static void var8_disc(void *, double);
static void var9_spiral(void *, double);
static void var10_hyperbolic(void *, double);
static void var11_diamond(void *, double);
static void var12_ex(void *, double);
static void var13_julia(void *, double);
static void var14_bent(void *, double);
static void var15_waves(void *, double);
static void var16_fisheye(void *, double);
static void var17_popcorn(void *, double);
static void var18_exponential(void *, double);
static void var19_power(void *, double);
static void var20_cosine(void *, double);
static void var21_rings(void *, double);
static void var22_fan(void *, double);
static void var23_blob(void *, double);
static void var24_pdj(void *, double);
static void var25_fan2(void *, double);
static void var26_rings2(void *, double);
static void var27_eyefish(void *, double);
static void var28_bubble(void *, double);
static void var29_cylinder(void *, double);
static void var30_perspective(void *, double);
static void var31_noise(void *, double);
static void var32_juliaN_generic(void *, double);
static void var33_juliaScope_generic(void *, double);
static void var34_blur(void *, double);
static void var35_gaussian(void *, double);
static void var36_radial_blur(void *, double);
static void var37_pie(void *, double);
static void var38_ngon(void *, double);
/*static void var39_image(void *, double);*/
static void var39_curl(void *, double);
static void var40_rectangles(void *, double);
static void var41_arch(void *helper, double weight);
static void var42_tangent(void *helper, double weight);
static void var43_square(void *helper, double weight);
static void var44_rays(void *helper, double weight);
static void var45_blade(void *helper, double weight);
static void var46_secant2(void *helper, double weight);
static void var47_twintrian(void *helper, double weight);
static void var48_cross(void *helper, double weight);
static void var49_disc2(void *helper, double weight);
//static void var49_amw(void *helper, double weight);
static void var50_supershape(void *helper, double weight);
static void var51_flower(void *helper, double weight);
static void var52_conic(void *helper, double weight);
static void var53_parabola(void *helper, double weight);
//static void var54_split(void *helper, double weight);
//static void var55_move(void *helper, double weight);

/* Precalculation functions */
static void perspective_precalc(flam3_xform *xf);
static void juliaN_precalc(flam3_xform *xf);
static void juliaScope_precalc(flam3_xform *xf);
static void radial_blur_precalc(flam3_xform *xf);
static void waves_precalc(flam3_xform *xf);
static void disc2_precalc(flam3_xform *xf);
static void supershape_precalc(flam3_xform *xf);

static int id_matrix(double s[3][2]);
static void copy_matrix(double to[3][2], double from[3][2]);
static void convert_linear_to_polar(flam3_genome *cp, int ncps, int xfi, int cflag, double cxang[4][2], double cxmag[4][2], double cxtrn[4][2]);
static void interp_and_convert_back(double *c, int ncps, int xfi, double cxang[4][2], double cxmag[4][2], double cxtrn[4][2],double store_array[3][2]);
void prepare_xform_fn_ptrs(flam3_genome *, randctx *);
static void initialize_xforms(flam3_genome *thiscp, int start_here);
static void parse_flame_element(xmlNode *);
//static void parse_image_element(xmlNode *);
static int apply_xform(flam3_genome *cp, int fn, double *p, double *q, randctx *rc);


void decode64( char *instring, char *outstring );
void encode64( FILE *infile, FILE *outfile, int linesize);
void decodeblock64 (unsigned char in[4], unsigned char out[3]);
void encodeblock64 (unsigned char in[3], unsigned char out[4], int len);
void b64decode(char* instr, char *outstr);

/*
 * VARIATION FUNCTIONS
 * must be of the form void (void *, double)
 */
static void var0_linear (void *helper, double weight) {
   /* linear */
   /* nx = tx;
      ny = ty;
      p[0] += v * nx;
      p[1] += v * ny; */

   flam3_iter_helper *f = (flam3_iter_helper *)helper;

   f->p0 += weight * f->tx;
   f->p1 += weight * f->ty;
}

static void var1_sinusoidal (void *helper, double weight) {
   /* sinusoidal */
   /* nx = sin(tx);
      ny = sin(ty);
      p[0] += v * nx;
      p[1] += v * ny; */

   flam3_iter_helper *f = (flam3_iter_helper *)helper;

   f->p0 += weight * sin(f->tx);
   f->p1 += weight * sin(f->ty);
}

static void var2_spherical (void *helper, double weight) {
   /* spherical */
   /* double r2 = tx * tx + ty * ty + 1e-6;
      nx = tx / r2;
      ny = ty / r2;
      p[0] += v * nx;
      p[1] += v * ny; */

   flam3_iter_helper *f = (flam3_iter_helper *)helper;
   double r2 = weight / ( (f->tx * f->tx) + (f->ty * f->ty) + EPS);

   f->p0 += r2 * f->tx;
   f->p1 += r2 * f->ty;
}

static void var3_swirl (void *helper, double weight) {
   /* swirl */
   /* double r2 = tx * tx + ty * ty;    /k here is fun
      double c1 = sin(r2);
      double c2 = cos(r2);
      nx = c1 * tx - c2 * ty;
      ny = c2 * tx + c1 * ty;
      p[0] += v * nx;
      p[1] += v * ny; */

   flam3_iter_helper *f = (flam3_iter_helper *)helper;
   double r2 = f->tx*f->tx + f->ty*f->ty;
   double c1 = sin(r2);
   double c2 = cos(r2);
   double nx = c1 * f->tx - c2 * f->ty;
   double ny = c2 * f->tx + c1 * f->ty;

   f->p0 += weight * nx;
   f->p1 += weight * ny;
}

static void var4_horseshoe (void *helper, double weight) {
   /* horseshoe */
   /* a = atan2(tx, ty);
      c1 = sin(a);
      c2 = cos(a);
      nx = c1 * tx - c2 * ty;
      ny = c2 * tx + c1 * ty;
      p[0] += v * nx;
      p[1] += v * ny;  */

   flam3_iter_helper *f = (flam3_iter_helper *)helper;

   double r = weight / (f->precalc_sqrt + EPS);

   f->p0 += (f->tx - f->ty) * (f->tx + f->ty) * r;
   f->p1 += 2.0 * f->tx * f->ty * r;
}

static void var5_polar (void *helper, double weight) {
   /* polar */
   /* nx = atan2(tx, ty) / M_PI;
      ny = sqrt(tx * tx + ty * ty) - 1.0;
      p[0] += v * nx;
      p[1] += v * ny; */

   flam3_iter_helper *f = (flam3_iter_helper *)helper;
   double nx = f->precalc_atan * M_1_PI;
   double ny = f->precalc_sqrt - 1.0;

   f->p0 += weight * nx;
   f->p1 += weight * ny;
}

static void var6_handkerchief (void *helper, double weight) {
   /* folded handkerchief */
   /* a = atan2(tx, ty);
      r = sqrt(tx*tx + ty*ty);
      p[0] += v * sin(a+r) * r;
      p[1] += v * cos(a-r) * r; */

   flam3_iter_helper *f = (flam3_iter_helper *)helper;
   double a = f->precalc_atan;
   double r = f->precalc_sqrt;

   f->p0 += weight * r * sin(a+r);
   f->p1 += weight * r * cos(a-r);
}

static void var7_heart (void *helper, double weight) {
   /* heart */
   /* a = atan2(tx, ty);
      r = sqrt(tx*tx + ty*ty);
      a *= r;
      p[0] += v * sin(a) * r;
      p[1] += v * cos(a) * -r; */

   flam3_iter_helper *f = (flam3_iter_helper *)helper;
   double a = f->precalc_sqrt * f->precalc_atan;
   double r = weight * f->precalc_sqrt;

   f->p0 += r * sin(a);
   f->p1 += (-r) * cos(a);
}

static void var8_disc (void *helper, double weight) {
   /* disc */
   /* nx = tx * M_PI;
      ny = ty * M_PI;
      a = atan2(nx, ny);
      r = sqrt(nx*nx + ny*ny);
      p[0] += v * sin(r) * a / M_PI;
      p[1] += v * cos(r) * a / M_PI; */

   flam3_iter_helper *f = (flam3_iter_helper *)helper;
   double a = f->precalc_atan * M_1_PI;
   double r = M_PI * f->precalc_sqrt;

   f->p0 += weight * sin(r) * a;
   f->p1 += weight * cos(r) * a;
}

static void var9_spiral (void *helper, double weight) {
   /* spiral */
   /* a = atan2(tx, ty);
      r = sqrt(tx*tx + ty*ty) + 1e-6;
      p[0] += v * (cos(a) + sin(r)) / r;
      p[1] += v * (sin(a) - cos(r)) / r; */

   flam3_iter_helper *f = (flam3_iter_helper *)helper;
   double r = f->precalc_sqrt + EPS;
   double r1 = weight/r;

   f->p0 += r1 * (f->precalc_cosa + sin(r));
   f->p1 += r1 * (f->precalc_sina - cos(r));
}

static void var10_hyperbolic (void *helper, double weight) {
   /* hyperbolic */
   /* a = atan2(tx, ty);
      r = sqrt(tx*tx + ty*ty) + 1e-6;
      p[0] += v * sin(a) / r;
      p[1] += v * cos(a) * r; */

   flam3_iter_helper *f = (flam3_iter_helper *)helper;
   double r = f->precalc_sqrt + EPS;

   f->p0 += weight * f->precalc_sina / r;
   f->p1 += weight * f->precalc_cosa * r;
}

static void var11_diamond (void *helper, double weight) {
   /* diamond */
   /* a = atan2(tx, ty);
      r = sqrt(tx*tx + ty*ty);
      p[0] += v * sin(a) * cos(r);
      p[1] += v * cos(a) * sin(r); */

   flam3_iter_helper *f = (flam3_iter_helper *)helper;
   double r = f->precalc_sqrt;

   f->p0 += weight * f->precalc_sina * cos(r);
   f->p1 += weight * f->precalc_cosa * sin(r);
}

static void var12_ex (void *helper, double weight) {
   /* ex */
   /* a = atan2(tx, ty);
      r = sqrt(tx*tx + ty*ty);
      n0 = sin(a+r);
      n1 = cos(a-r);
      m0 = n0 * n0 * n0 * r;
      m1 = n1 * n1 * n1 * r;
      p[0] += v * (m0 + m1);
      p[1] += v * (m0 - m1); */

   flam3_iter_helper *f = (flam3_iter_helper *)helper;
   double a = f->precalc_atan;
   double r = f->precalc_sqrt;

   double n0 = sin(a+r);
   double n1 = cos(a-r);

   double m0 = n0 * n0 * n0 * r;
   double m1 = n1 * n1 * n1 * r;

   f->p0 += weight * (m0 + m1);
   f->p1 += weight * (m0 - m1);
}

static void var13_julia (void *helper, double weight) {
   /* julia */
   /* a = atan2(tx, ty)/2.0;
      if (flam3_random_bit()) a += M_PI;
      r = pow(tx*tx + ty*ty, 0.25);
      nx = r * cos(a);
      ny = r * sin(a);
      p[0] += v * nx;
      p[1] += v * ny; */

   flam3_iter_helper *f = (flam3_iter_helper *)helper;
   double r;
   double a = 0.5 * f->precalc_atan;

   if (flam3_random_isaac_bit(f->rc)) //(flam3_random_bit())
      a += M_PI;

   r = weight * sqrt(sqrt(f->tx * f->tx + f->ty * f->ty));

   f->p0 += r * cos(a);
   f->p1 += r * sin(a);
}

static void var14_bent (void *helper, double weight) {
   /* bent */
   /* nx = tx;
      ny = ty;
      if (nx < 0.0) nx = nx * 2.0;
      if (ny < 0.0) ny = ny / 2.0;
      p[0] += v * nx;
      p[1] += v * ny; */

   flam3_iter_helper *f = (flam3_iter_helper *)helper;
   double nx = f->tx;
   double ny = f->ty;

   if (nx < 0.0)
      nx = nx * 2.0;
   if (ny < 0.0)
      ny = ny / 2.0;

   f->p0 += weight * nx;
   f->p1 += weight * ny;
}

static void var15_waves (void *helper, double weight) {
   /* waves */
   /* dx = coef[2][0];
      dy = coef[2][1];
      nx = tx + coef[1][0]*sin(ty/((dx*dx)+EPS));
      ny = ty + coef[1][1]*sin(tx/((dy*dy)+EPS));
      p[0] += v * nx;
      p[1] += v * ny; */

   flam3_iter_helper *f = (flam3_iter_helper *)helper;
   double c10 = f->xform->c[1][0];
   double c11 = f->xform->c[1][1];

   double nx = f->tx + c10 * sin( f->ty * f->xform->waves_dx2 );
   double ny = f->ty + c11 * sin( f->tx * f->xform->waves_dy2 );

   f->p0 += weight * nx;
   f->p1 += weight * ny;
}

static void var16_fisheye (void *helper, double weight) {
   /* fisheye */
   /* a = atan2(tx, ty);
      r = sqrt(tx*tx + ty*ty);
      r = 2 * r / (r + 1);
      nx = r * cos(a);
      ny = r * sin(a);
      p[0] += v * nx;
      p[1] += v * ny; */

   flam3_iter_helper *f = (flam3_iter_helper *)helper;
   double r = f->precalc_sqrt;

   r = 2 * weight / (r+1);

   f->p0 += r * f->ty;
   f->p1 += r * f->tx;
}

static void var17_popcorn (void *helper, double weight) {
   /* popcorn */
   /* dx = tan(3*ty);
      dy = tan(3*tx);
      nx = tx + coef[2][0] * sin(dx);
      ny = ty + coef[2][1] * sin(dy);
      p[0] += v * nx;
      p[1] += v * ny; */

   flam3_iter_helper *f = (flam3_iter_helper *)helper;
   double dx = tan(3*f->ty);
   double dy = tan(3*f->tx);

   double nx = f->tx + f->xform->c[2][0] * sin(dx);
   double ny = f->ty + f->xform->c[2][1] * sin(dy);

   f->p0 += weight * nx;
   f->p1 += weight * ny;
}

static void var18_exponential (void *helper, double weight) {
   /* exponential */
   /* dx = exp(tx-1.0);
      dy = M_PI * ty;
      nx = cos(dy) * dx;
      ny = sin(dy) * dx;
      p[0] += v * nx;
      p[1] += v * ny; */

   flam3_iter_helper *f = (flam3_iter_helper *)helper;
   double dx = weight * exp(f->tx - 1.0);
   double dy = M_PI * f->ty;

   f->p0 += dx * cos(dy);
   f->p1 += dx * sin(dy);
}

static void var19_power (void *helper, double weight) {
   /* power */
   /* a = atan2(tx, ty);
      sa = sin(a);
      r = sqrt(tx*tx + ty*ty);
      r = pow(r, sa);
      nx = r * precalc_cosa;
      ny = r * sa;
      p[0] += v * nx;
      p[1] += v * ny; */

   flam3_iter_helper *f = (flam3_iter_helper *)helper;
   double r = weight * pow(f->precalc_sqrt, f->precalc_sina);

   f->p0 += r * f->precalc_cosa;
   f->p1 += r * f->precalc_sina;
}

static void var20_cosine (void *helper, double weight) {
   /* cosine */
   /* nx = cos(tx * M_PI) * cosh(ty);
      ny = -sin(tx * M_PI) * sinh(ty);
      p[0] += v * nx;
      p[1] += v * ny; */

   flam3_iter_helper *f = (flam3_iter_helper *)helper;
   double a = f->tx * M_PI;
   double nx =  cos(a) * cosh(f->ty);
   double ny = -sin(a) * sinh(f->ty);

   f->p0 += weight * nx;
   f->p1 += weight * ny;
}

static void var21_rings (void *helper, double weight) {
   /* rings */
   /* dx = coef[2][0];
      dx = dx * dx + EPS;
      r = sqrt(tx*tx + ty*ty);
      r = fmod(r + dx, 2*dx) - dx + r*(1-dx);
      a = atan2(tx, ty);
      nx = cos(a) * r;
      ny = sin(a) * r;
      p[0] += v * nx;
      p[1] += v * ny; */

   flam3_iter_helper *f = (flam3_iter_helper *)helper;
   double dx = f->xform->c[2][0] * f->xform->c[2][0] + EPS;
   double r = f->precalc_sqrt;
   r = weight * (fmod(r+dx, 2*dx) - dx + r * (1 - dx));

   f->p0 += r * f->precalc_cosa;
   f->p1 += r * f->precalc_sina;
}

static void var22_fan (void *helper, double weight) {
   /* fan */
   /* dx = coef[2][0];
      dy = coef[2][1];
      dx = M_PI * (dx * dx + EPS);
      dx2 = dx/2;
      a = atan(tx,ty);
      r = sqrt(tx*tx + ty*ty);
      a += (fmod(a+dy, dx) > dx2) ? -dx2 : dx2;
      nx = cos(a) * r;
      ny = sin(a) * r;
      p[0] += v * nx;
      p[1] += v * ny; */

   flam3_iter_helper *f = (flam3_iter_helper *)helper;
   double dx = M_PI * (f->xform->c[2][0] * f->xform->c[2][0] + EPS);
   double dy = f->xform->c[2][1];
   double dx2 = 0.5 * dx;

   double a = f->precalc_atan;
   double r = weight * f->precalc_sqrt;

   a += (fmod(a+dy,dx) > dx2) ? -dx2 : dx2;

   f->p0 += r * cos(a);
   f->p1 += r * sin(a);
}

static void var23_blob (void *helper, double weight) {
   /* blob */
   /* a = atan2(tx, ty);
      r = sqrt(tx*tx + ty*ty);
      r = r * (bloblow + (blobhigh-bloblow) * (0.5 + 0.5 * sin(blobwaves * a)));
      nx = sin(a) * r;
      ny = cos(a) * r;

      p[0] += v * nx;
      p[1] += v * ny; */


   flam3_iter_helper *f = (flam3_iter_helper *)helper;
   double r = f->precalc_sqrt;
   double a = f->precalc_atan;
   double bdiff = f->xform->blob_high - f->xform->blob_low;

   r = r * (f->xform->blob_low +
            bdiff * (0.5 + 0.5 * sin(f->xform->blob_waves * a)));

   f->p0 += weight * f->precalc_sina * r;
   f->p1 += weight * f->precalc_cosa * r;
}

static void var24_pdj (void *helper, double weight) {
   /* pdj */
   /* nx1 = cos(pdjb * tx);
      nx2 = sin(pdjc * tx);
      ny1 = sin(pdja * ty);
      ny2 = cos(pdjd * ty);

      p[0] += v * (ny1 - nx1);
      p[1] += v * (nx2 - ny2); */

   flam3_iter_helper *f = (flam3_iter_helper *)helper;
   double nx1 = cos(f->xform->pdj_b * f->tx);
   double nx2 = sin(f->xform->pdj_c * f->tx);
   double ny1 = sin(f->xform->pdj_a * f->ty);
   double ny2 = cos(f->xform->pdj_d * f->ty);

   f->p0 += weight * (ny1 - nx1);
   f->p1 += weight * (nx2 - ny2);
}

static void var25_fan2 (void *helper, double weight) {
   /* fan2 */
   /* a = precalc_atan;
      r = precalc_sqrt;

      dy = fan2y;
      dx = M_PI * (fan2x * fan2x + EPS);
      dx2 = dx / 2.0;

      t = a + dy - dx * (int)((a + dy)/dx);

      if (t > dx2)
         a = a - dx2;
      else
         a = a + dx2;

      nx = sin(a) * r;
      ny = cos(a) * r;

      p[0] += v * nx;
      p[1] += v * ny; */

   flam3_iter_helper *f = (flam3_iter_helper *)helper;

   double dy = f->xform->fan2_y;
   double dx = M_PI * (f->xform->fan2_x * f->xform->fan2_x + EPS);
   double dx2 = 0.5 * dx;
   double a = f->precalc_atan;
   double r = weight * f->precalc_sqrt;

   double t = a + dy - dx * (int)((a + dy)/dx);

   if (t>dx2)
      a = a-dx2;
   else
      a = a+dx2;

   f->p0 += r * sin(a);
   f->p1 += r * cos(a);
}

static void var26_rings2 (void *helper, double weight) {
   /* rings2 */
   /* r = precalc_sqrt;
      dx = rings2val * rings2val + EPS;
      r += dx - 2.0*dx*(int)((r + dx)/(2.0 * dx)) - dx + r * (1.0-dx);
      nx = precalc_sina * r;
      ny = precalc_cosa * r;
      p[0] += v * nx;
      p[1] += v * ny; */

   flam3_iter_helper *f = (flam3_iter_helper *)helper;
   double r = f->precalc_sqrt;
   double dx = f->xform->rings2_val * f->xform->rings2_val + EPS;

   r += -2.0*dx*(int)((r+dx)/(2.0*dx)) + r * (1.0-dx);

   f->p0 += weight * f->precalc_sina * r;
   f->p1 += weight * f->precalc_cosa * r;
}

static void var27_eyefish (void *helper, double weight) {
   /* eyefish */
   /* r = 2.0 * v / (precalc_sqrt + 1.0);
      p[0] += r*tx;
      p[1] += r*ty; */

   flam3_iter_helper *f = (flam3_iter_helper *)helper;
   double r = (weight * 2.0) / (f->precalc_sqrt + 1.0);

   f->p0 += r * f->tx;
   f->p1 += r * f->ty;
}

static void var28_bubble (void *helper, double weight) {
   /* bubble */

   flam3_iter_helper *f = (flam3_iter_helper *)helper;
   double r = weight / (0.25 * (f->tx*f->tx + f->ty*f->ty) + 1);

  f->p0 += r * f->tx;
  f->p1 += r * f->ty;
}

static void var29_cylinder (void *helper, double weight) {
   /* cylinder (01/06) */
   flam3_iter_helper *f = (flam3_iter_helper *)helper;

   f->p0 += weight * sin(f->tx);
   f->p1 += weight * f->ty;
}

static void var30_perspective (void *helper, double weight) {
   /* perspective (01/06) */
   flam3_iter_helper *f = (flam3_iter_helper *)helper;
   double t = 1.0 / (f->xform->perspective_dist - f->ty * f->xform->persp_vsin + EPS);

   f->p0 += weight * f->xform->perspective_dist * f->tx * t;
   f->p1 += weight * f->xform->persp_vfcos * f->ty * t;
}

static void var31_noise (void *helper, double weight) {
   /* noise (03/06) */
   flam3_iter_helper *f = (flam3_iter_helper *)helper;
   double tmpr, sinr, cosr, r;

   tmpr = flam3_random_isaac_01(f->rc) * 2 * M_PI;
   sinr = sin(tmpr);
   cosr = cos(tmpr);

   r = weight * flam3_random_isaac_01(f->rc);

   f->p0 += f->tx * r * cosr;
   f->p1 += f->ty * r * sinr;
}

static void var32_juliaN_generic (void *helper, double weight) {
   /* juliaN (03/06) */
   flam3_iter_helper *f = (flam3_iter_helper *)helper;

   int t_rnd = (int)((f->xform->juliaN_rN)*flam3_random_isaac_01(f->rc));

   double tmpr = (f->precalc_atanyx + 2 * M_PI * t_rnd) / f->xform->juliaN_power;

   double sina = sin(tmpr);
   double cosa = cos(tmpr);

   double r = weight * pow(f->tx*f->tx + f->ty*f->ty, f->xform->juliaN_cn);

   f->p0 += r * cosa;
   f->p1 += r * sina;
}

static void var33_juliaScope_generic (void *helper, double weight) {
   /* juliaScope (03/06) */
   flam3_iter_helper *f = (flam3_iter_helper *)helper;

   int t_rnd = (int)((f->xform->juliaScope_rN) * flam3_random_isaac_01(f->rc));

   double tmpr, r;
   double sina, cosa;

   if ((t_rnd & 1) == 0)
      tmpr = (2 * M_PI * t_rnd + f->precalc_atanyx) / f->xform->juliaScope_power;
   else
      tmpr = (2 * M_PI * t_rnd - f->precalc_atanyx) / f->xform->juliaScope_power;

   sina = sin(tmpr);
   cosa = cos(tmpr);

   r = weight * pow(f->tx*f->tx + f->ty*f->ty, f->xform->juliaScope_cn);

   f->p0 += r * cosa;
   f->p1 += r * sina;
}

static void var34_blur (void *helper, double weight) {
   /* blur (03/06) */
   flam3_iter_helper *f = (flam3_iter_helper *)helper;
   double tmpr, sinr, cosr, r;

   tmpr = flam3_random_isaac_01(f->rc) * 2 * M_PI;
   sinr = sin(tmpr);
   cosr = cos(tmpr);

   r = weight * flam3_random_isaac_01(f->rc);

   f->p0 += r * cosr;
   f->p1 += r * sinr;
}

static void var35_gaussian (void *helper, double weight) {
   /* gaussian (09/06) */
   flam3_iter_helper *f = (flam3_iter_helper *)helper;
   double ang, r, sina, cosa;

   ang = flam3_random_isaac_01(f->rc) * 2 * M_PI;

   sina = sin(ang);
   cosa = cos(ang);

   r = weight * ( flam3_random_isaac_01(f->rc) + flam3_random_isaac_01(f->rc)
                   + flam3_random_isaac_01(f->rc) + flam3_random_isaac_01(f->rc) - 2.0 );

   f->p0 += r * cosa;
   f->p1 += r * sina;
}

static void var36_radial_blur (void *helper, double weight) {
   /* radial blur (09/06) */
   /* removed random storage 6/07 */
   flam3_iter_helper *f = (flam3_iter_helper *)helper;
   double rndG, ra, rz, tmpa, sa, ca;

   /* Get pseudo-gaussian */
   rndG = weight * (flam3_random_isaac_01(f->rc) + flam3_random_isaac_01(f->rc)
                   + flam3_random_isaac_01(f->rc) + flam3_random_isaac_01(f->rc) - 2.0);

   /* Calculate angle & zoom */
   ra = f->precalc_sqrt;
   tmpa = f->precalc_atanyx + f->xform->radialBlur_spinvar*rndG;
   sa = sin(tmpa);
   ca = cos(tmpa);
   rz = f->xform->radialBlur_zoomvar * rndG - 1;

   f->p0 += ra * ca + rz * f->tx;
   f->p1 += ra * sa + rz * f->ty;
}

static void var37_pie(void *helper, double weight) {
   /* pie by Joel Faber (June 2006) */
   flam3_iter_helper *f = (flam3_iter_helper *) helper;
   double a, r;
   int sl;

   sl = (int) (flam3_random_isaac_01(f->rc) * f->xform->pie_slices + 0.5);
   a = f->xform->pie_rotation +
       2.0 * M_PI * (sl + flam3_random_isaac_01(f->rc) * f->xform->pie_thickness) / f->xform->pie_slices;
   r = weight * flam3_random_isaac_01(f->rc);

   f->p0 += r * cos(a);
   f->p1 += r * sin(a);
}

static void var38_ngon(void *helper, double weight) {
   /* ngon by Joel Faber (09/06) */
   flam3_iter_helper *f = (flam3_iter_helper *) helper;
   double r_factor,theta,phi,b, amp;

   r_factor = pow(f->tx*f->tx + f->ty*f->ty, f->xform->ngon_power/2.0);

   theta = f->precalc_atanyx;
   b = 2*M_PI/f->xform->ngon_sides;

   phi = theta - (b*floor(theta/b));
   if (phi > b/2)
      phi -= b;

   amp = f->xform->ngon_corners * (1.0 / (cos(phi) + EPS) - 1.0) + f->xform->ngon_circle;
   amp /= (r_factor + EPS);

   f->p0 += weight * f->tx * amp;
   f->p1 += weight * f->ty * amp;
}

# if 0
static void var39_image(void *helper, double weight) {
   int lo=1, hi=255;
   int m,i,j;
   double r = flam3_random_isaac_01(f->rc);
   double xfrac,yfrac;
   double xmult,ymult;
   double pixelSize;

   flam3_iter_helper *f = (flam3_iter_helper *) helper;
   flam3_image_store *is = f->xform->image_storage;

   while (lo + 1 < hi) {
      m = (lo + hi) / 2;
      if (r>= is->intensity_weight[m])
         lo = m;
      else
         hi = m;
   }

   if (r >= is->intensity_weight[lo])
      i = hi;
   else
      i = lo;

   j = floor(flam3_random_isaac_01(f->rc) * is->bin_size[i]);

   xfrac = (double)(is->rowcols[is->bin_offset[i] + 2*j])/is->width;
   yfrac = (double)(is->rowcols[is->bin_offset[i] + 2*j+1])/is->height;

   if (is->width > is->height) {
      pixelSize = weight * 2.0 / is->width;
      xmult = weight * 2.0;
      ymult = weight * 2.0 * is->height / is->width;
   } else {
      pixelSize = weight * 2.0 / is->height;
      ymult = weight * 2.0;
      xmult = weight * 2.0 * is->width / is->height;
   }

   f->p0 += xmult * xfrac - xmult / 2 + flam3_random_isaac_01(f->rc)*pixelSize;
   f->p1 += ymult * yfrac - ymult / 2 + flam3_random_isaac_01(f->rc)*pixelSize;
}

#endif

static void var39_curl(void *helper, double weight)
{
    flam3_iter_helper *f = (flam3_iter_helper *) helper;

    double re = 1.0 + f->xform->curl_c1 * f->tx + f->xform->curl_c2 * (f->tx * f->tx - f->ty * f->ty);
    double im = f->xform->curl_c1 * f->ty + 2.0 * f->xform->curl_c2 * f->tx * f->ty;

    double r = weight / (re*re + im*im);

    f->p0 += (f->tx * re + f->ty * im) * r;
    f->p1 += (f->ty * re - f->tx * im) * r;
}

static void var40_rectangles(void *helper, double weight)
{
    flam3_iter_helper *f = (flam3_iter_helper *) helper;

    if (f->xform->rectangles_x==0)
       f->p0 += weight * f->tx;
    else
       f->p0 += weight * ((2 * floor(f->tx / f->xform->rectangles_x) + 1) * f->xform->rectangles_x - f->tx);

    if (f->xform->rectangles_y==0)
       f->p1 += weight * f->ty;
    else
       f->p1 += weight * ((2 * floor(f->ty / f->xform->rectangles_y) + 1) * f->xform->rectangles_y - f->ty);

}

static void var41_arch(void *helper, double weight)
{
   /* Z+ variation Jan 07
   procedure TXForm.Arch;
   var
     sinr, cosr: double;
   begin
     SinCos(random * vars[29]*pi, sinr, cosr);
     FPx := FPx + sinr*vars[29];
     FPy := FPy + sqr(sinr)/cosr*vars[29];
   end;
   */

   flam3_iter_helper *f = (flam3_iter_helper *) helper;

   double ang = flam3_random_isaac_01(f->rc) * weight * M_PI;
   double sinr = sin(ang);
   double cosr = cos(ang);

   f->p0 += weight * sinr;
   f->p1 += weight * (sinr*sinr)/cosr;

}

static void var42_tangent(void *helper, double weight)
{
   /* Z+ variation Jan 07
   procedure TXForm.Tangent;
   begin
     FPx := FPx + vars[30] * (sin(FTx)/cos(FTy));
     FPy := FPy + vars[30] * (sin(FTy)/cos(FTy));
   end;
   */

   flam3_iter_helper *f = (flam3_iter_helper *) helper;

   f->p0 += weight * sin(f->tx)/cos(f->ty);
   f->p1 += weight * sin(f->ty)/cos(f->ty);

}

static void var43_square(void *helper, double weight)
{
   /* Z+ variation Jan 07
   procedure TXForm.SquareBlur;
   begin
     FPx := FPx + vars[31] * (random - 0.5);
     FPy := FPy + vars[31] * (random - 0.5);
   end;
   */

   flam3_iter_helper *f = (flam3_iter_helper *) helper;

   f->p0 += weight * (flam3_random_isaac_01(f->rc) - 0.5);
   f->p1 += weight * (flam3_random_isaac_01(f->rc) - 0.5);

}

static void var44_rays(void *helper, double weight)
{
   /* Z+ variation Jan 07
   procedure TXForm.Rays;
   var
     r, sinr, cosr, tgr: double;
   begin
     SinCos(random * vars[32]*pi, sinr, cosr);
     r := vars[32] / (sqr(FTx) + sqr(FTy) + EPS);
     tgr := sinr/cosr;
     FPx := FPx + tgr * (cos(FTx)*vars[32]) * r;
     FPy := FPy + tgr * (sin(FTy)*vars[32]) * r;
   end;
   */

   flam3_iter_helper *f = (flam3_iter_helper *) helper;

   double ang = weight * flam3_random_isaac_01(f->rc) * M_PI;
   double r = weight / (f->tx*f->tx + f->ty*f->ty + EPS);
   double tanr = weight * tan(ang) * r;


   f->p0 += tanr * cos(f->tx);
   f->p1 += tanr * sin(f->ty);

}

static void var45_blade(void *helper, double weight)
{
   /* Z+ variation Jan 07
   procedure TXForm.Blade;
   var
     r, sinr, cosr: double;
   begin
     r := sqrt(sqr(FTx) + sqr(FTy))*vars[33];
     SinCos(r*random, sinr, cosr);
     FPx := FPx + vars[33] * FTx * (cosr + sinr);
     FPy := FPy + vars[33] * FTx * (cosr - sinr);
   end;
   */

   flam3_iter_helper *f = (flam3_iter_helper *) helper;

   double r = flam3_random_isaac_01(f->rc) * weight * f->precalc_sqrt;
   double sinr = sin(r);
   double cosr = cos(r);

   f->p0 += weight * f->tx * (cosr + sinr);
   f->p1 += weight * f->tx * (cosr - sinr);

}

static void var46_secant2(void *helper, double weight)
{
   /* Intended as a 'fixed' version of secant */

   flam3_iter_helper *f = (flam3_iter_helper *) helper;

   double r = weight * f->precalc_sqrt;
   double cr = cos(r);
   double icr = 1.0/cr;

   f->p0 += weight * f->tx;

   if (cr<0)
      f->p1 += weight*(icr + 1);
   else
      f->p1 += weight*(icr - 1);
}

static void var47_twintrian(void *helper, double weight)
{
   /* Z+ variation Jan 07
   procedure TXForm.TwinTrian;
   var
     r, diff, sinr, cosr: double;
   begin
     r := sqrt(sqr(FTx) + sqr(FTy))*vars[35];
     SinCos(r*random, sinr, cosr);
     diff := Math.Log10(sinr*sinr)+cosr;
     FPx := FPx + vars[35] * FTx * diff;
     FPy := FPy + vars[35] * FTx * (diff - (sinr*pi));
   end;
   */

   flam3_iter_helper *f = (flam3_iter_helper *) helper;

   double r = flam3_random_isaac_01(f->rc) * weight * f->precalc_sqrt;
   double sinr = sin(r);
   double cosr = cos(r);
   double diff = log10(sinr*sinr)+cosr;

   f->p0 += weight * f->tx * diff;
   f->p1 += weight * f->tx * (diff - sinr*M_PI);

}

static void var48_cross(void *helper, double weight)
{
   /* Z+ variation Jan 07
   procedure TXForm.Cross;
   var
     r: double;
   begin
     r := vars[36]*sqrt(1/(sqr(sqr(FTx)-sqr(FTy))+EPS));
     FPx := FPx + FTx * r;
     FPy := FPy + FTy * r;
   end;
   */

   flam3_iter_helper *f = (flam3_iter_helper *) helper;

   double s = f->tx*f->tx - f->ty*f->ty;
   double r = weight * sqrt(1.0 / (s*s+EPS));

   f->p0 += f->tx * r;
   f->p1 += f->ty * r;

}

//static void var49_amw(void *helper, double weight)
//{
   ///* Z+ variation Jan 07
   //precalc:
   //k := PI*PI/(AMP+1E-300);
   //procedure TVariationAMWaves.CalcFunction;
   //begin
     //FPx^ := FPx^ + FTx^*vvar;
     //FPy^ := FPy^ + (FTy^*Cos(k*FTx^*FTy^))*vvar;
   //*/

   //flam3_iter_helper *f = (flam3_iter_helper *) helper;

   //double k=M_PI * M_PI / (f->xform->amw_amp + 1e-300);

   ////fprintf(stdout,"%f, %f, %f\n",FAngle, k, h);

   //f->p0 += f->tx * weight;
   //f->p1 += (f->ty*cos(k*f->tx*f->ty))*weight;

//}

static void var49_disc2(void *helper, double weight)
{
   /* Z+ variation Jan 07
   c := vvar/PI;
   k := rot*PI;
     sinadd := Sin(add);
     cosadd := Cos(add);
   cosadd := cosadd - 1;
   if (add > 2*PI) then begin
     cosadd := cosadd * (1 + add - 2*PI);
     sinadd := sinadd * (1 + add - 2*PI)
   end
   else if (add < -2*PI) then begin
     cosadd := cosadd * (1 + add + 2*PI);
     sinadd := sinadd * (1 + add + 2*PI)
   end
   end;
   procedure TVariationDisc2.CalcFunction;
   var
     r, sinr, cosr: extended;
   begin
     SinCos(k * (FTx^+FTy^), sinr, cosr);   //rot*PI
     r := c * arctan2(FTx^, FTy^); //vvar/PI
     FPx^ := FPx^ + (sinr + cosadd) * r;
     FPy^ := FPy^ + (cosr + sinadd) * r;
   */

   flam3_iter_helper *f = (flam3_iter_helper *) helper;

   double r,t,sinr, cosr;

   t = f->xform->disc2_timespi * (f->tx + f->ty);
   sinr = sin(t);
   cosr = cos(t);
   r = weight * f->precalc_atan / M_PI;

   f->p0 += (sinr + f->xform->disc2_cosadd) * r;
   f->p1 += (cosr + f->xform->disc2_sinadd) * r;

}

static void var50_supershape(void *helper, double weight) {

   flam3_iter_helper *f = (flam3_iter_helper *) helper;

   double theta;
   double t1,t2,r;
   double myrnd;

   theta = f->xform->supershape_pm_4 * f->precalc_atanyx + M_PI_4;

   t1 = fabs(cos(theta));
   t1 = pow(t1,f->xform->supershape_n2);

   t2 = fabs(sin(theta));
   t2 = pow(t2,f->xform->supershape_n3);

   myrnd = f->xform->supershape_rnd;

   r = weight * ( (myrnd*flam3_random_isaac_01(f->rc) + (1.0-myrnd)*f->precalc_sqrt) - f->xform->supershape_holes)
      * pow(t1+t2,f->xform->supershape_pneg1_n1) / f->precalc_sqrt;

   f->p0 += r * f->tx;
   f->p1 += r * f->ty;
}

static void var51_flower(void *helper, double weight) {
    /* cyberxaos, 4/2007 */
    /*   theta := arctan2(FTy^, FTx^);
         r := (random-holes)*cos(petals*theta);
         FPx^ := FPx^ + vvar*r*cos(theta);
         FPy^ := FPy^ + vvar*r*sin(theta);*/

    flam3_iter_helper *f = (flam3_iter_helper *)helper;
    double theta = f->precalc_atanyx;
    double r = weight * (flam3_random_isaac_01(f->rc) - f->xform->flower_holes) *
                    cos(f->xform->flower_petals*theta);

    f->p0 += r * cos(theta);
    f->p1 += r * sin(theta);
}

static void var52_conic(void *helper, double weight) {
    /* cyberxaos, 4/2007 */
    /*   theta := arctan2(FTy^, FTx^);
         r :=  (random - holes)*((eccentricity)/(1+eccentricity*cos(theta)));
         FPx^ := FPx^ + vvar*r*cos(theta);
         FPy^ := FPy^ + vvar*r*sin(theta); */

    flam3_iter_helper *f = (flam3_iter_helper *)helper;
    double theta = f->precalc_atanyx;
    double r = weight * (flam3_random_isaac_01(f->rc) - f->xform->conic_holes) *
                    f->xform->conic_eccen / (1 + f->xform->conic_eccen*cos(theta));

    f->p0 += r * cos(theta);
    f->p1 += r * sin(theta);
}

static void var53_parabola(void *helper, double weight) {
    /* cyberxaos, 4/2007 */
    /*   r := sqrt(sqr(FTx^) + sqr(FTy^));
         FPx^ := FPx^ + parabola_height*vvar*sin(r)*sin(r)*random;
         FPy^ := FPy^ + parabola_width*vvar*cos(r)*random; */

    flam3_iter_helper *f = (flam3_iter_helper *)helper;
    double r = f->precalc_sqrt;

    f->p0 += f->xform->parabola_height * weight * sin(r) * sin(r) * flam3_random_isaac_01(f->rc);
    f->p1 += f->xform->parabola_width * weight * cos(r) * flam3_random_isaac_01(f->rc);

}

# if 0
static void var54_split (void *helper, double weight) {

   flam3_iter_helper *f = (flam3_iter_helper *)helper;
   double sgnx,sgny;

   if (cos(f->tx*f->xform->split_xsize*M_PI)*(1-f->xform->split_shift) +
         sin(f->tx*f->xform->split_xsize*M_PI)*(f->xform->split_shift) >= 0)
      sgny = 1.0;
   else
      sgny = -1.0;

   if (cos(f->ty*f->xform->split_ysize*M_PI)*(1-f->xform->split_shift) +
         sin(f->ty*f->xform->split_ysize*M_PI)*(f->xform->split_shift) >= 0)
      sgnx = 1.0;
   else
      sgnx = -1.0;

   f->p0 += weight * f->tx * sgnx;
   f->p1 += weight * f->ty * sgny;

}

static void var55_move (void *helper, double weight) {

   flam3_iter_helper *f = (flam3_iter_helper *)helper;

   f->p0 += weight * f->xform->move_x;
   f->p1 += weight * f->xform->move_y;
}
#endif

static void perspective_precalc(flam3_xform *xf) {
   double ang = xf->perspective_angle * M_PI / 2.0;
   xf->persp_vsin = sin(ang);
   xf->persp_vfcos = xf->perspective_dist * cos(ang);
}

static void juliaN_precalc(flam3_xform *xf) {
   xf->juliaN_rN = fabs(xf->juliaN_power);
   xf->juliaN_cn = xf->juliaN_dist / (double)xf->juliaN_power / 2.0;
}

static void juliaScope_precalc(flam3_xform *xf) {
   xf->juliaScope_rN = fabs(xf->juliaScope_power);
   xf->juliaScope_cn = xf->juliaScope_dist / (double)xf->juliaScope_power / 2.0;
}

static void radial_blur_precalc(flam3_xform *xf) {
   xf->radialBlur_spinvar = sin(xf->radialBlur_angle * M_PI / 2);
   xf->radialBlur_zoomvar = cos(xf->radialBlur_angle * M_PI / 2);
}

static void waves_precalc(flam3_xform *xf) {
   double dx = xf->c[2][0];
   double dy = xf->c[2][1];

   xf->waves_dx2 = 1.0/(dx * dx + EPS);
   xf->waves_dy2 = 1.0/(dy * dy + EPS);
}

static void disc2_precalc(flam3_xform *xf) {
   double add = xf->disc2_twist;
   double k;

   xf->disc2_timespi = xf->disc2_rot * M_PI;

   xf->disc2_sinadd = sin(add);
   xf->disc2_cosadd = cos(add) - 1;

   if (add > 2 * M_PI) {
      k = (1 + add - 2*M_PI);
      xf->disc2_cosadd *= k;
      xf->disc2_sinadd *= k;
   }

   if (add < -2 * M_PI) {
      k = (1 + add + 2*M_PI);
      xf->disc2_cosadd *= k;
      xf->disc2_sinadd *= k;
   }

}

static void supershape_precalc(flam3_xform *xf) {
   xf->supershape_pm_4 = xf->supershape_m / 4.0;
   xf->supershape_pneg1_n1 = -1.0 / xf->supershape_n1;
}

void prepare_xform_fn_ptrs(flam3_genome *cp, randctx *rc) {

   double d;
   int i,j,totnum;

   /* Loop over valid xforms */
   for (i = 0; i < cp->num_xforms; i++) {
      d = cp->xform[i].density;
      if (d < 0.0) {
         fprintf(stderr, "xform %d weight must be non-negative, not %g.\n",i,d);
         exit(1);
      }

      if (i != cp->final_xform_index && d == 0.0)
         continue;

      totnum = 0;
      cp->xform[i].precalc_angles_flag=0;
      cp->xform[i].precalc_atan_xy_flag=0;
      cp->xform[i].precalc_atan_yx_flag=0;
      cp->xform[i].precalc_sqrt_flag=0;

      for (j = 0; j < flam3_nvariations; j++) {

         if (cp->xform[i].var[j]!=0) {

            cp->xform[i].varFunc[totnum] = j;
            cp->xform[i].active_var_weights[totnum] = cp->xform[i].var[j];

            if (j==VAR_HORSESHOE) {
               cp->xform[i].precalc_sqrt_flag=1;
            } else if (j==VAR_POLAR) {
               cp->xform[i].precalc_atan_xy_flag=1;
               cp->xform[i].precalc_sqrt_flag=1;
            } else if (j==VAR_HANDKERCHIEF) {
               cp->xform[i].precalc_atan_xy_flag=1;
               cp->xform[i].precalc_sqrt_flag=1;
            } else if (j==VAR_HEART) {
               cp->xform[i].precalc_atan_xy_flag=1;
               cp->xform[i].precalc_sqrt_flag=1;
            } else if (j==VAR_DISC) {
               cp->xform[i].precalc_atan_xy_flag=1;
               cp->xform[i].precalc_sqrt_flag=1;
            } else if (j==VAR_SPIRAL) {
               cp->xform[i].precalc_atan_xy_flag=1;
               cp->xform[i].precalc_angles_flag=1;
               cp->xform[i].precalc_sqrt_flag=1;
            } else if (j==VAR_HYPERBOLIC) {
               cp->xform[i].precalc_atan_xy_flag=1;
               cp->xform[i].precalc_angles_flag=1;
               cp->xform[i].precalc_sqrt_flag=1;
            } else if (j==VAR_DIAMOND) {
               cp->xform[i].precalc_atan_xy_flag=1;
               cp->xform[i].precalc_angles_flag=1;
               cp->xform[i].precalc_sqrt_flag=1;
            } else if (j==VAR_EX) {
               cp->xform[i].precalc_atan_xy_flag=1;
               cp->xform[i].precalc_sqrt_flag=1;
            } else if (j==VAR_JULIA) {
               cp->xform[i].precalc_atan_xy_flag=1;
            } else if (j==VAR_FISHEYE) {
               cp->xform[i].precalc_sqrt_flag=1;
            } else if (j==VAR_POWER) {
               cp->xform[i].precalc_atan_xy_flag=1;
               cp->xform[i].precalc_angles_flag=1;
               cp->xform[i].precalc_sqrt_flag=1;
            } else if (j==VAR_RINGS) {
               cp->xform[i].precalc_atan_xy_flag=1;
               cp->xform[i].precalc_angles_flag=1;
               cp->xform[i].precalc_sqrt_flag=1;
            } else if (j==VAR_FAN) {
               cp->xform[i].precalc_atan_xy_flag=1;
               cp->xform[i].precalc_sqrt_flag=1;
            } else if (j==VAR_BLOB) {
               cp->xform[i].precalc_atan_xy_flag=1;
               cp->xform[i].precalc_angles_flag=1;
               cp->xform[i].precalc_sqrt_flag=1;
            } else if (j==VAR_FAN2) {
               cp->xform[i].precalc_atan_xy_flag=1;
               cp->xform[i].precalc_sqrt_flag=1;
            } else if (j==VAR_RINGS2) {
               cp->xform[i].precalc_atan_xy_flag=1;
               cp->xform[i].precalc_angles_flag=1;
               cp->xform[i].precalc_sqrt_flag=1;
            } else if (j==VAR_EYEFISH) {
               cp->xform[i].precalc_sqrt_flag=1;
            } else if (j==VAR_JULIAN) {
               cp->xform[i].precalc_atan_yx_flag=1;
            } else if (j==VAR_JULIASCOPE) {
               cp->xform[i].precalc_atan_yx_flag=1;
            } else if (j==VAR_RADIAL_BLUR) {
               cp->xform[i].precalc_atan_yx_flag=1;
               cp->xform[i].precalc_sqrt_flag=1;
            } else if (j==VAR_NGON) {
               cp->xform[i].precalc_atan_yx_flag=1;
            } else if (j==VAR_BLADE) {
               cp->xform[i].precalc_sqrt_flag=1;
            } else if (j==VAR_SECANT2) {
               cp->xform[i].precalc_sqrt_flag=1;
            } else if (j==VAR_TWINTRIAN) {
               cp->xform[i].precalc_sqrt_flag=1;
            } else if (j==VAR_DISC2) {
               cp->xform[i].precalc_atan_xy_flag=1;
            } else if (j==VAR_SUPER_SHAPE) {
               cp->xform[i].precalc_sqrt_flag=1;
               cp->xform[i].precalc_atan_yx_flag=1;
            } else if (j==VAR_FLOWER) {
               cp->xform[i].precalc_atan_yx_flag=1;
            } else if (j==VAR_CONIC) {
               cp->xform[i].precalc_atan_yx_flag=1;
            } else if (j==VAR_PARABOLA) {
               cp->xform[i].precalc_sqrt_flag=1;
            }

            totnum++;
         }
      }

      cp->xform[i].num_active_vars = totnum;

   }
}

FILE *cout = NULL;
void write_color(double c) {
    if (NULL == cout) {
   cout = fopen("cout.txt", "w");
    }
    fprintf(cout, "%.30f\n", c);
}

void flam3_create_xform_distrib(flam3_genome *cp, unsigned short *xform_distrib) {

   /* Xform distrib is a preallocated array of CHOOSE_XFORM_GRAIN chars */
   /* address of array is passed in, contents are modified              */
   double t,r,dr;
   int i,j;

     dr = 0.0;
      for (i = 0; i < cp->num_xforms; i++) {
         double d = cp->xform[i].density;

         /* Ignore final xform density */
         if (i == cp->final_xform_index)
            continue;

         if (d < 0.0) {
            fprintf(stderr, "xform weight must be non-negative, not %g.\n", d);
            exit(1);
         }
         dr += d;
      }
      if (dr == 0.0) {
         fprintf(stderr, "cannot iterate empty flame.\n");
         exit(1);
      }
      dr = dr / CHOOSE_XFORM_GRAIN;

      j = 0;
      t = cp->xform[0].density;
      r = 0.0;
      for (i = 0; i < CHOOSE_XFORM_GRAIN; i++) {
         while (r >= t) {
            j++;
            t += cp->xform[j].density;
         }
         xform_distrib[i] = j;
         r += dr;
      }
}

/*
 * run the function system described by CP forward N generations.  store
 * the N resulting 4-vectors in SAMPLES.  the initial point is passed in
 * SAMPLES[0..3].  ignore the first FUSE iterations.
 */


int flam3_iterate(flam3_genome *cp, int n, int fuse,  double *samples, unsigned short *xform_distrib, randctx *rc) {
   int i;
   double p[4], q[4];
   int consec = 0;
   int badvals = 0;

   p[0] = samples[0];
   p[1] = samples[1];
   p[2] = samples[2];
   p[3] = samples[3];

   /* Perform precalculations */
   for (i=0;i<cp->num_xforms;i++) {
      perspective_precalc(&(cp->xform[i]));
      juliaN_precalc(&(cp->xform[i]));
      juliaScope_precalc(&(cp->xform[i]));
      radial_blur_precalc(&(cp->xform[i]));
      waves_precalc(&(cp->xform[i]));
      disc2_precalc(&(cp->xform[i]));
      supershape_precalc(&(cp->xform[i]));
   }

   for (i = -4*fuse; i < 4*n; i+=4) {
       int fn = xform_distrib[((unsigned)irand(rc)) % CHOOSE_XFORM_GRAIN];

      if (1) {
         if (apply_xform(cp, fn, p, q, rc)>0) {
            //fprintf(stdout,"bad result from xform %d\n",fn);
            consec ++;
            badvals ++;
            if (consec<5) {
               p[0] = q[0];
               p[1] = q[1];
               p[2] = q[2];
               p[3] = q[3];
               i -= 4;
               continue;
            } else
               consec = 0;
         } else
            consec = 0;
      } else {
         apply_xform(cp, fn, p, q, rc);
      }

      p[0] = q[0];
      p[1] = q[1];
      p[2] = q[2];
      p[3] = q[3];

      if (cp->final_xform_enable == 1) {
         apply_xform(cp, cp->final_xform_index, p, q, rc);
      }

      /* if fuse over, store it */
      if (i >= 0) {
         samples[i] = q[0];
         samples[i+1] = q[1];
         samples[i+2] = q[2];
         samples[i+3] = q[3];
      }
   }

   return(badvals);
}

static int apply_xform(flam3_genome *cp, int fn, double *p, double *q, randctx *rc)
{
   flam3_iter_helper f;
   int var_n;
   double next_color,s,s1;

   f.rc = rc;

   s = cp->xform[fn].symmetry;
   s1 = 0.5 - 0.5 * s;

   next_color = (p[2] + cp->xform[fn].color[0]) * s1 + s * p[2];
   q[2] = next_color;
   q[3] = (p[3] + cp->xform[fn].color[1]) * s1 + s * p[3];

   f.tx = cp->xform[fn].c[0][0] * p[0] + cp->xform[fn].c[1][0] * p[1] + cp->xform[fn].c[2][0];
   f.ty = cp->xform[fn].c[0][1] * p[0] + cp->xform[fn].c[1][1] * p[1] + cp->xform[fn].c[2][1];

   /* Check to see if we can precalculate any parts */
   /* Precalculate atanxy, sin, cos */
   if (cp->xform[fn].precalc_atan_xy_flag > 0) {
      f.precalc_atan = atan2(f.tx,f.ty);
      if (cp->xform[fn].precalc_angles_flag > 0) {
         f.precalc_sina = sin(f.precalc_atan);
         f.precalc_cosa = cos(f.precalc_atan);
      }
   }

   /* Precalc atanyx */
   if (cp->xform[fn].precalc_atan_yx_flag > 0) {
      f.precalc_atanyx = atan2(f.ty,f.tx);
   }

   /* Check for sqrt */
   if (cp->xform[fn].precalc_sqrt_flag > 0) {
      f.precalc_sqrt = sqrt(f.tx*f.tx + f.ty*f.ty);
   }

   f.p0 = 0.0;
   f.p1 = 0.0;
   f.xform = &(cp->xform[fn]);


   for (var_n=0; var_n < cp->xform[fn].num_active_vars; var_n++) {
//      (*cp->xform[fn].varFunc[var_n])(&f, cp->xform[fn].active_var_weights[var_n]);
//   }
//

      switch (cp->xform[fn].varFunc[var_n]) {

         case (VAR_LINEAR):
            var0_linear(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_SINUSOIDAL):
                var1_sinusoidal(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_SPHERICAL):
                var2_spherical(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_SWIRL):
                var3_swirl(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_HORSESHOE):
                var4_horseshoe(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_POLAR):
                var5_polar(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_HANDKERCHIEF):
                var6_handkerchief(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_HEART):
                var7_heart(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_DISC):
                var8_disc(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_SPIRAL):
                var9_spiral(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_HYPERBOLIC):
                var10_hyperbolic(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_DIAMOND):
                var11_diamond(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_EX):
                var12_ex(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_JULIA):
                var13_julia(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_BENT):
                var14_bent(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_WAVES):
                var15_waves(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_FISHEYE):
                var16_fisheye(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_POPCORN):
                var17_popcorn(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_EXPONENTIAL):
                var18_exponential(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_POWER):
                var19_power(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_COSINE):
                var20_cosine(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_RINGS):
                var21_rings(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_FAN):
                var22_fan(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_BLOB):
                var23_blob(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_PDJ):
                var24_pdj(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_FAN2):
                var25_fan2(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_RINGS2):
                var26_rings2(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_EYEFISH):
                var27_eyefish(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_BUBBLE):
                var28_bubble(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_CYLINDER):
                var29_cylinder(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_PERSPECTIVE):
                var30_perspective(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_NOISE):
                var31_noise(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_JULIAN):
                var32_juliaN_generic(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_JULIASCOPE):
                var33_juliaScope_generic(&f, cp->xform[fn].active_var_weights[var_n]);break;
         case (VAR_BLUR):
                var34_blur(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_GAUSSIAN_BLUR):
                var35_gaussian(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_RADIAL_BLUR):
                var36_radial_blur(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_PIE):
                var37_pie(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_NGON):
                var38_ngon(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_CURL):
                var39_curl(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_RECTANGLES):
                var40_rectangles(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_ARCH):
                var41_arch(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_TANGENT):
                var42_tangent(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_SQUARE):
                var43_square(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_RAYS):
                var44_rays(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_BLADE):
                var45_blade(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_SECANT2):
                var46_secant2(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_TWINTRIAN):
                var47_twintrian(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_CROSS):
                var48_cross(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_DISC2):
                var49_disc2(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_SUPER_SHAPE):
                var50_supershape(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_FLOWER):
                var51_flower(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_CONIC):
                var52_conic(&f, cp->xform[fn].active_var_weights[var_n]); break;
         case (VAR_PARABOLA):
                var53_parabola(&f, cp->xform[fn].active_var_weights[var_n]); break;
//         case (VAR_SPLIT):
//                var54_split(&f, cp->xform[fn].active_var_weights[var_n]); break;
//         case (VAR_MOVE):
//                var55_move(&f, cp->xform[fn].active_var_weights[var_n]); break;
      }
   }
   /* apply the post transform */
   if (!id_matrix(cp->xform[fn].post)) {
      q[0] = cp->xform[fn].post[0][0] * f.p0 + cp->xform[fn].post[1][0] * f.p1 + cp->xform[fn].post[2][0];
      q[1] = cp->xform[fn].post[0][1] * f.p0 + cp->xform[fn].post[1][1] * f.p1 + cp->xform[fn].post[2][1];
   } else {
      q[0] = f.p0;
      q[1] = f.p1;
   }

   if (badvalue(q[0]) || badvalue(q[1])) {
      q[0] = flam3_random_isaac_11(rc);
      q[1] = flam3_random_isaac_11(rc);
      return(1);
   } else
      return(0);

}
/* correlation dimension, after clint sprott.
   computes slope of the correlation sum at a size scale
   the order of 2% the size of the attractor or the camera. */
double flam3_dimension(flam3_genome *cp, int ntries, int clip_to_camera) {
  double fd;
  double *hist;
  double bmin[2];
  double bmax[2];
  double d2max;
  int lp;
  long int default_isaac_seed = (long int)time(0);
  randctx rc;
  int i, n1=0, n2=0, got, nclipped;

  /* Set up the isaac rng */
  for (lp = 0; lp < RANDSIZ; lp++)
     rc.randrsl[lp] = default_isaac_seed;

  irandinit(&rc,1);

  if (ntries < 2) ntries = 3000*1000;

  if (clip_to_camera) {
    double scale, ppux, corner0, corner1;
    scale = pow(2.0, cp->zoom);
    ppux = cp->pixels_per_unit * scale;
    corner0 = cp->center[0] - cp->width / ppux / 2.0;
    corner1 = cp->center[1] - cp->height / ppux / 2.0;
    bmin[0] = corner0;
    bmin[1] = corner1;
    bmax[0] = corner0 + cp->width  / ppux;
    bmax[1] = corner1 + cp->height / ppux;
  } else {
    flam3_estimate_bounding_box(cp, 0.0, 0, bmin, bmax, &rc);
  }

  d2max =
    (bmax[0] - bmin[0]) * (bmax[0] - bmin[0]) +
    (bmax[1] - bmin[1]) * (bmax[1] - bmin[1]);

  //  fprintf(stderr, "d2max=%g %g %g %g %g\n", d2max,
  //  bmin[0], bmin[1], bmax[0], bmax[1]);

  hist = malloc(2 * ntries * sizeof(double));

  got = 0;
  nclipped = 0;
  while (got < 2*ntries) {
    double subb[4*SUB_BATCH_SIZE];
    int i4, clipped;
    unsigned short xform_distrib[CHOOSE_XFORM_GRAIN];
    subb[0] = flam3_random_isaac_11(&rc);
    subb[1] = flam3_random_isaac_11(&rc);
    subb[2] = 0.0;
    subb[3] = 0.0;
    prepare_xform_fn_ptrs(cp,&rc);
    flam3_create_xform_distrib(cp,xform_distrib);
    flam3_iterate(cp, SUB_BATCH_SIZE, 20, subb, xform_distrib, &rc);
    i4 = 0;
    for (i = 0; i < SUB_BATCH_SIZE; i++) {
      if (got == 2*ntries) break;
      clipped = clip_to_camera &&
   ((subb[i4] < bmin[0]) ||
    (subb[i4+1] < bmin[1]) ||
    (subb[i4] > bmax[0]) ||
    (subb[i4+1] > bmax[1]));
      if (!clipped) {
   hist[got] = subb[i4];
   hist[got+1] = subb[i4+1];
   got += 2;
      } else {
   nclipped++;
   if (nclipped > 10 * ntries) {
       fprintf(stderr, "warning: too much clipping, "
          "flam3_dimension giving up.\n");
       return sqrt(-1.0);
   }
      }
      i4 += 4;
    }
  }
  if (0)
    fprintf(stderr, "cliprate=%g\n", nclipped/(ntries+(double)nclipped));

  for (i = 0; i < ntries; i++) {
    int ri;
    double dx, dy, d2;
    double tx, ty;

    tx = hist[2*i];
    ty = hist[2*i+1];

    do {
      ri = 2 * (random() % ntries);
    } while (ri == i);

    dx = hist[ri] - tx;
    dy = hist[ri+1] - ty;
    d2 = dx*dx + dy*dy;
    if (d2 < 0.004 * d2max) n2++;
    if (d2 < 0.00004 * d2max) n1++;
  }

  fd = 0.434294 * log(n2 / (n1 - 0.5));

  if (0)
    fprintf(stderr, "n1=%d n2=%d\n", n1, n2);

  free(hist);
  return fd;
}

double flam3_lyapunov(flam3_genome *cp, int ntries) {
  double p[4];
  double x, y;
  double xn, yn;
  double xn2, yn2;
  double dx, dy, r;
  double eps = 1e-5;
  int i;
  double sum = 0.0;
  unsigned short xform_distrib[CHOOSE_XFORM_GRAIN];

  int lp;
  long int default_isaac_seed = (long int)time(0);
  randctx rc;

  /* Set up the isaac rng */
  for (lp = 0; lp < RANDSIZ; lp++)
     rc.randrsl[lp] = default_isaac_seed;

  irandinit(&rc,1);


  if (ntries < 1) ntries = 10000;

  for (i = 0; i < ntries; i++) {
    x = flam3_random_isaac_11(&rc);
    y = flam3_random_isaac_11(&rc);

    p[0] = x;
    p[1] = y;
    p[2] = 0.0;
    p[3] = 0.0;

    // get into the attractor
    prepare_xform_fn_ptrs(cp,&rc);
    flam3_create_xform_distrib(cp,xform_distrib);
    flam3_iterate(cp, 1, 20+(random()%10), p, xform_distrib, &rc);

    x = p[0];
    y = p[1];

    // take one deterministic step
    srandom(i);

    prepare_xform_fn_ptrs(cp,&rc);
    flam3_create_xform_distrib(cp,xform_distrib);
    flam3_iterate(cp, 1, 0, p, xform_distrib, &rc);

    xn = p[0];
    yn = p[1];

    do {
      dx = flam3_random_isaac_11(&rc);
      dy = flam3_random_isaac_11(&rc);
      r = sqrt(dx * dx + dy * dy);
    } while (r == 0.0);
    dx /= r;
    dy /= r;

    dx *= eps;
    dy *= eps;

    p[0] = x + dx;
    p[1] = y + dy;
    p[2] = 0.0;

    // take the same step but with eps
    srandom(i);
    prepare_xform_fn_ptrs(cp,&rc);
    flam3_create_xform_distrib(cp,xform_distrib);
    flam3_iterate(cp, 1, 0, p, xform_distrib, &rc);

    xn2 = p[0];
    yn2 = p[1];

    r = sqrt((xn-xn2)*(xn-xn2) + (yn-yn2)*(yn-yn2));

    sum += log(r/eps);
  }
  return sum/(log(2.0)*ntries);
}

/* args must be non-overlapping */
static void mult_matrix(double s1[2][2], double s2[2][2], double d[2][2]) {
   d[0][0] = s1[0][0] * s2[0][0] + s1[1][0] * s2[0][1];
   d[1][0] = s1[0][0] * s2[1][0] + s1[1][0] * s2[1][1];
   d[0][1] = s1[0][1] * s2[0][0] + s1[1][1] * s2[0][1];
   d[1][1] = s1[0][1] * s2[1][0] + s1[1][1] * s2[1][1];
}

/* BY is angle in degrees */
void flam3_rotate(flam3_genome *cp, double by) {
   int i;
   for (i = 0; i < cp->num_xforms; i++) {
      double r[2][2];
      double T[2][2];
      double U[2][2];
      double dtheta = by * 2.0 * M_PI / 360.0;

		/* Don't rotate symmetric xforms unless necessary */
		/* 1 means that all rotating xform[i]'s have symmetry>0 */
		/* or that it's OK to check */

//      if (symm[i]==1) {
//	      if (cp->xform[i].symmetry > 0.0) continue;
//		}

      if (cp->xform[i].symmetry > 0.0)
      	continue;

      if (cp->xform[i].padding == 1) {
         if (cp->interpolation_type == flam3_inttype_compat) {
            /* gen 202 era flam3 did not rotate padded xforms */
            continue;
         } else if (cp->interpolation_type == flam3_inttype_older) {
            /* not sure if 198 era flam3 rotated padded xforms */
            continue;
         } else if (cp->interpolation_type == flam3_inttype_linear) {
            /* don't rotate for prettier symsings */
            continue;
         } else if (cp->interpolation_type == flam3_inttype_log) {
            /* Current flam3: what do we prefer? */
            //continue;
         }
      }

      /* Do NOT rotate final xforms */
      if (cp->final_xform_enable==1 && cp->final_xform_index==i)
         continue;

      r[1][1] = r[0][0] = cos(dtheta);
      r[0][1] = sin(dtheta);
      r[1][0] = -r[0][1];
      T[0][0] = cp->xform[i].c[0][0];
      T[1][0] = cp->xform[i].c[1][0];
      T[0][1] = cp->xform[i].c[0][1];
      T[1][1] = cp->xform[i].c[1][1];
      mult_matrix(r, T, U);
      cp->xform[i].c[0][0] = U[0][0];
      cp->xform[i].c[1][0] = U[1][0];
      cp->xform[i].c[0][1] = U[0][1];
      cp->xform[i].c[1][1] = U[1][1];
   }
}

static double det_matrix(double s[2][2]) {
   return s[0][0] * s[1][1] - s[0][1] * s[1][0];
}

static int id_matrix(double s[3][2]) {
  return
    (s[0][0] == 1.0) &&
    (s[0][1] == 0.0) &&
    (s[1][0] == 0.0) &&
    (s[1][1] == 1.0) &&
    (s[2][0] == 0.0) &&
    (s[2][1] == 0.0);
}

static void copy_matrix(double to[3][2], double from[3][2]) {

    to[0][0] = from[0][0];
    to[0][1] = from[0][1];
    to[1][0] = from[1][0];
    to[1][1] = from[1][1];
    to[2][0] = from[2][0];
    to[2][1] = from[2][1];
}


static void clear_matrix(double m[3][2]) {
   m[0][0] = 0.0;
   m[0][1] = 0.0;
   m[1][0] = 0.0;
   m[1][1] = 0.0;
   m[2][0] = 0.0;
   m[2][1] = 0.0;
}

/* element-wise linear */
static void sum_matrix(double s, double m1[3][2], double m2[3][2]) {

   m2[0][0] += s * m1[0][0];
   m2[0][1] += s * m1[0][1];
   m2[1][0] += s * m1[1][0];
   m2[1][1] += s * m1[1][1];
   m2[2][0] += s * m1[2][0];
   m2[2][1] += s * m1[2][1];

}


static void interpolate_cmap(double cmap[256][3], double blend,
              int index0, double hue0, int index1, double hue1) {
  double p0[256][3];
  double p1[256][3];
  int i, j;

  flam3_get_palette(index0, p0, hue0);
  flam3_get_palette(index1, p1, hue1);

  for (i = 0; i < 256; i++) {
    double t[3], s[3];
    rgb2hsv(p0[i], s);
    rgb2hsv(p1[i], t);
    for (j = 0; j < 3; j++)
      t[j] = ((1.0-blend) * s[j]) + (blend * t[j]);
    hsv2rgb(t, cmap[i]);
  }
}

/*   tTime2 = tTime * tTime
     tTime3 = tTime2 * tTime
tpFinal = (((-tp1 + 3 * tp2 - 3 * tp3 + tp4) * tTime3)
                + ((2 * tp1 - 5 * tp2 + 4 * tp3 - tp4) * tTime2)
                + ((-tp1 + tp3) * tTime)
                + (2 * tp2))
                / 2
*/
static void interpolate_catmull_rom(flam3_genome cps[], double t, flam3_genome *result) {
    double t2 = t * t;
    double t3 = t2 * t;
    double cmc[4];

    cmc[0] = (2*t2 - t - t3) / 2;
    cmc[1] = (3*t3 - 5*t2 + 2) / 2;
    cmc[2] = (4*t2 - 3*t3 + t) / 2;
    cmc[3] = (t3 - t2) / 2;

    flam3_interpolate_n(result, 4, cps, cmc);
}


#define INTERP(x)  do { result->x = 0.0; \
   for (k = 0; k < ncp; k++) result->x += c[k] * cpi[k].x; } while(0)

#define INTERI(x)  do { double tt = 0.0; \
   for (k = 0; k < ncp; k++) tt += c[k] * cpi[k].x; \
   result->x = (int)rint(tt); } while(0)


/* all cpi and result must be aligned (have the same number of xforms,
   and have final xform in the same slot) */
void flam3_interpolate_n(flam3_genome *result, int ncp,
          flam3_genome *cpi, double *c) {
    int i, j, k;
    if (flam3_palette_interpolation_hsv ==
   cpi[0].palette_interpolation) {
   for (i = 0; i < 256; i++) {
       double t[3], s[3];
       s[0] = s[1] = s[2] = 0.0;
       for (k = 0; k < ncp; k++) {
      rgb2hsv(cpi[k].palette[i], t);
      for (j = 0; j < 3; j++)
          s[j] += c[k] * t[j];
       }
       hsv2rgb(s, result->palette[i]);
       for (j = 0; j < 3; j++) {
      if (result->palette[i][j] < 0.0) {
          result->palette[i][j] = 0.0;
      }
       }
   }
    } else {
   for (i = 0; i < 256; i++) {
       j = (i < (256 * c[0])) ? 0 : 1;
       for (k = 0; k < 3; k++)
      result->palette[i][k] =
          cpi[j].palette[i][k];
   }
    }

   result->palette_index = flam3_palette_random;
   result->symmetry = 0;
   result->spatial_filter_select = cpi[0].spatial_filter_select;
   result->temporal_filter_type = cpi[0].temporal_filter_type;

   result->interpolation_type = cpi[0].interpolation_type;
   INTERP(brightness);
   INTERP(contrast);
   INTERP(gamma);
   INTERP(vibrancy);
   INTERP(hue_rotation);
   INTERI(width);
   INTERI(height);
   INTERI(spatial_oversample);
   INTERP(center[0]);
   INTERP(center[1]);
   INTERP(rot_center[0]);
   INTERP(rot_center[1]);
   INTERP(background[0]);
   INTERP(background[1]);
   INTERP(background[2]);
   INTERP(pixels_per_unit);
   INTERP(spatial_filter_radius);
   INTERP(temporal_filter_exp);
   INTERP(temporal_filter_width);
   INTERP(sample_density);
   INTERP(zoom);
   INTERP(rotate);
   INTERI(nbatches);
   INTERI(ntemporal_samples);
   INTERP(estimator);
   INTERP(estimator_minimum);
   INTERP(estimator_curve);
   INTERP(gam_lin_thresh);
//   INTERP(motion_exp);

   for (i = 0; i < cpi[0].num_xforms; i++) {
      double td;
      int all_id;
      INTERP(xform[i].density);
      td = result->xform[i].density;
      result->xform[i].density = (td < 0.0) ? 0.0 : td;
      INTERP(xform[i].color[0]);
      INTERP(xform[i].color[1]);
      INTERP(xform[i].symmetry);
      INTERP(xform[i].blob_low);
      INTERP(xform[i].blob_high);
      INTERP(xform[i].blob_waves);
      INTERP(xform[i].pdj_a);
      INTERP(xform[i].pdj_b);
      INTERP(xform[i].pdj_c);
      INTERP(xform[i].pdj_d);
      INTERP(xform[i].fan2_x);
      INTERP(xform[i].fan2_y);
      INTERP(xform[i].rings2_val);
      INTERP(xform[i].perspective_angle);
      INTERP(xform[i].perspective_dist);
      INTERP(xform[i].juliaN_power);
      INTERP(xform[i].juliaN_dist);
      INTERP(xform[i].juliaScope_power);
      INTERP(xform[i].juliaScope_dist);
      INTERP(xform[i].radialBlur_angle);
      INTERP(xform[i].pie_slices);
      INTERP(xform[i].pie_rotation);
      INTERP(xform[i].pie_thickness);
      INTERP(xform[i].ngon_sides);
      INTERP(xform[i].ngon_power);
      INTERP(xform[i].ngon_circle);
      INTERP(xform[i].ngon_corners);
      INTERP(xform[i].curl_c1);
      INTERP(xform[i].curl_c2);
      INTERP(xform[i].rectangles_x);
      INTERP(xform[i].rectangles_y);
      INTERP(xform[i].amw_amp);
      INTERP(xform[i].disc2_rot);
      INTERP(xform[i].disc2_twist);
      INTERP(xform[i].supershape_rnd);
      INTERP(xform[i].supershape_m);
      INTERP(xform[i].supershape_n1);
      INTERP(xform[i].supershape_n2);
      INTERP(xform[i].supershape_n3);
      INTERP(xform[i].supershape_holes);
      INTERP(xform[i].flower_petals);
      INTERP(xform[i].flower_holes);
      INTERP(xform[i].conic_eccen);
      INTERP(xform[i].conic_holes);
      INTERP(xform[i].parabola_height);
      INTERP(xform[i].parabola_width);
//      INTERP(xform[i].split_xsize);
//      INTERP(xform[i].split_ysize);
//      INTERP(xform[i].split_shift);
//      INTERP(xform[i].move_x);
//      INTERP(xform[i].move_y);

      for (j = 0; j < flam3_nvariations; j++)
         INTERP(xform[i].var[j]);

 # if 0
      if (result->xform[i].var[39]>0) {
         imid=-1;
         for (k = 0; k < ncp; k++) {
            if (cpi[k].xform[i].image_id >= 0) {
               if (imid<0 || imid ==cpi[k].xform[i].image_id) {
                  int numcpy;
                  imid = cpi[k].xform[i].image_id;
                  result->xform[i].image_id = imid;
                  /* Copy the image data from the source */
                  result->xform[i].image_storage = (flam3_image_store *)malloc(sizeof(flam3_image_store));
                  memcpy(result->xform[i].image_storage,cpi[k].xform[i].image_storage,sizeof(flam3_image_store));
                  /* ...and the rowcols */
                  numcpy = 2 * result->xform[i].image_storage->height * result->xform[i].image_storage->width;
                  result->xform[i].image_storage->rowcols = (unsigned short *)malloc(sizeof(unsigned short)*numcpy);
                  memcpy(result->xform[i].image_storage->rowcols,cpi[k].xform[i].image_storage->rowcols,sizeof(unsigned short)*numcpy);
               } else {
                  fprintf(stderr,"Error!  Unable to interpolate between two image variations!\n");
                  exit(1);
               }
            }
         }
      }
#endif

      if (flam3_inttype_log == cpi[0].interpolation_type) {
         int col;
         double cxmag[4][2];  // XXX why only 4? should be ncp
         double cxang[4][2];
         double cxtrn[4][2];

         /* affine part */
         clear_matrix(result->xform[i].c);
         convert_linear_to_polar(cpi,ncp,i,0,cxang,cxmag,cxtrn);
         interp_and_convert_back(c, ncp, i, cxang, cxmag, cxtrn,result->xform[i].c);

         /* post part */
         all_id = 1;
         for (k=0; k<ncp; k++)
            all_id &= id_matrix(cpi[k].xform[i].post);

         clear_matrix(result->xform[i].post);
         if (all_id) {
            result->xform[i].post[0][0] = 1.0;
            result->xform[i].post[1][1] = 1.0;
         } else {
            convert_linear_to_polar(cpi,ncp,i,1,cxang,cxmag,cxtrn);
            interp_and_convert_back(c, ncp, i, cxang, cxmag, cxtrn,result->xform[i].post);
         }


         if (0) {
            fprintf(stderr,"original coefs\n");
            for (k=0;k<3;k++) {
               for (col=0;col<2;col++) {
                  fprintf(stderr,"%f ",cpi[0].xform[i].c[k][col]);
               }
            }
            fprintf(stderr,"\n");

            fprintf(stderr,"new coefs\n");
            for (k=0;k<3;k++) {
               for (col=0;col<2;col++) {
                  fprintf(stderr,"%f ",result->xform[i].c[k][col]);
               }
            }
            fprintf(stderr,"\n");

         }

      } else {

         /* Interpolate c matrix & post */
         clear_matrix(result->xform[i].c);
         clear_matrix(result->xform[i].post);
         all_id = 1;
         for (k = 0; k < ncp; k++) {
            sum_matrix(c[k], cpi[k].xform[i].c, result->xform[i].c);
            sum_matrix(c[k], cpi[k].xform[i].post, result->xform[i].post);

            all_id &= id_matrix(cpi[k].xform[i].post);

         }
         if (all_id) {
            clear_matrix(result->xform[i].post);
            result->xform[i].post[0][0] = 1.0;
            result->xform[i].post[1][1] = 1.0;
         }
      }

      /* Precalculate additional params for some variations */
//      perspective_precalc(&(result->xform[i]));
//      juliaN_precalc(&(result->xform[i]));
//      juliaScope_precalc(&(result->xform[i]));
//      radial_blur_precalc(&(result->xform[i]));
//      waves_precalc(&(result->xform[i]));
//      disc2_precalc(&(result->xform[i]));
//      supershape_precalc(&(result->xform[i]));

   }
}

void establish_asymmetric_refangles(flam3_genome *cp, int ncps) {

   int k, xfi, col;

   double cxang[4][2],d,c1[2];

   for (xfi=0; xfi<cp[0].num_xforms; xfi++) {

     /* Final xforms don't rotate regardless of their symmetry */
     if (cp[0].final_xform_enable==1 && xfi==cp[0].final_xform_index)
        continue;

     for (k=0; k<ncps;k++) {

          /* Establish the angle for each component */
          /* Should potentially functionalize */
          for (col=0;col<2;col++) {

               c1[0] = cp[k].xform[xfi].c[col][0];
               c1[1] = cp[k].xform[xfi].c[col][1];

               cxang[k][col] = atan2(c1[1],c1[0]);
          }
     }

     for (k=1; k<ncps; k++) {

          for (col=0;col<2;col++) {

               int sym0,sym1;
	       int padsymflag;

               d = cxang[k][col]-cxang[k-1][col];

               /* Adjust to avoid the -pi/pi discontinuity */
               if (d > M_PI+EPS)
               cxang[k][col] -= 2*M_PI;
               else if (d < -(M_PI-EPS) )
               cxang[k][col] += 2*M_PI;

               /* If this is an asymmetric case, store the NON-symmetric angle    */
               /* Check them pairwise and store the reference angle in the second */
               /* to avoid overwriting if asymmetric on both sides                */
               /* Depending on the interpolation type, treat padded xforms as symmetric */
               //padsymflag = (cp[k-1].interpolation_type==flam3_inttype_log);
               padsymflag = 0;

               sym0 = (cp[k-1].xform[xfi].symmetry>0 || (cp[k-1].xform[xfi].padding==1 && padsymflag));
               sym1 = (cp[k].xform[xfi].symmetry>0 || (cp[k].xform[xfi].padding==1 && padsymflag));

/*
               if (cp[k].xform[xfi].symmetry>0 && cp[k-1].xform[xfi].symmetry<=0)
                  cp[k].xform[xfi].wind[col] = cxang[k-1][col] + 2*M_PI;
               else if (cp[k].xform[xfi].symmetry<=0 && cp[k-1].xform[xfi].symmetry>0)
                  cp[k].xform[xfi].wind[col] = cxang[k][col] + 2*M_PI;
*/
               if ( sym1 && !sym0 )
                  cp[k].xform[xfi].wind[col] = cxang[k-1][col] + 2*M_PI;
               else if ( sym0 && !sym1 )
                  cp[k].xform[xfi].wind[col] = cxang[k][col] + 2*M_PI;

          }
     }
   }
}



static void convert_linear_to_polar(flam3_genome *cp, int ncps, int xfi, int cflag, double cxang[4][2], double cxmag[4][2], double cxtrn[4][2]) {

   double c1[2],d,t,refang;
   int col,k;
   int zlm[2];

   for (k=0; k<ncps;k++) {

      /* Establish the angles and magnitudes for each component */
      /* Keep translation linear */
      zlm[0]=zlm[1]=0;
      for (col=0;col<2;col++) {

         if (cflag==0) {
            c1[0] = cp[k].xform[xfi].c[col][0];
            c1[1] = cp[k].xform[xfi].c[col][1];
            t = cp[k].xform[xfi].c[2][col];
         } else {
            c1[0] = cp[k].xform[xfi].post[col][0];
            c1[1] = cp[k].xform[xfi].post[col][1];
            t = cp[k].xform[xfi].post[2][col];
         }

         cxang[k][col] = atan2(c1[1],c1[0]);
         cxmag[k][col] = sqrt(c1[0]*c1[0] + c1[1]*c1[1]);

         if (cxmag[k][col]== 0.0)
            zlm[col]=1;

         cxtrn[k][col] = t;
      }

      if (zlm[0]==1 && zlm[1]==0)
         cxang[k][0] = cxang[k][1];
      else if (zlm[0]==0 && zlm[1]==1)
         cxang[k][1] = cxang[k][0];

   }

   /* Make sure the rotation is the shorter direction around the circle */
   /* by adjusting each angle in succession, and rotate clockwise if 180 degrees */
   {
      for (col=0; col<2; col++) {
         for (k=1;k<ncps;k++) {

            /* Adjust angles differently if we have an asymmetric case */
            if (cp[k].xform[xfi].wind[col]>0 && cflag==0) {

               /* Adjust the angles to make sure that it's within wind:wind+2pi */
               refang = cp[k].xform[xfi].wind[col] - 2*M_PI;

               /* Make sure both angles are within [refang refang+2*pi] */
               while(cxang[k-1][col] < refang)
                    cxang[k-1][col] += 2*M_PI;

               while(cxang[k-1][col] > refang + 2*M_PI)
                    cxang[k-1][col] -= 2*M_PI;

               while(cxang[k][col] < refang)
                    cxang[k][col] += 2*M_PI;

               while(cxang[k][col] > refang + 2*M_PI)
                    cxang[k][col] -= 2*M_PI;

            } else {

	       /* Normal way of adjusting angles */
               d = cxang[k][col]-cxang[k-1][col];

	       /* Adjust to avoid the -pi/pi discontinuity */
	       if (d > M_PI+EPS)
                  cxang[k][col] -= 2*M_PI;
	       else if (d < -(M_PI-EPS) ) /* Forces clockwise rotation at 180 */
	          cxang[k][col] += 2*M_PI;
	    }


         }
      }
   }
}

static void interp_and_convert_back(double *c, int ncps, int xfi, double cxang[4][2], double cxmag[4][2], double cxtrn[4][2],double store_array[3][2]) {

   int i,col;

   double accang[2],accmag[2];
   double expmag;
   int accmode[2];

   accang[0] = 0.0;
   accang[1] = 0.0;
   accmag[0] = 0.0;
   accmag[1] = 0.0;

   accmode[0]=accmode[1]=0;

   /* accumulation mode defaults to logarithmic, but in special */
   /* cases we want to switch to linear accumulation            */
   for (col=0; col<2; col++) {
      for (i=0; i<ncps; i++) {
         if (log(cxmag[i][col])<-10)
            accmode[col]=1; // Mode set to linear interp
      }
   }

   for (i=0; i<ncps; i++) {
      for (col=0; col<2; col++) {

         accang[col] += c[i] * cxang[i][col];

         if (accmode[col]==0)
            accmag[col] += c[i] * log(cxmag[i][col]);
         else
            accmag[col] += c[i] * (cxmag[i][col]);

         /* translation is ready to go */
         store_array[2][col] += c[i] * cxtrn[i][col];
      }
   }

   /* Convert the angle back to rectangular */
   for (col=0;col<2;col++) {
      if (accmode[col]==0)
          expmag = exp(accmag[col]);
     else
          expmag = accmag[col];

      store_array[col][0] = expmag * cos(accang[col]);
      store_array[col][1] = expmag * sin(accang[col]);
   }

}

void flam3_align(flam3_genome *dst, flam3_genome *src, int nsrc) {
   int i, tfx, tnx, max_nx = 0, max_fx = 0;
   int already_aligned=1;
   int xf,j;
   int ii,fnd;
   double normed;

   max_nx = src[0].num_xforms - (src[0].final_xform_index >= 0);
   max_fx = src[0].final_xform_enable;

   for (i = 1; i < nsrc; i++) {
      tnx = src[i].num_xforms - (src[i].final_xform_index >= 0);
      if (max_nx != tnx) {
         already_aligned = 0;
         if (tnx > max_nx) max_nx = tnx;
      }

      tfx = src[i].final_xform_enable;
      if (max_fx != tfx) {
         already_aligned = 0;
         max_fx |= tfx;
      }
   }

   /* Pad the cps to equal xforms */
   for (i = 0; i < nsrc; i++) {
      flam3_copyx(&dst[i], &src[i], max_nx, max_fx);
   }

   /* Check to see if there's a parametric variation present in one xform   */
   /* but not in an aligned xform.  If this is the case, use the parameters */
   /* from the xform with the variation as the defaults for the blank one.  */

   /* Skip if this genome is compatibility mode */
   if (dst[i].interpolation_type == flam3_inttype_compat ||
       dst[i].interpolation_type == flam3_inttype_older)
      return;

   /* All genomes will have the same number of xforms at this point */
   /* num = max_nx + max_fx */
   for (i = 0; i<nsrc; i++) {

       for (xf = 0; xf<max_nx+max_fx; xf++) {

          /* Loop over the variations to see which of them are set to 0 */
          /* Note that there are no parametric variations < 23 */
          for (j = 23; j < flam3_nvariations; j++) {

              if (dst[i].xform[xf].var[j]==0) {

                 if (i>0) {

                    /* Check to see if the prior genome's xform is populated */
                    if (dst[i-1].xform[xf].var[j] != 0) {

                       /* Copy the prior genome's parameters and continue */
                       flam3_copy_params(&(dst[i].xform[xf]), &(dst[i-1].xform[xf]), j);
                       continue;
                    }

                 } else if (i<nsrc-1) {

                    /* Check to see if the next genome's xform is populated */
                    if (dst[i+1].xform[xf].var[j] != 0) {

                       /* Copy the next genome's parameters and continue */
                       flam3_copy_params(&(dst[i].xform[xf]), &(dst[i+1].xform[xf]), j);
                       continue;
                    }
                 }
              }
          } /* variations */

          if (dst[i].xform[xf].padding == 1 && !already_aligned) {

             /* This is a new xform.  Let's see if we can choose a better 'identity' xform. */
             /* Check the neighbors to see if any of these variations are used: */
             /* rings2, fan2, blob, perspective, julian, juliascope, ngon, curl, super_shape, split */
             /* If so, we can use a better starting point for these */

             /* Remove linear from the list */
             dst[i].xform[xf].var[0] = 0.0;

             /* Look through all of the 'companion' xforms to see if we get a match on any of these */
             fnd=0;

             /* Only do the next substitution for log interpolation */
             if (dst[i].interpolation_type == flam3_inttype_log) {

             for (ii=-1; ii<=1; ii+=2) {

                /* Skip if out of bounds */
                if (i+ii<0 || i+ii>=nsrc)
                   continue;

                /* Skip if this is also padding */
                if (dst[i+ii].xform[xf].padding==1)
                   continue;

                /* Spherical / Ngon (trumps all others due to holes)       */
                /* Interpolate these against a 180 degree rotated identity */
                /* with weight -1.                                         */
                /* Added JULIAN/JULIASCOPE to get rid of black wedges      */
                if (dst[i+ii].xform[xf].var[VAR_SPHERICAL]>0 ||
                      dst[i+ii].xform[xf].var[VAR_NGON]>0 ||
                      dst[i+ii].xform[xf].var[VAR_JULIAN]>0 ||
                      dst[i+ii].xform[xf].var[VAR_JULIASCOPE]>0) {

                   dst[i].xform[xf].var[VAR_LINEAR] = -1.0;
                   /* Set the coefs appropriately */
                   dst[i].xform[xf].c[0][0] = -1.0;
                   dst[i].xform[xf].c[0][1] = 0.0;
                   dst[i].xform[xf].c[1][0] = 0.0;
                   dst[i].xform[xf].c[1][1] = -1.0;
                   dst[i].xform[xf].c[2][0] = 0.0;
                   dst[i].xform[xf].c[2][1] = 0.0;
                   fnd=-1;
                }
             }

             }

             if (fnd==0) {

                for (ii=-1; ii<=1; ii+=2) {

                   /* Skip if out of bounds */
                   if (i+ii<0 || i+ii>=nsrc)
                      continue;

                   /* Skip if also padding */
                   if (dst[i+ii].xform[xf].padding==1)
                      continue;

                   /* Rectangles */
                   if (dst[i+ii].xform[xf].var[VAR_RECTANGLES]>0) {
                      dst[i].xform[xf].var[VAR_RECTANGLES] = 1.0;
                      dst[i].xform[xf].rectangles_x = 0.0;
                      dst[i].xform[xf].rectangles_y = 0.0;
                      fnd++;
                   }

                   /* Rings 2 */
                   if (dst[i+ii].xform[xf].var[VAR_RINGS2]>0) {
                      dst[i].xform[xf].var[VAR_RINGS2] = 1.0;
                      dst[i].xform[xf].rings2_val = 0.0;
                      fnd++;
                   }

                   /* Fan 2 */
                   if (dst[i+ii].xform[xf].var[VAR_FAN2]>0) {
                      dst[i].xform[xf].var[VAR_FAN2] = 1.0;
                      dst[i].xform[xf].fan2_x = 0.0;
                      dst[i].xform[xf].fan2_y = 0.0;
                      fnd++;
                   }

                   /* Blob */
                   if (dst[i+ii].xform[xf].var[VAR_BLOB]>0) {
                      dst[i].xform[xf].var[VAR_BLOB] = 1.0;
                      dst[i].xform[xf].blob_low = 1.0;
                      dst[i].xform[xf].blob_high = 1.0;
                      dst[i].xform[xf].blob_waves = 1.0;
                      fnd++;
                   }

                   /* Perspective */
                   if (dst[i+ii].xform[xf].var[VAR_PERSPECTIVE]>0) {
                      dst[i].xform[xf].var[VAR_PERSPECTIVE] = 1.0;
                      dst[i].xform[xf].perspective_angle = 0.0;
                      /* Keep the perspective distance as-is */
                      fnd++;
                   }

                   /* Curl */
                   if (dst[i+ii].xform[xf].var[VAR_CURL]>0) {
                      dst[i].xform[xf].var[VAR_CURL] = 1.0;
                      dst[i].xform[xf].curl_c1 = 0.0;
                      dst[i].xform[xf].curl_c2 = 0.0;
                      fnd++;
                   }

                   /* Super-Shape */
                   if (dst[i+ii].xform[xf].var[VAR_SUPER_SHAPE]>0) {
                      dst[i].xform[xf].var[VAR_SUPER_SHAPE] = 1.0;
                      /* Keep supershape_m the same */
                      dst[i].xform[xf].supershape_n1 = 2.0;
                      dst[i].xform[xf].supershape_n2 = 2.0;
                      dst[i].xform[xf].supershape_n3 = 2.0;
                      dst[i].xform[xf].supershape_rnd = 0.0;
                      dst[i].xform[xf].supershape_holes = 0.0;
                      fnd++;
                   }
                }
             }

             /* If we didn't have any matches with those, */
             /* try the affine ones, fan and rings        */
             if (fnd==0) {

                for (ii=-1; ii<=1; ii+=2) {

                   /* Skip if out of bounds */
                   if (i+ii<0 || i+ii>=nsrc)
                      continue;

                   /* Skip if also a padding xform */
                   if (dst[i+ii].xform[xf].padding==1)
                      continue;

                   /* Fan */
                   if (dst[i+ii].xform[xf].var[VAR_FAN]>0) {
                      dst[i].xform[xf].var[VAR_FAN] = 1.0;
                      fnd++;
                   }

                   /* Rings */
                   if (dst[i+ii].xform[xf].var[VAR_RINGS]>0) {
                      dst[i].xform[xf].var[VAR_RINGS] = 1.0;
                      fnd++;
                   }

                }

                if (fnd>0) {
                   /* Set the coefs appropriately */
                   dst[i].xform[xf].c[0][0] = 0.0;
                   dst[i].xform[xf].c[0][1] = 1.0;
                   dst[i].xform[xf].c[1][0] = 1.0;
                   dst[i].xform[xf].c[1][1] = 0.0;
                   dst[i].xform[xf].c[2][0] = 0.0;
                   dst[i].xform[xf].c[2][1] = 0.0;
                }
             }

             /* If we still have no matches, switch back to linear */
             if (fnd==0)

                dst[i].xform[xf].var[VAR_LINEAR] = 1.0;

             else if (fnd>0) {

                /* Otherwise, go through and normalize the weights. */
                normed = 0.0;
                for (j = 0; j < flam3_nvariations; j++)
                   normed += dst[i].xform[xf].var[j];

                for (j = 0; j < flam3_nvariations; j++)
                   dst[i].xform[xf].var[j] /= normed;

             }
          }
       } /* xforms */
   } /* genomes */

}


/*
 * create a control point that interpolates between the control points
 * passed in CPS.  CPS must be sorted by time.
 */
void flam3_interpolate(flam3_genome cps[], int ncps,
             double time, flam3_genome *result) {
   int i1, i2;
   double c[2];
   flam3_genome cpi[4];
   int cpsi,xfi;

   if (1 == ncps) {
      flam3_copy(result, &(cps[0]));
      return;
   }

   if (cps[0].time >= time) {
      i1 = 0;
      i2 = 1;
   } else if (cps[ncps - 1].time <= time) {
      i1 = ncps - 2;
      i2 = ncps - 1;
   } else {
      i1 = 0;
      while (cps[i1].time < time)
         i1++;

      i1--;
      i2 = i1 + 1;

//      if (time - cps[i1].time > -1e-7 && time - cps[i1].time < 1e-7) {
//         flam3_copy(result, &(cps[i1]));
//         return;
//      }
   }

   c[0] = (cps[i2].time - time) / (cps[i2].time - cps[i1].time);
   c[1] = 1.0 - c[0];

   memset(cpi, 0, 4*sizeof(flam3_genome));

   /* To interpolate the xforms, we will make copies of the source cps  */
   /* and ensure that they both have the same number before progressing */
   if (flam3_interpolation_linear == cps[i1].interpolation) {
       flam3_align(&cpi[0], &cps[i1], 2);

   } else {
       if (0 == i1) {
      fprintf(stderr, "error: cannot use smooth interpolation on first segment.\n");
      exit(1);
       }
       if (ncps-1 == i2) {
      fprintf(stderr, "error: cannot use smooth interpolation on last segment.\n");
      exit(1);
       }
       flam3_align(&cpi[0], &cps[i1-1], 4);
   }

   if (result->num_xforms > 0 && result->xform != NULL) {
       free(result->xform);
       result->num_xforms = 0;
   }
   flam3_add_xforms(result, cpi[0].num_xforms, 0);

   result->final_xform_enable = cpi[0].final_xform_enable;
   result->final_xform_index = cpi[0].final_xform_index;

   result->time = time;
   result->interpolation = flam3_interpolation_linear;
   result->interpolation_type = cpi[0].interpolation_type;
   result->palette_interpolation = flam3_palette_interpolation_hsv;

   if (flam3_interpolation_linear == cps[i1].interpolation) {
       flam3_interpolate_n(result, 2, cpi, c);
   } else {
       interpolate_catmull_rom(cpi, c[1], result);
       free(cpi[2].xform);
       free(cpi[3].xform);
   }

   free(cpi[0].xform);
   free(cpi[1].xform);

#if 0
   flam3_print(stdout, result, NULL);
   for (i = 0; i < sizeof(result->xform[0].post); i++) {
       printf("%d ", ((unsigned char *)result->xform[0].post)[i]);
   }
   printf("\n");
#endif
}

static int compare_xforms(const void *av, const void *bv) {
   flam3_xform *a = (flam3_xform *) av;
   flam3_xform *b = (flam3_xform *) bv;
   double aa[2][2];
   double bb[2][2];
   double ad, bd;

   aa[0][0] = a->c[0][0];
   aa[0][1] = a->c[0][1];
   aa[1][0] = a->c[1][0];
   aa[1][1] = a->c[1][1];
   bb[0][0] = b->c[0][0];
   bb[0][1] = b->c[0][1];
   bb[1][0] = b->c[1][0];
   bb[1][1] = b->c[1][1];
   ad = det_matrix(aa);
   bd = det_matrix(bb);

   if (a->symmetry < b->symmetry) return 1;
   if (a->symmetry > b->symmetry) return -1;
   if (a->symmetry) {
      if (ad < 0) return -1;
      if (bd < 0) return 1;
      ad = atan2(a->c[0][0], a->c[0][1]);
      bd = atan2(b->c[0][0], b->c[0][1]);
   }

   if (ad < bd) return -1;
   if (ad > bd) return 1;
   return 0;
}


static void initialize_xforms(flam3_genome *thiscp, int start_here) {

   int i,j;

   for (i = start_here ; i < thiscp->num_xforms ; i++) {
       thiscp->xform[i].padding = 0;
       thiscp->xform[i].density = 0.0;
       thiscp->xform[i].symmetry = 0;
       thiscp->xform[i].color[0] = i&1;
       thiscp->xform[i].color[1] = (i&2)>>1;
       thiscp->xform[i].var[0] = 1.0;
       for (j = 1; j < flam3_nvariations; j++)
          thiscp->xform[i].var[j] = 0.0;
       thiscp->xform[i].c[0][0] = 1.0;
       thiscp->xform[i].c[0][1] = 0.0;
       thiscp->xform[i].c[1][0] = 0.0;
       thiscp->xform[i].c[1][1] = 1.0;
       thiscp->xform[i].c[2][0] = 0.0;
       thiscp->xform[i].c[2][1] = 0.0;
       thiscp->xform[i].post[0][0] = 1.0;
       thiscp->xform[i].post[0][1] = 0.0;
       thiscp->xform[i].post[1][0] = 0.0;
       thiscp->xform[i].post[1][1] = 1.0;
       thiscp->xform[i].post[2][0] = 0.0;
       thiscp->xform[i].post[2][1] = 0.0;
       thiscp->xform[i].wind[0] = 0.0;
       thiscp->xform[i].wind[1] = 0.0;
       thiscp->xform[i].blob_low = 0.0;
       thiscp->xform[i].blob_high = 1.0;
       thiscp->xform[i].blob_waves = 1.0;
       thiscp->xform[i].pdj_a = 0.0;
       thiscp->xform[i].pdj_b = 0.0;
       thiscp->xform[i].pdj_c = 0.0;
       thiscp->xform[i].pdj_d = 0.0;
       thiscp->xform[i].fan2_x = 0.0;
       thiscp->xform[i].fan2_y = 0.0;
       thiscp->xform[i].rings2_val = 0.0;
       thiscp->xform[i].perspective_angle = 0.0;
       thiscp->xform[i].perspective_dist = 0.0;
       thiscp->xform[i].persp_vsin = 0.0;
       thiscp->xform[i].persp_vfcos = 0.0;
       thiscp->xform[i].radialBlur_angle = 0.0;
       thiscp->xform[i].disc2_rot = 0.0;
       thiscp->xform[i].disc2_twist = 0.0;
       thiscp->xform[i].disc2_sinadd = 0.0;
       thiscp->xform[i].disc2_cosadd = 0.0;
       thiscp->xform[i].disc2_timespi = 0.0;
       thiscp->xform[i].flower_petals = 0.0;
       thiscp->xform[i].flower_holes = 0.0;
       thiscp->xform[i].parabola_height = 0.0;
       thiscp->xform[i].parabola_width = 0.0;
//       thiscp->xform[i].split_xsize = 0.0;
//       thiscp->xform[i].split_ysize = 0.0;
//       thiscp->xform[i].split_shift = 0.0;
//       thiscp->xform[i].move_x = 0.0;
//       thiscp->xform[i].move_y = 0.0;

       /*thiscp->xform[i].image_id = -1.0;*/

       /* Change parameters so that zeros aren't interpolated against */
       thiscp->xform[i].juliaN_power = 1.0;
       thiscp->xform[i].juliaN_dist = 1.0;
       thiscp->xform[i].juliaN_rN = 1.0;
       thiscp->xform[i].juliaN_cn = 0.5;
       thiscp->xform[i].juliaScope_power = 1.0;
       thiscp->xform[i].juliaScope_dist = 1.0;
       thiscp->xform[i].juliaScope_rN = 1.0;
       thiscp->xform[i].juliaScope_cn = 0.5;
       thiscp->xform[i].radialBlur_spinvar = 0.0;
       thiscp->xform[i].radialBlur_zoomvar = 1.0;
       thiscp->xform[i].pie_slices = 6.0;
       thiscp->xform[i].pie_rotation = 0.0;
       thiscp->xform[i].pie_thickness = 0.5;
       thiscp->xform[i].ngon_sides = 5;
       thiscp->xform[i].ngon_power = 3;
       thiscp->xform[i].ngon_circle = 1;
       thiscp->xform[i].ngon_corners = 2;
       thiscp->xform[i].curl_c1 = 1.0;
       thiscp->xform[i].curl_c2 = 0.0;
       thiscp->xform[i].rectangles_x = 1.0;
       thiscp->xform[i].rectangles_y = 1.0;
       thiscp->xform[i].amw_amp = 1.0;
       thiscp->xform[i].supershape_rnd = 0.0;
       thiscp->xform[i].supershape_m = 0.0;
       thiscp->xform[i].supershape_n1 = 1.0;
       thiscp->xform[i].supershape_n2 = 1.0;
       thiscp->xform[i].supershape_n3 = 1.0;
       thiscp->xform[i].supershape_holes = 0.0;
       thiscp->xform[i].conic_eccen = 1.0;
       thiscp->xform[i].conic_holes = 0.0;


   }
}

void flam3_copy_params(flam3_xform *dest, flam3_xform *src, int varn) {

   /* We only want to copy param var coefs for this one */
   if (varn==VAR_BLOB) {
      /* Blob */
      dest->blob_low = src->blob_low;
      dest->blob_high = src->blob_high;
      dest->blob_waves = src->blob_waves;
   } else if (varn==VAR_PDJ) {
      /* PDJ */
      dest->pdj_a = src->pdj_a;
      dest->pdj_b = src->pdj_b;
      dest->pdj_c = src->pdj_c;
      dest->pdj_d = src->pdj_d;
   } else if (varn==VAR_FAN2) {
      /* Fan2 */
      dest->fan2_x = src->fan2_x;
      dest->fan2_y = src->fan2_y;
   } else if (varn==VAR_RINGS2) {
      /* Rings2 */
      dest->rings2_val = src->rings2_val;
   } else if (varn==VAR_PERSPECTIVE) {
      /* Perspective */
      dest->perspective_angle = src->perspective_angle;
      dest->perspective_dist = src->perspective_dist;
      dest->persp_vsin = src->persp_vsin;
      dest->persp_vfcos = src->persp_vfcos;
   } else if (varn==VAR_JULIAN) {
      /* Julia_N */
      dest->juliaN_power = src->juliaN_power;
      dest->juliaN_dist = src->juliaN_dist;
      dest->juliaN_rN = src->juliaN_rN;
      dest->juliaN_cn = src->juliaN_cn;
   } else if (varn==VAR_JULIASCOPE) {
      /* Julia_Scope */
      dest->juliaScope_power = src->juliaScope_power;
      dest->juliaScope_dist = src->juliaScope_dist;
      dest->juliaScope_rN = src->juliaScope_rN;
      dest->juliaScope_cn = src->juliaScope_cn;
   } else if (varn==VAR_RADIAL_BLUR) {
      /* Radial Blur */
      dest->radialBlur_angle = src->radialBlur_angle;
   } else if (varn==VAR_PIE) {
      /* Pie */
      dest->pie_slices = src->pie_slices;
      dest->pie_rotation = src->pie_rotation;
      dest->pie_thickness = src->pie_thickness;
   } else if (varn==VAR_NGON) {
      /* Ngon */
      dest->ngon_sides = src->ngon_sides;
      dest->ngon_power = src->ngon_power;
      dest->ngon_corners = src->ngon_corners;
      dest->ngon_circle = src->ngon_circle;
   } else if (varn==VAR_CURL) {
      /* Curl */
      dest->curl_c1 = src->curl_c1;
      dest->curl_c2 = src->curl_c2;
   } else if (varn==VAR_RECTANGLES) {
      /* Rect */
      dest->rectangles_x = src->rectangles_x;
      dest->rectangles_y = src->rectangles_y;
   } else if (varn==VAR_DISC2) {
      /* Disc2 */
      dest->disc2_rot = src->disc2_rot;
      dest->disc2_twist = src->disc2_twist;
   } else if (varn==VAR_SUPER_SHAPE) {
      /* Supershape */
      dest->supershape_rnd = src->supershape_rnd;
      dest->supershape_m = src->supershape_m;
      dest->supershape_n1 = src->supershape_n1;
      dest->supershape_n2 = src->supershape_n2;
      dest->supershape_n3 = src->supershape_n3;
      dest->supershape_holes = src->supershape_holes;
   } else if (varn==VAR_FLOWER) {
      /* Flower */
      dest->flower_petals = src->flower_petals;
      dest->flower_petals = src->flower_petals;
   } else if (varn==VAR_CONIC) {
      /* Conic */
      dest->conic_eccen = src->conic_eccen;
      dest->conic_holes = src->conic_holes;
   } else if (varn==VAR_PARABOLA) {
      /* Parabola */
      dest->parabola_height = src->parabola_height;
      dest->parabola_width = src->parabola_width;
   }
}

/* Xform support functions */
void flam3_add_xforms(flam3_genome *thiscp, int num_to_add, int interp_padding) {

   int i;
   int old_num = thiscp->num_xforms;

   if (thiscp->num_xforms > 0)
      thiscp->xform = (flam3_xform *)realloc(thiscp->xform, (thiscp->num_xforms + num_to_add) * sizeof(flam3_xform));
   else
      thiscp->xform = (flam3_xform *)malloc(num_to_add * sizeof(flam3_xform));

   thiscp->num_xforms += num_to_add;

   /* Initialize all the new xforms */
   initialize_xforms(thiscp, old_num);

   /* Set the padding flag for the new xforms */
   if (interp_padding) {
      for (i = old_num ; i < thiscp->num_xforms ; i++)
         thiscp->xform[i].padding=1;
   }

}

void flam3_delete_xform(flam3_genome *thiscp, int idx_to_delete) {

   int i;

   /* Handle the final xform index */
   if (thiscp->final_xform_index == idx_to_delete) {
      thiscp->final_xform_index = -1;
      thiscp->final_xform_enable = 0;
   } else if (thiscp->final_xform_index > idx_to_delete) {
      thiscp->final_xform_index--;
   }

   /* Move all of the xforms down one */
   for (i=idx_to_delete; i<thiscp->num_xforms-1; i++)
      thiscp->xform[i] = thiscp->xform[i+1];

   thiscp->num_xforms--;

   /* Reduce the memory storage by one xform */
   thiscp->xform = (flam3_xform *)realloc(thiscp->xform, sizeof(flam3_xform) * thiscp->num_xforms);

}



/* Copy one control point to another */
void flam3_copy(flam3_genome *dest, flam3_genome *src) {

   /* If there are any xforms in dest before the copy, clean them up */
   if (dest->num_xforms > 0 && dest->xform!=NULL) {
      free(dest->xform);
      dest->num_xforms = 0;
   }

   /* Copy main contents of genome */
   memcpy(dest, src, sizeof(flam3_genome));

   /* Only the pointer to the xform was copied, not the actual xforms. */
   /* We need to create new xform memory storage for this new cp       */
   dest->num_xforms = 0;
   dest->xform = NULL;

   flam3_add_xforms(dest, src->num_xforms, 0);
   memcpy(dest->xform, src->xform, dest->num_xforms * sizeof(flam3_xform));
}

void flam3_copyx(flam3_genome *dest, flam3_genome *src, int dest_std_xforms, int dest_final_xform) {

   int i,j;

   /* If there are any xforms in dest before the copy, clean them up */
   if (dest->num_xforms > 0 && dest->xform!=NULL) {
      free(dest->xform);
      dest->num_xforms = 0;
   }

   /* Copy main contents of genome */
   memcpy(dest, src, sizeof(flam3_genome));

   /* Only the pointer to the xform was copied, not the actual xforms. */
   /* We need to create new xform memory storage for this new cp       */
   dest->num_xforms = 0;
   dest->xform = NULL;

   /* Add the padded standard xform list */
   /* Set the pad to 1 for these */
   flam3_add_xforms(dest, dest_std_xforms, 1);

   j=0;
   for(i=0;i<src->num_xforms;i++) {

      if (i==src->final_xform_index)
         continue;

      /* When we copy the old xform, the pad is set to 0 */
      dest->xform[j++] = src->xform[i];
   }

   /* Add the final xform if necessary */
   if (dest_final_xform > 0) {
      flam3_add_xforms(dest, dest_final_xform, 1);
      dest->final_xform_index = dest->num_xforms-1;
      dest->final_xform_enable = 1;

      if (src->final_xform_enable > 0) {
         dest->xform[dest->num_xforms-1] = src->xform[src->final_xform_index];
      } else {
         /* Interpolated-against final xforms need symmetry set */
         dest->xform[dest->num_xforms-1].symmetry=1.0;
      }

   } else {
      dest->final_xform_index = -1;
      dest->final_xform_enable = 0;
   }

}


static flam3_genome xml_current_cp;
static flam3_genome *xml_all_cp;
/*static flam3_image_store *im_store;*/
static int xml_all_ncps;
/*static int xml_num_images;*/

static void clear_cp(flam3_genome *cp, int default_flag) {
    cp->palette_index = flam3_palette_random;
    cp->center[0] = 0.0;
    cp->center[1] = 0.0;
    cp->rot_center[0] = 0.0;
    cp->rot_center[1] = 0.0;
    cp->gamma = 4.0;
    cp->vibrancy = 1.0;
    cp->contrast = 1.0;
    cp->brightness = 4.0;
    cp->symmetry = 0;
    cp->hue_rotation = 0.0;
    cp->rotate = 0.0;
    cp->edits = NULL;
    cp->pixels_per_unit = 50;
    cp->final_xform_enable = 0;
    cp->final_xform_index = -1;
    cp->interpolation = flam3_interpolation_linear;
    cp->palette_interpolation = flam3_palette_interpolation_hsv;

    cp->genome_index = 0;
    memset(cp->parent_fname,0,flam3_parent_fn_len);

    if (default_flag==flam3_defaults_on) {
       /* If defaults are on, set to reasonable values */
       cp->background[0] = 0.0;
       cp->background[1] = 0.0;
       cp->background[2] = 0.0;
       cp->width = 100;
       cp->height = 100;
       cp->spatial_oversample = 1;
       cp->spatial_filter_radius = 0.5;
       cp->zoom = 0.0;
       cp->sample_density = 1;
       /* Density estimation stuff defaulting to ON */
       cp->estimator = 9.0;
       cp->estimator_minimum = 0.0;
       cp->estimator_curve = 0.4;
       cp->gam_lin_thresh = 0.01;
//       cp->motion_exp = 0.0;
       cp->nbatches = 1;
       cp->ntemporal_samples = 1000;
       cp->spatial_filter_select = flam3_gaussian_kernel;
       cp->interpolation_type = flam3_inttype_log;
       cp->temporal_filter_type = flam3_temporal_box;
       cp->temporal_filter_width = 1.0;
       cp->temporal_filter_exp = 0.0;

    } else {
       /* Defaults are off, so set to UN-reasonable values. */
       cp->background[0] = -1.0;
       cp->background[1] = -1.0;
       cp->background[2] = -1.0;
       cp->zoom = 999999999;
       cp->spatial_oversample = -1;
       cp->spatial_filter_radius = -1;
       cp->nbatches = -1;
       cp->ntemporal_samples = -1;
       cp->width = -1;
       cp->height = -1;
       cp->sample_density = -1;
       cp->estimator = -1;
       cp->estimator_minimum = -1;
       cp->estimator_curve = -1;
       cp->gam_lin_thresh = -1;
//       cp->motion_exp = -999;
       cp->nbatches = 0;
       cp->ntemporal_samples = 0;
       cp->spatial_filter_select = -1;
       cp->interpolation_type = -1;
       cp->temporal_filter_type = -1;
       cp->temporal_filter_width = -1;
       cp->temporal_filter_exp = -999;
    }

    if (cp->xform != NULL && cp->num_xforms > 0) {
       free(cp->xform);
       cp->num_xforms = 0;
    }
}

static double flam3_spatial_support[flam3_num_spatialfilters] = {

   1.8, /* gaussian */
   1.0, /* hermite */
   0.5, /* box */
   1.0, /* triangle */
   1.5, /* bell */
   2.0, /* b spline */
   2.0, /* mitchell */
   1.0, /* blackman */
   2.0, /* catrom */
   1.0, /* hanning */
   1.0, /* hamming */
   3.0, /* lanczos3 */
   2.0, /* lanczos2 */
   1.5  /* quadratic */
};

double flam3_spatial_filter(int knum, double x) {

   if (knum==0)
      return flam3_gaussian_filter(x);
   else if (knum==1)
      return flam3_hermite_filter(x);
   else if (knum==2)
      return flam3_box_filter(x);
   else if (knum==3)
      return flam3_triangle_filter(x);
   else if (knum==4)
      return flam3_bell_filter(x);
   else if (knum==5)
      return flam3_b_spline_filter(x);
   else if (knum==6)
      return flam3_mitchell_filter(x);
   else if (knum==7)
      return flam3_blackman_filter(x);
   else if (knum==8)
      return flam3_catrom_filter(x);
   else if (knum==9)
      return flam3_hanning_filter(x);
   else if (knum==10)
      return flam3_hamming_filter(x);
   else if (knum==11)
      return flam3_lanczos3_filter(x);
   else if (knum==12)
      return flam3_lanczos2_filter(x);
   else if (knum==13)
      return flam3_quadratic_filter(x);
   else {
      fprintf(stderr,"Unknown filter kernel %d!\n",knum);
      exit(1);
   }
}

char *flam3_variation_names[1+flam3_nvariations] = {
  "linear",
  "sinusoidal",
  "spherical",
  "swirl",
  "horseshoe",
  "polar",
  "handkerchief",
  "heart",
  "disc",
  "spiral",
  "hyperbolic",
  "diamond",
  "ex",
  "julia",
  "bent",
  "waves",
  "fisheye",
  "popcorn",
  "exponential",
  "power",
  "cosine",
  "rings",
  "fan",
  "blob",
  "pdj",
  "fan2",
  "rings2",
  "eyefish",
  "bubble",
  "cylinder",
  "perspective",
  "noise",
  "julian",
  "juliascope",
  "blur",
  "gaussian_blur",
  "radial_blur",
  "pie",
  "ngon",
  "curl",
  "rectangles",
  "arch",
  "tangent",
  "square",
  "rays",
  "blade",
  "secant2",
  "twintrian",
  "cross",
  "disc2",
  "super_shape",
  "flower",
  "conic",
  "parabola",
  0
};


static int var2n(const char *s) {
  int i;
  for (i = 0; i < flam3_nvariations; i++)
    if (!strcmp(s, flam3_variation_names[i])) return i;
  return flam3_variation_none;
}

#if 0
static void parse_image_element(xmlNode *image_node) {
   flam3_image_store *im = &(im_store[xml_num_images]);
   xmlAttrPtr att_ptr, cur_att;
   xmlNode *chld_node;
   int cur_bin;
   int cur_ptr;
   char *att_str;
   char *bin_content;
   int i;
   unsigned char *tmp_ptr;

   att_ptr = image_node->properties;

   if (att_ptr==NULL) {
      fprintf(stderr,"Error : <image> element has no attributes.\n");
      exit(1);
   }

   for (cur_att = att_ptr; cur_att; cur_att = cur_att->next) {

      att_str = (char *) xmlGetProp(image_node,cur_att->name);

      if (!xmlStrcmp(cur_att->name, (const xmlChar *)"encoding")) {

         if (strcmp("base64", att_str)) {
            fprintf(stderr,"Only base64 encoded image data is currently supported.\n");
            exit(1);
         }

      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"version")) {
         im->version = atoi(att_str);
      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"id")) {
         im->id = atoi(att_str);
      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"width")) {
         im->width = atoi(att_str);
      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"height")) {
         im->height = atoi(att_str);
      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"bits")) {
         if (atoi(att_str) != 16) {
            fprintf(stderr,"Only 16 bit encoded data is currently supported.\n");
            exit(1);
         }
      }

      xmlFree(att_str);
   }

   /* Allocate the memory for the data in the bins */
   im->rowcols = (unsigned short *)calloc(2 * im->width * im->height, sizeof(unsigned short));
   tmp_ptr = (unsigned char *)im->rowcols;
   /* The image properties have been parsed.  Now there will be a series */
   /* of 255 bin elements with lots of good information in them          */
   cur_bin = 0; cur_ptr = 0;
   for (chld_node=image_node->children; chld_node; chld_node = chld_node->next) {

      /* Is this a bin node? */
      if (!xmlStrcmp(chld_node->name, (const xmlChar *)"bin")) {

         cur_bin++;

         /* The bin will also have attributes (which we should store) */
         att_ptr = chld_node->properties;

         if (att_ptr==NULL) {
            fprintf(stderr,"Error : <bin> element has no attributes.\n");
            exit(1);
         }

         for (cur_att = att_ptr; cur_att; cur_att = cur_att->next) {

            att_str = (char *) xmlGetProp(chld_node,cur_att->name);

            if (!xmlStrcmp(cur_att->name, (const xmlChar *)"weight")) {
               im->intensity_weight[cur_bin] = atof(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"size")) {
               im->bin_size[cur_bin] = atoi(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"intensity")) {
               if (cur_bin != atoi(att_str)) {
                  fprintf(stderr,"Error : Bin %d is missing!\n",cur_bin+1);
                  exit(1);
               }
            }

            xmlFree(att_str);
         }


         im->bin_offset[cur_bin] = cur_ptr;

         /* Now, read the contents of the <bin> and decode into 16-bit ints */
         if (im->bin_size[cur_bin]>0) {
            bin_content = (char *) xmlNodeGetContent(chld_node);
            b64decode(bin_content, (char *)&(im->rowcols[cur_ptr]) );

            if (cur_ptr >= im->width*im->height*2) {
               fprintf(stderr,"Error!  Too many values specified for image! (bin=%d)\n",cur_bin);
               exit(1);
            }

            /* Swap bytes (contents are little-endian since written on PC */
            for (i=cur_ptr; i<cur_ptr+2*im->bin_size[cur_bin]; i++) {
               im->rowcols[i] = 256*(int)tmp_ptr[i*2] + (int)tmp_ptr[i*2+1];
            }

            /* Shift the pointer forward to the first element of the next bin*/
            cur_ptr += 2 * im->bin_size[cur_bin];

            xmlFree(bin_content);
         }

      }
   }
   /* Check to see that we got all the bins */
   if (cur_bin != 255) {
      fprintf(stderr,"Error: Bins missing! %d\n",cur_bin);
      exit(1);
   }
}
#endif

static void parse_flame_element(xmlNode *flame_node) {
   flam3_genome *cp = &xml_current_cp;
   xmlNode *chld_node;
   xmlNodePtr edit_node;
   xmlAttrPtr att_ptr, cur_att;
   char *att_str;
   char *cpy;
   int i;

   /* Store this flame element in the current cp */

   /* The top level element is a flame element. */
   /* Read the attributes of it and store them. */
   att_ptr = flame_node->properties;

   if (att_ptr==NULL) {
      fprintf(stderr, "Error : <flame> element has no attributes.\n");
      exit(1);
   }

   memset(cp->flame_name,0,flam3_name_len+1);

   for (cur_att = att_ptr; cur_att; cur_att = cur_att->next) {

       att_str = (char *) xmlGetProp(flame_node,cur_att->name);

      /* Compare attribute names */
      if (!xmlStrcmp(cur_att->name, (const xmlChar *)"time")) {
         cp->time = atof(att_str);
      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"interpolation")) {
     if (!strcmp("linear", att_str)) {
         cp->interpolation = flam3_interpolation_linear;
     } else if  (!strcmp("smooth", att_str)) {
         cp->interpolation = flam3_interpolation_smooth;
     } else {
         fprintf(stderr, "warning: unrecognized interpolation type %s.\n", att_str);
     }
      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"palette_interpolation")) {
     if (!strcmp("hsv", att_str)) {
         cp->palette_interpolation = flam3_palette_interpolation_hsv;
     } else if  (!strcmp("sweep", att_str)) {
         cp->palette_interpolation = flam3_palette_interpolation_sweep;
     } else {
         fprintf(stderr, "warning: unrecognized palette interpolation type %s.\n", att_str);
     }
      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"interpolation_space") ||
                 !xmlStrcmp(cur_att->name, (const xmlChar *)"interpolation_type")) {

         if (!strcmp("linear", att_str))
            cp->interpolation_type = flam3_inttype_linear;
         else if (!strcmp("log", att_str))
            cp->interpolation_type = flam3_inttype_log;
         else if (!strcmp("old", att_str))
            cp->interpolation_type = flam3_inttype_compat;
         else if (!strcmp("older", att_str))
            cp->interpolation_type = flam3_inttype_older;
         else
            fprintf(stderr,"warning: unrecognized interpolation_type %s.\n",att_str);

      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"name")) {
         strncpy(cp->flame_name, att_str, flam3_name_len);
         i = (int)strlen(cp->flame_name)-1;
         while(i-->0) {
            if (isspace(cp->flame_name[i]))
               cp->flame_name[i] = '_';
         }

      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"palette")) {
         cp->palette_index = atoi(att_str);
      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"size")) {
         sscanf(att_str, "%d %d", &cp->width, &cp->height);
      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"center")) {
         sscanf(att_str, "%lf %lf", &cp->center[0], &cp->center[1]);
         cp->rot_center[0] = cp->center[0];
         cp->rot_center[1] = cp->center[1];
      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"scale")) {
         cp->pixels_per_unit = atof(att_str);
      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"rotate")) {
         cp->rotate = atof(att_str);
      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"zoom")) {
         cp->zoom = atof(att_str);
      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"oversample")) {
         cp->spatial_oversample = atoi(att_str);
      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"supersample")) {
         cp->spatial_oversample = atoi(att_str);
      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"filter")) {
         cp->spatial_filter_radius = atof(att_str);
      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"filter_shape")) {
         if (!strcmp("gaussian", att_str))
            cp->spatial_filter_select = flam3_gaussian_kernel;
         else if (!strcmp("hermite", att_str))
            cp->spatial_filter_select = flam3_hermite_kernel;
         else if (!strcmp("box", att_str))
            cp->spatial_filter_select = flam3_box_kernel;
         else if (!strcmp("triangle", att_str))
            cp->spatial_filter_select = flam3_triangle_kernel;
         else if (!strcmp("bell", att_str))
            cp->spatial_filter_select = flam3_bell_kernel;
         else if (!strcmp("bspline", att_str))
            cp->spatial_filter_select = flam3_b_spline_kernel;
         else if (!strcmp("mitchell", att_str))
            cp->spatial_filter_select = flam3_mitchell_kernel;
         else if (!strcmp("blackman", att_str))
            cp->spatial_filter_select = flam3_blackman_kernel;
         else if (!strcmp("catrom", att_str))
            cp->spatial_filter_select = flam3_catrom_kernel;
         else if (!strcmp("hanning", att_str))
            cp->spatial_filter_select = flam3_hanning_kernel;
         else if (!strcmp("hamming", att_str))
            cp->spatial_filter_select = flam3_hamming_kernel;
         else if (!strcmp("lanczos3", att_str))
            cp->spatial_filter_select = flam3_lanczos3_kernel;
         else if (!strcmp("lanczos2", att_str))
            cp->spatial_filter_select = flam3_lanczos2_kernel;
         else if (!strcmp("quadratic", att_str))
            cp->spatial_filter_select = flam3_quadratic_kernel;
         else
            fprintf(stderr, "warning: unrecognized kernel shape %s.  Using gaussian.\n", att_str);

      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"temporal_filter_type")) {
         if (!strcmp("box", att_str))
            cp->temporal_filter_type = flam3_temporal_box;
         else if (!strcmp("gaussian", att_str))
            cp->temporal_filter_type = flam3_temporal_gaussian;
         else if (!strcmp("exp",att_str))
            cp->temporal_filter_type = flam3_temporal_exp;
         else
            fprintf(stderr, "warning: unrecognized spatial filter %s.  Using box.\n",att_str);
      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"temporal_filter_width")) {
         cp->temporal_filter_width = atof(att_str);
      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"temporal_filter_exp")) {
         cp->temporal_filter_exp = atof(att_str);
      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"quality")) {
         cp->sample_density = atof(att_str);
      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"passes")) {
         cp->nbatches = atoi(att_str);
      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"temporal_samples")) {
         cp->ntemporal_samples = atoi(att_str);
      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"background")) {
         sscanf(att_str, "%lf %lf %lf", &cp->background[0], &cp->background[1], &cp->background[2]);
      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"brightness")) {
         cp->brightness = atof(att_str);
/*      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"contrast")) {
         cp->contrast = atof(att_str);*/
      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"gamma")) {
         cp->gamma = atof(att_str);
      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"vibrancy")) {
         cp->vibrancy = atof(att_str);
      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"hue")) {
         cp->hue_rotation = fmod(atof(att_str), 1.0);
      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"estimator_radius")) {
         cp->estimator = atof(att_str);
      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"estimator_minimum")) {
         cp->estimator_minimum = atof(att_str);
      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"estimator_curve")) {
         cp->estimator_curve = atof(att_str);
      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"gamma_threshold")) {
         cp->gam_lin_thresh = atof(att_str);
//      } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"motion_exponent")) {
//         cp->motion_exp = atof(att_str);
      }

      xmlFree(att_str);

   }

   /* Finished with flame attributes.  Now look at children of flame element. */
   for (chld_node=flame_node->children; chld_node; chld_node = chld_node->next) {

      /* Is this a color node? */
      if (!xmlStrcmp(chld_node->name, (const xmlChar *)"color")) {
         int index = -1;
         double r=0.0,g=0.0,b=0.0;

         /* Loop through the attributes of the color element */
         att_ptr = chld_node->properties;

         if (att_ptr==NULL) {
            fprintf(stderr,"Error:  No attributes for color element.\n");
            exit(1);
         }

         for (cur_att=att_ptr; cur_att; cur_att = cur_att->next) {

            att_str = (char *) xmlGetProp(chld_node,cur_att->name);

            if (!xmlStrcmp(cur_att->name, (const xmlChar *)"index")) {
               index = atoi(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"rgb")) {
               sscanf(att_str, "%lf %lf %lf", &r, &g, &b);
            } else {
               fprintf(stderr,"Error:  Unknown color attribute '%s'\n",cur_att->name);
               exit(1);
            }

            xmlFree(att_str);
         }

         if (index >= 0 && index < 256) {
            cp->palette[index][0] = r / 255.0;
            cp->palette[index][1] = g / 255.0;
            cp->palette[index][2] = b / 255.0;
         } else {
            fprintf(stderr,"Error:  Color element with bad/missing index attribute (%d)\n",index);
            exit(1);
         }
      } else if (!xmlStrcmp(chld_node->name, (const xmlChar *)"colors")) {

         int count;

         /* Loop through the attributes of the colors element */
         att_ptr = chld_node->properties;

         if (att_ptr==NULL) {
            fprintf(stderr,"Error: No attributes for colors element.\n");
            exit(1);
         }

         for (cur_att=att_ptr; cur_att; cur_att = cur_att->next) {

            att_str = (char *) xmlGetProp(chld_node,cur_att->name);

            if (!xmlStrcmp(cur_att->name, (const xmlChar *)"count")) {
               count = atoi(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"data")) {
               flam3_parse_hexformat_colors(att_str, cp, count, 4);
            } else {
               fprintf(stderr,"Error:  Unknown color attribute '%s'\n",cur_att->name);
               exit(1);
            }

            xmlFree(att_str);
         }


      } else if (!xmlStrcmp(chld_node->name, (const xmlChar *)"palette")) {

         /* This could be either the old form of palette or the new form */
         /* Make sure BOTH are not specified, otherwise either are ok    */
         int numcolors=0;
         int numbytes=0;
         int old_format=0;
         int new_format=0;
         int index0, index1;
         double hue0, hue1;
         double blend = 0.5;
         index0 = index1 = flam3_palette_random;
         hue0 = hue1 = 0.0;

         /* Loop through the attributes of the palette element */
         att_ptr = chld_node->properties;

         if (att_ptr==NULL) {
            fprintf(stderr,"Error:  No attributes for palette element.\n");
            exit(1);
         }

         for (cur_att=att_ptr; cur_att; cur_att = cur_att->next) {

            att_str = (char *) xmlGetProp(chld_node,cur_att->name);

            if (!xmlStrcmp(cur_att->name, (const xmlChar *)"index0")) {
               old_format++;
               index0 = atoi(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"index1")) {
               old_format++;
               index1 = atoi(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"hue0")) {
               old_format++;
               hue0 = atof(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"hue1")) {
               old_format++;
               hue1 = atof(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"blend")) {
               old_format++;
               blend = atof(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"count")) {
               new_format++;
               numcolors = atoi(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"format")) {
               new_format++;
               if (!strcmp(att_str,"RGB"))
                  numbytes=3;
               else if (!strcmp(att_str,"RGBA"))
                  numbytes=4;
               else {
                  fprintf(stderr,"Error: Unrecognized palette format string (%s)\n",att_str);
                  exit(1);
               }
            } else {
               fprintf(stderr,"Error:  Unknown palette attribute '%s'\n",cur_att->name);
               exit(1);
            }

            xmlFree(att_str);
         }

         /* Old or new format? */
         if (new_format>0 && old_format>0) {
            fprintf(stderr,"Error: mixing of old and new palette tag syntax not allowed.\n");
            exit(1);
         }

         if (old_format>0)
            interpolate_cmap(cp->palette, blend, index0, hue0, index1, hue1);
         else {

            char *pal_str;

            /* Read formatted string from contents of tag */

            pal_str = (char *) xmlNodeGetContent(chld_node);

            flam3_parse_hexformat_colors(pal_str, cp, numcolors, numbytes);

            xmlFree(pal_str);
         }
      } else if (!xmlStrcmp(chld_node->name, (const xmlChar *)"symmetry")) {

         int kind=0;

         /* Loop through the attributes of the symmetry element */
         att_ptr = chld_node->properties;

         if (att_ptr==NULL) {
            fprintf(stderr,"Error:  No attributes for symmetry element.\n");
            exit(1);
         }

         for (cur_att=att_ptr; cur_att; cur_att = cur_att->next) {

            att_str = (char *) xmlGetProp(chld_node,cur_att->name);

            if (!xmlStrcmp(cur_att->name, (const xmlChar *)"kind")) {
               kind = atoi(att_str);
            } else {
               fprintf(stderr,"Error:  Unknown symmetry attribute '%s'\n",cur_att->name);
               exit(1);
            }

            xmlFree(att_str);
         }

         flam3_add_symmetry(cp,kind);

      } else if (!xmlStrcmp(chld_node->name, (const xmlChar *)"xform") ||
                  !xmlStrcmp(chld_node->name, (const xmlChar *)"finalxform")) {

         int j,k,xf;
         int perspective_used = 0;
         int julian_used = 0;
         int juliascope_used = 0;
         int radialblur_used = 0;
         int disc2_used = 0;
         int supershape_used = 0;

         xf = cp->num_xforms;
         flam3_add_xforms(cp, 1, 0);

         if (!xmlStrcmp(chld_node->name, (const xmlChar *)"finalxform")) {

            if (cp->final_xform_index >=0) {
               fprintf(stderr,"Error:  Cannot specify more than one final xform.\n");
               exit(1);
            }

            cp->final_xform_index = xf;
            /* Now, if present, the xform enable defaults to on */
            cp->final_xform_enable = 1;
         }

         /* Even though most of these are already 0, set them all to be sure */
         for (j = 0; j < flam3_nvariations; j++) {
            cp->xform[xf].var[j] = 0.0;
         }

         /* Loop through the attributes of the xform element */
         att_ptr = chld_node->properties;

         if (att_ptr==NULL) {
            fprintf(stderr,"Error: No attributes for xform element.\n");
            exit(1);
         }

         for (cur_att=att_ptr; cur_att; cur_att = cur_att->next) {


            att_str = (char *) xmlGetProp(chld_node,cur_att->name);

            cpy = att_str;

            if (!xmlStrcmp(cur_att->name, (const xmlChar *)"weight")) {
               if (cp->final_xform_index==xf) {
                  fprintf(stderr,"Error: Final xforms should not have weight specified.\n");
               	exit(1);
               }
               cp->xform[xf].density = atof(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"enabled")) {
               cp->final_xform_enable = atoi(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"symmetry")) {
               cp->xform[xf].symmetry = atof(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"color")) {
               cp->xform[xf].color[1] = 0.0;
               sscanf(att_str, "%lf %lf", &cp->xform[xf].color[0], &cp->xform[xf].color[1]);
               sscanf(att_str, "%lf", &cp->xform[xf].color[0]);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"var1")) {
               for (j=0; j < flam3_nvariations; j++) {
                  cp->xform[xf].var[j] = 0.0;
               }
               j = atoi(att_str);

               if (j < 0 || j >= flam3_nvariations) {
                  fprintf(stderr,"Error:  Bad variation (%d)\n",j);
                  j=0;
               }

               cp->xform[xf].var[j] = 1.0;
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"var")) {
               for (j=0; j < flam3_nvariations; j++) {
                  cp->xform[xf].var[j] = strtod(cpy, &cpy);
               }
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"coefs")) {
               for (k=0; k<3; k++) {
                  for (j=0; j<2; j++) {
                     cp->xform[xf].c[k][j] = strtod(cpy, &cpy);
                  }
               }
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"post")) {
               for (k = 0; k < 3; k++) {
                  for (j = 0; j < 2; j++) {
                     cp->xform[xf].post[k][j] = strtod(cpy, &cpy);
                  }
               }
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"blob_low")) {
               cp->xform[xf].blob_low = atof(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"blob_high")) {
               cp->xform[xf].blob_high = atof(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"blob_waves")) {
               cp->xform[xf].blob_waves = atof(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"pdj_a")) {
               cp->xform[xf].pdj_a = atof(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"pdj_b")) {
               cp->xform[xf].pdj_b = atof(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"pdj_c")) {
               cp->xform[xf].pdj_c = atof(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"pdj_d")) {
               cp->xform[xf].pdj_d = atof(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"fan2_x")) {
               cp->xform[xf].fan2_x = atof(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"fan2_y")) {
               cp->xform[xf].fan2_y = atof(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"rings2_val")) {
               cp->xform[xf].rings2_val = atof(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"perspective_angle")) {
               cp->xform[xf].perspective_angle = atof(att_str);
               perspective_used = 1;
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"perspective_dist")) {
               cp->xform[xf].perspective_dist = atof(att_str);
               perspective_used = 1;
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"julian_power")) {
               cp->xform[xf].juliaN_power = atof(att_str);
               julian_used = 1;
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"julian_dist")) {
               cp->xform[xf].juliaN_dist = atof(att_str);
               julian_used = 1;
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"juliascope_power")) {
               cp->xform[xf].juliaScope_power = atof(att_str);
               juliascope_used = 1;
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"juliascope_dist")) {
               cp->xform[xf].juliaScope_dist = atof(att_str);
               juliascope_used = 1;
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"radial_blur_angle")) {
               cp->xform[xf].radialBlur_angle = atof(att_str);
               radialblur_used = 1;
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"pie_slices")) {
               cp->xform[xf].pie_slices = atof(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"pie_rotation")) {
               cp->xform[xf].pie_rotation = atof(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"pie_thickness")) {
               cp->xform[xf].pie_thickness = atof(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"ngon_sides")) {
               cp->xform[xf].ngon_sides = atof(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"ngon_power")) {
               cp->xform[xf].ngon_power = atof(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"ngon_circle")) {
               cp->xform[xf].ngon_circle = atof(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"ngon_corners")) {
               cp->xform[xf].ngon_corners = atof(att_str);
/*            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"image_filename")) {
               cp->xform[xf].image_id = atoi(att_str);*/
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"curl_c1")) {
               cp->xform[xf].curl_c1 = atof(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"curl_c2")) {
               cp->xform[xf].curl_c2 = atof(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"rectangles_x")) {
               cp->xform[xf].rectangles_x = atof(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"rectangles_y")) {
               cp->xform[xf].rectangles_y = atof(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"amw_amp")) {
               cp->xform[xf].amw_amp = atof(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"disc2_rot")) {
               cp->xform[xf].disc2_rot = atof(att_str);
               disc2_used = 1;
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"disc2_twist")) {
               cp->xform[xf].disc2_twist = atof(att_str);
               disc2_used = 1;
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"super_shape_rnd")) {
               cp->xform[xf].supershape_rnd = atof(att_str);
               /* Limit to [0,1] */
               if (cp->xform[xf].supershape_rnd<0)
                  cp->xform[xf].supershape_rnd=0;
               else if (cp->xform[xf].supershape_rnd>1)
                  cp->xform[xf].supershape_rnd=1;
               supershape_used = 1;
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"super_shape_m")) {
               cp->xform[xf].supershape_m = atof(att_str);
               supershape_used = 1;
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"super_shape_n1")) {
               cp->xform[xf].supershape_n1 = atof(att_str);
               supershape_used = 1;
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"super_shape_n2")) {
               cp->xform[xf].supershape_n2 = atof(att_str);
               supershape_used = 1;
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"super_shape_n3")) {
               cp->xform[xf].supershape_n3 = atof(att_str);
               supershape_used = 1;
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"super_shape_holes")) {
               cp->xform[xf].supershape_holes = atof(att_str);
               supershape_used = 1;
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"flower_petals")) {
               cp->xform[xf].flower_petals = atof(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"flower_holes")) {
               cp->xform[xf].flower_holes = atof(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"conic_eccentricity")) {
               cp->xform[xf].conic_eccen = atof(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"conic_holes")) {
               cp->xform[xf].conic_holes = atof(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"parabola_height")) {
               cp->xform[xf].parabola_height = atof(att_str);
            } else if (!xmlStrcmp(cur_att->name, (const xmlChar *)"parabola_width")) {
               cp->xform[xf].parabola_width = atof(att_str);
            } else {
               int v = var2n((char *) cur_att->name);
               if (v != flam3_variation_none)
                  cp->xform[xf].var[v] = atof(att_str);
               else
                  fprintf(stderr,"Warning: unrecognized variation %s.  Ignoring.\n",(char *)cur_att->name);
            }


            xmlFree(att_str);
         }

         /* Precalculate meta-parameters if necessary */
/*         if (perspective_used>0)
            perspective_precalc(&(cp->xform[xf]));

         if (julian_used>0)
            juliaN_precalc(&(cp->xform[xf]));

         if (juliascope_used>0)
            juliaScope_precalc(&(cp->xform[xf]));

         if (radialblur_used>0)
            radial_blur_precalc(&(cp->xform[xf]));

         if (disc2_used>0)
            disc2_precalc(&(cp->xform[xf]));

         if (supershape_used>0)
            supershape_precalc(&(cp->xform[xf]));

         waves_precalc(&(cp->xform[xf]));
*/

      } else if (!xmlStrcmp(chld_node->name, (const xmlChar *)"edit")) {

         /* Create a new XML document with this edit node as the root node */
         cp->edits = xmlNewDoc( (const xmlChar *)"1.0");
         edit_node = xmlCopyNode( chld_node, 1 );
         xmlDocSetRootElement(cp->edits, edit_node);

      }
   } /* Done parsing flame element. */
}

int flam3_count_nthreads(void) {
   int nthreads;
#ifndef HAVE_LIBPTHREAD
   return(1);
#endif

#ifdef WIN32
   SYSTEM_INFO sysInfo;
   GetSystemInfo(&sysInfo);
   nthreads = sysInfo.dwNumberOfProcessors;
#else
#ifdef __APPLE__
   kern_return_t    kr;
   host_name_port_t   host;
   unsigned int     size;
   struct host_basic_info   hi;

   host = mach_host_self();
   size = sizeof(hi)/sizeof(int);
   kr = host_info(host, HOST_BASIC_INFO, (host_info_t)&hi, &size);
   if (kr != KERN_SUCCESS) {
       mach_error("host_info():", kr);
       exit(EXIT_FAILURE);
   }
   nthreads = hi.avail_cpus;
#else
#ifndef _SC_NPROCESSORS_ONLN
   char line[MAXBUF];
   FILE *f = fopen("/proc/cpuinfo", "r");
   if (NULL == f) goto def;
   nthreads = 0;
   while (fgets(line, MAXBUF, f)) {
      if (!strncmp("processor\t:", line, 11))
         nthreads++;
   }
   fclose(f);
   if (nthreads < 1) goto def;
   return (nthreads);
def:
   fprintf(stderr, "could not read /proc/cpuinfo, using one render thread.\n");
   nthreads = 1;
#else
   nthreads = sysconf(_SC_NPROCESSORS_ONLN);
   if (nthreads < 1) nthreads = 1;
#endif
#endif
#endif
   return (nthreads);
}

static void scan_for_flame_nodes(xmlNode *cur_node, char *parent_file, int default_flag) {

   xmlNode *this_node = NULL;
   size_t f3_storage; //,im_storage;

   /* Loop over this level of elements */
   for (this_node=cur_node; this_node; this_node = this_node->next) {

      /* Check to see if this element is a <flame> element */
      if (this_node->type == XML_ELEMENT_NODE && !xmlStrcmp(this_node->name, (const xmlChar *)"flame")) {

         /* This is a flame element.  Parse it. */
         clear_cp(&xml_current_cp, default_flag);

         parse_flame_element(this_node);

         /* Copy this cp into the array */
         f3_storage = (1+xml_all_ncps)*sizeof(flam3_genome);
         xml_all_cp = realloc(xml_all_cp, f3_storage);
         /* Clear out the realloc'd memory */
         memset(&(xml_all_cp[xml_all_ncps]),0,sizeof(flam3_genome));

         if (xml_current_cp.palette_index != flam3_palette_random) {
            flam3_get_palette(xml_current_cp.palette_index, xml_current_cp.palette,
               xml_current_cp.hue_rotation);
         }

         xml_current_cp.genome_index = xml_all_ncps;
         memset(xml_current_cp.parent_fname, 0, flam3_parent_fn_len);
         strncpy(xml_current_cp.parent_fname,parent_file,flam3_parent_fn_len-1);

         flam3_copy(&(xml_all_cp[xml_all_ncps]), &xml_current_cp);
         xml_all_ncps ++;

/*      } else if (this_node->type == XML_ELEMENT_NODE && !xmlStrcmp(this_node->name, (const xmlChar *)"image")) {

         im_storage = (1+xml_num_images)*sizeof(flam3_image_store);
         im_store = realloc(im_store, im_storage);
         parse_image_element(this_node);
         xml_num_images++;

      }*/
      } else {
         /* Check all of the children of this element */
         scan_for_flame_nodes(this_node->children, parent_file, default_flag);
      }
   }
}

flam3_genome *flam3_parse_xml2(char *xmldata, char *xmlfilename, int default_flag, int *ncps) {

   xmlDocPtr doc; /* Parsed XML document tree */
   xmlNode *rootnode;
   char *bn;
   int i;

   /* Parse XML string into internal document */
   /* Forbid network access during read       */
   doc = xmlReadMemory(xmldata, (int)strlen(xmldata), xmlfilename, NULL, XML_PARSE_NONET);

   /* Check for errors */
   if (doc==NULL) {
      fprintf(stderr, "Failed to parse %s\n", xmlfilename);
      return NULL;
   }

   /* What is the root node of the document? */
   rootnode = xmlDocGetRootElement(doc);

   /* Scan for <flame> nodes, starting with this node */
   xml_all_cp = NULL;
   xml_all_ncps = 0;
   /*xml_num_images = 0;*/

   bn = basename(xmlfilename);
   scan_for_flame_nodes(rootnode, bn, default_flag);

   xmlFreeDoc(doc);

#if 0
   /* We must check here to see if there are any image variations present */
   /* and if the proper image xml was read in to support it               */

   /* First, check to see if there are duplicate image ids */
   /* or missing ids */
   if (xml_num_images>1) {
      for (i=0; i<xml_num_images-1; i++) {
         for (j=i+1; j<xml_num_images; j++) {
            if (im_store[i].id == im_store[j].id) {
               fprintf(stderr,"Error: Duplicate image ids in flame file (%d).  Aborting.\n",im_store[i].id);
               exit(1);
            }
            if (im_store[i].id == -1) {
               fprintf(stderr,"Error: Image id missing.  Is the attr spelled correctly?  Aborting.\n");
               exit(1);
            }

         }
      }
   }

   /* Next, make sure that for all image variations, an available image is specified */
   for (i=0; i<xml_all_ncps; i++) {

      for (j=0; j<xml_all_cp[i].num_xforms; j++) {

         if (xml_all_cp[i].xform[j].var[39] > 0) {

            for (k=0; k<xml_num_images; k++) {

               if (im_store[k].id == xml_all_cp[i].xform[j].image_id) {

                  int sz;

                  /* Copy the im_store to the xform for iteration processing */
                  sz = sizeof(flam3_image_store);
                  xml_all_cp[i].xform[j].image_storage = (flam3_image_store *)malloc(sz);
                  memcpy(xml_all_cp[i].xform[j].image_storage,&(im_store[k]), sz);
                  /* Don't forget the rowcols */

                  sz = 2 * sizeof(unsigned short)*im_store[k].width*im_store[k].height;
                  xml_all_cp[i].xform[j].image_storage->rowcols = (unsigned short *)malloc(sz);
                  memcpy(xml_all_cp[i].xform[j].image_storage->rowcols,im_store[k].rowcols, sz);
                  break;
               }
            }
         }
      }
   }
   /* Free the image data that is no longer necessary */
   for (i=0; i<xml_num_images; i++) {
      free(im_store[i].rowcols);
   }
   free(im_store);

#endif

   *ncps = xml_all_ncps;
   /* Free the allocated string */
   free(xmldata);

   /* Finally, ensure that consecutive 'rotate' parameters never exceed */
   /* a difference of more than 180 degrees (+/-) for interpolation.    */
   /* An adjustment of +/- 360 degrees is made until this is true.      */
   if (*ncps>1) {

      for (i=1;i<*ncps;i++) {

         /* Only do this adjustment if we're not in compat mode */
         if (flam3_inttype_compat != xml_all_cp[i].interpolation_type
	     && flam3_inttype_older != xml_all_cp[i].interpolation_type) {

            while (xml_all_cp[i].rotate < xml_all_cp[i-1].rotate-180)
               xml_all_cp[i].rotate += 360;

            while (xml_all_cp[i].rotate > xml_all_cp[i-1].rotate+180)
               xml_all_cp[i].rotate -= 360;
         }
      }
   }

   return xml_all_cp;
}

flam3_genome * flam3_parse_from_file(FILE *f, char *fname, int default_flag, int *ncps) {
   int i, c, slen = 5000;
   char *s;

   /* Incrementally read XML file into a string */
   s = malloc(slen);
   i = 0;
   do {
      c = getc(f);
      if (EOF == c)
         break;
      s[i++] = c;
      if (i == slen-1) {
         slen *= 2;
         s = realloc(s, slen);
      }
   } while (1);

   /* Null-terminate the read XML data */
   s[i] = 0;

   /* Parse the XML string */
   if (fname) {
      return flam3_parse_xml2(s, fname, default_flag, ncps);
   } else
      return flam3_parse_xml2(s, "stdin", default_flag, ncps);

}

static void flam3_edit_print(FILE *f, xmlNodePtr editNode, int tabs, int formatting) {

   char *tab_string = "   ";
   int ti,strl;
   xmlAttrPtr att_ptr=NULL,cur_att=NULL;
   xmlNodePtr chld_ptr=NULL, cur_chld=NULL;
   int edit_or_sheep = 0, indent_printed = 0;
   char *ai;
   int tablim = argi("print_edit_depth",0);

   char *att_str,*cont_str,*cpy_string;

   if (tablim>0 && tabs>tablim)
	return;

   /* If this node is an XML_ELEMENT_NODE, print it and it's attributes */
   if (editNode->type==XML_ELEMENT_NODE) {

      /* Print the node at the tab specified */
      if (formatting) {
         for (ti=0;ti<tabs;ti++)
            fprintf(f,"%s",tab_string);
      }

      fprintf(f,"<%s",editNode->name);

      /* This can either be an edit node or a sheep node */
      /* If it's an edit node, add one to the tab        */
      if (!xmlStrcmp(editNode->name, (const xmlChar *)"edit")) {
         edit_or_sheep = 1;
         tabs ++;
      } else if (!xmlStrcmp(editNode->name, (const xmlChar *)"sheep"))
         edit_or_sheep = 2;
      else
         edit_or_sheep = 0;


      /* Print the attributes */
      att_ptr = editNode->properties;

      for (cur_att = att_ptr; cur_att; cur_att = cur_att->next) {

         att_str = (char *) xmlGetProp(editNode,cur_att->name);
         fprintf(f," %s=\"%s\"",cur_att->name,att_str);
         xmlFree(att_str);
      }

      /* Does this node have children? */
      if (!editNode->children || (tablim>0 && tabs>tablim)) {
         /* Close the tag and subtract the tab */
         fprintf(f,"/>");
         if (formatting)
            fprintf(f,"\n");
         tabs--;
      } else {

         /* Close the tag */
         fprintf(f,">");

         if (formatting)
            fprintf(f,"\n");

         /* Loop through the children and print them */
         chld_ptr = editNode->children;

         indent_printed = 0;

         for (cur_chld=chld_ptr; cur_chld; cur_chld = cur_chld->next) {

            /* If child is an element, indent first and then print it. */
            if (cur_chld->type==XML_ELEMENT_NODE &&
               (!xmlStrcmp(cur_chld->name, (const xmlChar *)"edit") ||
      (!xmlStrcmp(cur_chld->name, (const xmlChar *)"sheep")))) {

               if (indent_printed) {
                  indent_printed = 0;
                  fprintf(f,"\n");
               }

               flam3_edit_print(f, cur_chld, tabs, 1);

            } else {

               /* Child is a text node.  We don't want to indent more than once. */
               if (xmlIsBlankNode(cur_chld))
                  continue;

               if (indent_printed==0 && formatting==1) {
                  for (ti=0;ti<tabs;ti++)
                     fprintf(f,"%s",tab_string);
                  indent_printed = 1;
               }

               /* Print nodes without formatting. */
               flam3_edit_print(f, cur_chld, tabs, 0);

            }
         }

         if (indent_printed && formatting)
            fprintf(f,"\n");

         /* Tab out. */
         tabs --;
         if (formatting) {
            for (ti=0;ti<tabs;ti++)
               fprintf(f,"%s",tab_string);
         }

         /* Close the tag */
         fprintf(f,"</%s>",editNode->name);

         if (formatting) {
            fprintf(f,"\n");
         }
      }

   } else if (editNode->type==XML_TEXT_NODE) {

      /* Print text node */
      cont_str = (char *) xmlNodeGetContent(editNode);
      cpy_string = &(cont_str[0]);
      while (isspace(*cpy_string))
         cpy_string++;

      strl = (int)strlen(cont_str)-1;

      while (isspace(cont_str[strl]))
         strl--;

      cont_str[strl+1] = 0;

      fprintf(f,"%s",cpy_string);

   }
}

void flam3_apply_template(flam3_genome *cp, flam3_genome *templ) {

   /* Check for invalid values - only replace those with valid ones */
   if (templ->background[0] >= 0)
      cp->background[0] = templ->background[0];
   if (templ->background[1] >= 0)
      cp->background[1] = templ->background[1];
   if (templ->background[1] >= 0)
      cp->background[2] = templ->background[2];
   if (templ->zoom < 999999998)
      cp->zoom = templ->zoom;
   if (templ->spatial_oversample > 0)
      cp->spatial_oversample = templ->spatial_oversample;
   if (templ->spatial_filter_radius >= 0)
      cp->spatial_filter_radius = templ->spatial_filter_radius;
   if (templ->sample_density > 0)
      cp->sample_density = templ->sample_density;
   if (templ->nbatches > 0)
      cp->nbatches = templ->nbatches;
   if (templ->ntemporal_samples > 0)
      cp->ntemporal_samples = templ->ntemporal_samples;
   if (templ->width > 0) {
      /* preserving scale should be an option */
      cp->pixels_per_unit = cp->pixels_per_unit * templ->width / cp->width;
      cp->width = templ->width;
   }
   if (templ->height > 0)
      cp->height = templ->height;
   if (templ->estimator >= 0)
      cp->estimator = templ->estimator;
   if (templ->estimator_minimum >= 0)
      cp->estimator_minimum = templ->estimator_minimum;
   if (templ->estimator_curve >= 0)
      cp->estimator_curve = templ->estimator_curve;
   if (templ->gam_lin_thresh >= 0)
      cp->gam_lin_thresh = templ->gam_lin_thresh;
//   if (templ->motion_exp != -999)
//      cp->motion_exp = templ->motion_exp;
   if (templ->nbatches>0)
      cp->nbatches = templ->nbatches;
   if (templ->ntemporal_samples>0)
      cp->ntemporal_samples = templ->ntemporal_samples;
   if (templ->spatial_filter_select>0) {
      cp->spatial_filter_select = templ->spatial_filter_select;
   }
   if (templ->interpolation_type >= 0)
      cp->interpolation_type = templ->interpolation_type;
   if (templ->temporal_filter_type >= 0)
      cp->temporal_filter_type = templ->temporal_filter_type;
   if (templ->temporal_filter_width > 0)
      cp->temporal_filter_width = templ->temporal_filter_width;
   if (templ->temporal_filter_exp > -900)
      cp->temporal_filter_exp = templ->temporal_filter_width;

}

char *flam3_print_to_string(flam3_genome *cp) {

   FILE *tmpflame;
   long stringbytes;
   char *genome_string;

   tmpflame = tmpfile();
   flam3_print(tmpflame,cp,NULL,flam3_dont_print_edits);
   stringbytes = ftell(tmpflame);
   fseek(tmpflame,0L, SEEK_SET);
   genome_string = (char *)calloc(stringbytes+1,1);
   fread(genome_string, 1, stringbytes, tmpflame);
   fclose(tmpflame);

   return(genome_string);
}


void flam3_print(FILE *f, flam3_genome *cp, char *extra_attributes, int print_edits) {
   int i, j;

   fprintf(f, "<flame time=\"%g\"", cp->time);

   if (cp->flame_name[0]!=0)
      fprintf(f, " name=\"%s\"",cp->flame_name);

   fprintf(f, " size=\"%d %d\"", cp->width, cp->height);
   fprintf(f, " center=\"%g %g\"", cp->center[0], cp->center[1]);
   fprintf(f, " scale=\"%g\"", cp->pixels_per_unit);

   if (cp->zoom != 0.0)
      fprintf(f, " zoom=\"%g\"", cp->zoom);

   fprintf(f, " rotate=\"%g\"", cp->rotate);
   fprintf(f, " supersample=\"%d\"", cp->spatial_oversample);
   fprintf(f, " filter=\"%g\"", cp->spatial_filter_radius);

   /* Need to print the correct kernel to use */
   if (cp->spatial_filter_select == flam3_gaussian_kernel)
      fprintf(f, " filter_shape=\"gaussian\"");
   else if (cp->spatial_filter_select == flam3_hermite_kernel)
      fprintf(f, " filter_shape=\"hermite\"");
   else if (cp->spatial_filter_select == flam3_box_kernel)
      fprintf(f, " filter_shape=\"box\"");
   else if (cp->spatial_filter_select == flam3_triangle_kernel)
      fprintf(f, " filter_shape=\"triangle\"");
   else if (cp->spatial_filter_select == flam3_bell_kernel)
      fprintf(f, " filter_shape=\"bell\"");
   else if (cp->spatial_filter_select == flam3_b_spline_kernel)
      fprintf(f, " filter_shape=\"bspline\"");
   else if (cp->spatial_filter_select == flam3_mitchell_kernel)
      fprintf(f, " filter_shape=\"mitchell\"");
   else if (cp->spatial_filter_select == flam3_blackman_kernel)
      fprintf(f, " filter_shape=\"blackman\"");
   else if (cp->spatial_filter_select == flam3_catrom_kernel)
      fprintf(f, " filter_shape=\"catrom\"");
   else if (cp->spatial_filter_select == flam3_hanning_kernel)
      fprintf(f, " filter_shape=\"hanning\"");
   else if (cp->spatial_filter_select == flam3_hamming_kernel)
      fprintf(f, " filter_shape=\"hamming\"");
   else if (cp->spatial_filter_select == flam3_lanczos3_kernel)
      fprintf(f, " filter_shape=\"lanczos3\"");
   else if (cp->spatial_filter_select == flam3_lanczos2_kernel)
      fprintf(f, " filter_shape=\"lanczos2\"");
   else if (cp->spatial_filter_select == flam3_quadratic_kernel)
      fprintf(f, " filter_shape=\"quadratic\"");

   if (cp->temporal_filter_type == flam3_temporal_box)
      fprintf(f, " temporal_filter_type=\"box\"");
   else if (cp->temporal_filter_type == flam3_temporal_gaussian)
      fprintf(f, " temporal_filter_type=\"gaussian\"");
   else if (cp->temporal_filter_type == flam3_temporal_exp)
      fprintf(f, " temporal_filter_type=\"exp\" temporal_filter_exp=\"%g\"",cp->temporal_filter_exp);

   fprintf(f, " temporal_filter_width=\"%g\"",cp->temporal_filter_width);



   fprintf(f, " quality=\"%g\"", cp->sample_density);
   fprintf(f, " passes=\"%d\"", cp->nbatches);
   fprintf(f, " temporal_samples=\"%d\"", cp->ntemporal_samples);
   fprintf(f, " background=\"%g %g %g\"",
      cp->background[0], cp->background[1], cp->background[2]);
   fprintf(f, " brightness=\"%g\"", cp->brightness);
   fprintf(f, " gamma=\"%g\"", cp->gamma);
   fprintf(f, " vibrancy=\"%g\"", cp->vibrancy);
   fprintf(f, " estimator_radius=\"%g\" estimator_minimum=\"%g\" estimator_curve=\"%g\"",
      cp->estimator, cp->estimator_minimum, cp->estimator_curve);
   fprintf(f, " gamma_threshold=\"%g\"", cp->gam_lin_thresh);

   if (flam3_interpolation_linear != cp->interpolation)
       fprintf(f, " interpolation=\"smooth\"");

   if (flam3_inttype_linear == cp->interpolation_type)
       fprintf(f, " interpolation_type=\"linear\"");
   else if (flam3_inttype_log == cp->interpolation_type)
       fprintf(f, " interpolation_type=\"log\"");
   else if (flam3_inttype_compat == cp->interpolation_type)
       fprintf(f, " interpolation_type=\"old\"");
   else if (flam3_inttype_older == cp->interpolation_type)
       fprintf(f, " interpolation_type=\"older\"");


   if (flam3_palette_interpolation_hsv != cp->palette_interpolation)
       fprintf(f, " palette_interpolation=\"sweep\"");

//   if (cp->motion_exp != 0.0)
//      fprintf(f, " motion_exponent=\"%g\"", cp->motion_exp);


   if (extra_attributes)
      fprintf(f, " %s", extra_attributes);

   fprintf(f, ">\n");

   if (cp->symmetry)
      fprintf(f, "   <symmetry kind=\"%d\"/>\n", cp->symmetry);
   for (i = 0; i < cp->num_xforms; i++) {
      int blob_var=0,pdj_var=0,fan2_var=0,rings2_var=0,perspective_var=0;
      int juliaN_var=0,juliaScope_var=0,radialBlur_var=0,pie_var=0,disc2_var=0;
      int ngon_var=0,curl_var=0,rectangles_var=0,supershape_var=0;
      int flower_var=0,conic_var=0,parabola_var=0;
//      if ( (cp->xform[i].density > 0.0 || i==cp->final_xform_index)
//             && !(cp->symmetry &&  cp->xform[i].symmetry == 1.0)) {
      if ( !(cp->symmetry &&  cp->xform[i].symmetry == 1.0)) {

         if (i==cp->final_xform_index )
            fprintf(f, "   <finalxform color=\"%g", cp->xform[i].color[0]);
         else
            fprintf(f, "   <xform weight=\"%g\" color=\"%g", cp->xform[i].density, cp->xform[i].color[0]);

         if (0 && 0.0 != cp->xform[i].color[1]) {
            fprintf(f, " %g\" ", cp->xform[i].color[1]);
         } else {
            fprintf(f, "\" ");
         }

         fprintf(f, "symmetry=\"%g\" ", cp->xform[i].symmetry);

         for (j = 0; j < flam3_nvariations; j++) {
            double v = cp->xform[i].var[j];
            if (0.0 != v) {
               fprintf(f, "%s=\"%g\" ", flam3_variation_names[j], v);
               if (j==VAR_BLOB)
                  blob_var=1;
               else if (j==VAR_PDJ)
                  pdj_var=1;
               else if (j==VAR_FAN2)
                  fan2_var=1;
               else if (j==VAR_RINGS2)
                  rings2_var=1;
               else if (j==VAR_PERSPECTIVE)
                  perspective_var=1;
               else if (j==VAR_JULIAN)
                  juliaN_var=1;
               else if (j==VAR_JULIASCOPE)
                  juliaScope_var=1;
               else if (j==VAR_RADIAL_BLUR)
                  radialBlur_var=1;
               else if (j==VAR_PIE)
                  pie_var=1;
               else if (j==VAR_NGON)
                  ngon_var=1;
               else if (j==VAR_CURL)
                  curl_var=1;
               else if (j==VAR_RECTANGLES)
                  rectangles_var=1;
               else if (j==VAR_DISC2)
                  disc2_var=1;
               else if (j==VAR_SUPER_SHAPE)
                  supershape_var=1;
               else if (j==VAR_FLOWER)
                  flower_var=1;
               else if (j==VAR_CONIC)
                  conic_var=1;
               else if (j==VAR_PARABOLA)
                  parabola_var=1;
            }
         }

         if (blob_var==1) {
            fprintf(f, "blob_low=\"%g\" ", cp->xform[i].blob_low);
            fprintf(f, "blob_high=\"%g\" ", cp->xform[i].blob_high);
            fprintf(f, "blob_waves=\"%g\" ", cp->xform[i].blob_waves);
         }

         if (pdj_var==1) {
            fprintf(f, "pdj_a=\"%g\" ", cp->xform[i].pdj_a);
            fprintf(f, "pdj_b=\"%g\" ", cp->xform[i].pdj_b);
            fprintf(f, "pdj_c=\"%g\" ", cp->xform[i].pdj_c);
            fprintf(f, "pdj_d=\"%g\" ", cp->xform[i].pdj_d);
         }

         if (fan2_var==1) {
            fprintf(f, "fan2_x=\"%g\" ", cp->xform[i].fan2_x);
            fprintf(f, "fan2_y=\"%g\" ", cp->xform[i].fan2_y);
         }

         if (rings2_var==1) {
            fprintf(f, "rings2_val=\"%g\" ", cp->xform[i].rings2_val);
         }

         if (perspective_var==1) {
            fprintf(f, "perspective_angle=\"%g\" ", cp->xform[i].perspective_angle);
            fprintf(f, "perspective_dist=\"%g\" ", cp->xform[i].perspective_dist);
         }

         if (juliaN_var==1) {
            fprintf(f, "julian_power=\"%g\" ", cp->xform[i].juliaN_power);
            fprintf(f, "julian_dist=\"%g\" ", cp->xform[i].juliaN_dist);
         }

         if (juliaScope_var==1) {
            fprintf(f, "juliascope_power=\"%g\" ", cp->xform[i].juliaScope_power);
            fprintf(f, "juliascope_dist=\"%g\" ", cp->xform[i].juliaScope_dist);
         }

         if (radialBlur_var==1) {
            fprintf(f, "radial_blur_angle=\"%g\" ", cp->xform[i].radialBlur_angle);
         }

         if (pie_var==1) {
            fprintf(f, "pie_slices=\"%g\" ", cp->xform[i].pie_slices);
            fprintf(f, "pie_rotation=\"%g\" ", cp->xform[i].pie_rotation);
            fprintf(f, "pie_thickness=\"%g\" ", cp->xform[i].pie_thickness);
         }

         if (ngon_var==1) {
            fprintf(f, "ngon_sides=\"%g\" ", cp->xform[i].ngon_sides);
            fprintf(f, "ngon_power=\"%g\" ", cp->xform[i].ngon_power);
            fprintf(f, "ngon_corners=\"%g\" ", cp->xform[i].ngon_corners);
            fprintf(f, "ngon_circle=\"%g\" ", cp->xform[i].ngon_circle);
         }

         if (curl_var==1) {
            fprintf(f, "curl_c1=\"%g\" ", cp->xform[i].curl_c1);
            fprintf(f, "curl_c2=\"%g\" ", cp->xform[i].curl_c2);
         }

         if (rectangles_var==1) {
            fprintf(f, "rectangles_x=\"%g\" ", cp->xform[i].rectangles_x);
            fprintf(f, "rectangles_y=\"%g\" ", cp->xform[i].rectangles_y);
         }
/*
         if (amw_var==1) {
            fprintf(f, "amw_amp=\"%g\" ", cp->xform[i].amw_amp);
         }
*/
         if (disc2_var==1) {
            fprintf(f, "disc2_rot=\"%g\" ", cp->xform[i].disc2_rot);
            fprintf(f, "disc2_twist=\"%g\" ", cp->xform[i].disc2_twist);
         }

         if (supershape_var==1) {
            fprintf(f, "super_shape_rnd=\"%g\" ", cp->xform[i].supershape_rnd);
            fprintf(f, "super_shape_m=\"%g\" ", cp->xform[i].supershape_m);
            fprintf(f, "super_shape_n1=\"%g\" ", cp->xform[i].supershape_n1);
            fprintf(f, "super_shape_n2=\"%g\" ", cp->xform[i].supershape_n2);
            fprintf(f, "super_shape_n3=\"%g\" ", cp->xform[i].supershape_n3);
            fprintf(f, "super_shape_holes=\"%g\" ", cp->xform[i].supershape_holes);
         }

         if (flower_var==1) {
            fprintf(f, "flower_petals=\"%g\" ", cp->xform[i].flower_petals);
            fprintf(f, "flower_holes=\"%g\" ", cp->xform[i].flower_holes);
         }

         if (conic_var==1) {
            fprintf(f, "conic_eccentricity=\"%g\" ", cp->xform[i].conic_eccen);
            fprintf(f, "conic_holes=\"%g\" ", cp->xform[i].conic_holes);
         }

         if (parabola_var==1) {
            fprintf(f, "parabola_height=\"%g\" ", cp->xform[i].parabola_height);
            fprintf(f, "parabola_width=\"%g\" ", cp->xform[i].parabola_width);
         }
#if 0
         if (split_var==1) {
            fprintf(f, "split_xsize=\"%g\" ", cp->xform[i].split_xsize);
            fprintf(f, "split_ysize=\"%g\" ", cp->xform[i].split_ysize);
            fprintf(f, "split_shift=\"%g\" ", cp->xform[i].split_shift);
         }

         if (move_var==1) {
            fprintf(f, "move_x=\"%g\" ", cp->xform[i].move_x);
            fprintf(f, "move_y=\"%g\" ", cp->xform[i].move_y);
         }
#endif
         fprintf(f, "coefs=\"");
         for (j = 0; j < 3; j++) {
            if (j) fprintf(f, " ");
            fprintf(f, "%g %g", cp->xform[i].c[j][0], cp->xform[i].c[j][1]);
         }
         fprintf(f, "\"");
         if (!id_matrix(cp->xform[i].post)) {
            fprintf(f, " post=\"");
            for (j = 0; j < 3; j++) {
               if (j) fprintf(f, " ");
               fprintf(f, "%g %g", cp->xform[i].post[j][0], cp->xform[i].post[j][1]);
            }
            fprintf(f, "\"");
         }
         fprintf(f, "/>\n");

      }
   }

   for (i = 0; i < 256; i++) {
      double r, g, b;
      r = (cp->palette[i][0] * 255.0);
      g = (cp->palette[i][1] * 255.0);
      b = (cp->palette[i][2] * 255.0);
      if (getenv("intpalette"))
         fprintf(f, "   <color index=\"%d\" rgb=\"%d %d %d\"/>\n", i, (int)rint(r), (int)rint(g), (int)rint(b));
      else
         fprintf(f, "   <color index=\"%d\" rgb=\"%.6g %.6g %.6g\"/>\n", i, r, g, b);
   }

   if (cp->edits != NULL && print_edits==flam3_print_edits) {

      /* We need a custom script for printing these */
      /* and it needs to be recursive               */
      xmlNodePtr elem_node = xmlDocGetRootElement(cp->edits);
      flam3_edit_print(f,elem_node, 1, 1);
   }
   fprintf(f, "</flame>\n");

}

/* returns a uniform variable from 0 to 1 */
double flam3_random01() {
   return (random() & 0xfffffff) / (double) 0xfffffff;
}

double flam3_random11() {
   return ((random() & 0xfffffff) - 0x7ffffff) / (double) 0x7ffffff;
}

/* This function must be called prior to rendering a frame */
void flam3_init_frame(flam3_frame *f) {

   char *ai;
   char *isaac_seed = args("isaac_seed",NULL);
   long int default_isaac_seed = (long int)time(0);

   /* Clear out the isaac state */
   memset(f->rc.randrsl, 0, RANDSIZ*sizeof(ub4));

   /* Set the isaac seed */
   if (NULL == isaac_seed) {
      int lp;
      /* No isaac seed specified.  Use the system time to initialize. */
      for (lp = 0; lp < RANDSIZ; lp++)
         f->rc.randrsl[lp] = default_isaac_seed;
   } else {
      /* Use the specified string */
      strncpy((char *)&f->rc.randrsl,(const char *)isaac_seed, RANDSIZ*sizeof(ub4));
   }

   /* Initialize the random number generator */
   irandinit(&f->rc,1);
}

/* returns uniform variable from ISAAC rng */
double flam3_random_isaac_01(randctx *ct) {
   return ((int)irand(ct) & 0xfffffff) / (double) 0xfffffff;
}

double flam3_random_isaac_11(randctx *ct) {
   return (((int)irand(ct) & 0xfffffff) - 0x7ffffff) / (double) 0x7ffffff;
}

int flam3_random_bit() {
  /* might not be threadsafe */
  static int n = 0;
  static int l;
  if (0 == n) {
    l = random();
    n = 20;
  } else {
    l = l >> 1;
    n--;
  }
  return l & 1;
}

int flam3_random_isaac_bit(randctx *ct) {
   int tmp = irand(ct);
   return tmp & 1;
}

void flam3_parse_hexformat_colors(char *colstr, flam3_genome *cp, int numcolors, int chan) {

   int c_idx=0;
   int col_count=0;
   int r,g,b;
   int sscanf_ret;

   /* Strip whitespace prior to first color */
   while (isspace( (int)colstr[c_idx]))
      c_idx++;

   do {

      /* Parse an RGB triplet at a time... */
      if (chan==3)
         sscanf_ret = sscanf(&(colstr[c_idx]),"%2x%2x%2x",&r,&g,&b);
      else
         sscanf_ret = sscanf(&(colstr[c_idx]),"00%2x%2x%2x",&r,&g,&b);

      if (sscanf_ret != 3) {
         fprintf(stderr, "Error:  Problem reading hexadecimal color data.\n");
         exit(1);
      }

      c_idx += 2*chan;

      while (isspace( (int)colstr[c_idx]))
         c_idx++;

      cp->palette[col_count][0] = r / 255.0;
      cp->palette[col_count][1] = g / 255.0;
      cp->palette[col_count][2] = b / 255.0;

      col_count++;

   } while (col_count<numcolors);
}

/* sum of entries of vector to 1 */
static int normalize_vector(double *v, int n) {
    double t = 0.0;
    int i;
    for (i = 0; i < n; i++)
   t += v[i];
    if (0.0 == t) return 1;
    t = 1.0 / t;
    for (i = 0; i < n; i++)
   v[i] *= t;
    return 0;
}



static double round6(double x) {
  x *= 1e6;
  if (x < 0) x -= 1.0;
  return 1e-6*(int)(x+0.5);
}

/* sym=2 or more means rotational
   sym=1 means identity, ie no symmetry
   sym=0 means pick a random symmetry (maybe none)
   sym=-1 means bilateral (reflection)
   sym=-2 or less means rotational and reflective
*/
void flam3_add_symmetry(flam3_genome *cp, int sym) {
   int i, j, k;
   double a;
   int result = 0;

   if (0 == sym) {
      static int sym_distrib[] = {
         -4, -3,
         -2, -2, -2,
         -1, -1, -1,
         2, 2, 2,
         3, 3,
         4, 4,
      };
      if (random()&1) {
         sym = random_distrib(sym_distrib);
      } else if (random()&31) {
         sym = (random()%13)-6;
      } else {
         sym = (random()%51)-25;
      }
   }

   if (1 == sym || 0 == sym) return;

   cp->symmetry = sym;

   if (sym < 0) {

      i = cp->num_xforms;
      flam3_add_xforms(cp,1,0);

      cp->xform[i].density = 1.0;
      cp->xform[i].symmetry = 1.0;
      cp->xform[i].var[0] = 1.0;
      for (j = 1; j < flam3_nvariations; j++)
         cp->xform[i].var[j] = 0;
      cp->xform[i].color[0] = 1.0;
      cp->xform[i].color[1] = 1.0;
      cp->xform[i].c[0][0] = -1.0;
      cp->xform[i].c[0][1] = 0.0;
      cp->xform[i].c[1][0] = 0.0;
      cp->xform[i].c[1][1] = 1.0;
      cp->xform[i].c[2][0] = 0.0;
      cp->xform[i].c[2][1] = 0.0;

      result++;
      sym = -sym;
   }

   a = 2*M_PI/sym;

   for (k = 1; k < sym; k++) {

      i = cp->num_xforms;
      flam3_add_xforms(cp, 1, 0);

      cp->xform[i].density = 1.0;
      cp->xform[i].var[0] = 1.0;
      cp->xform[i].symmetry = 1.0;
      for (j = 1; j < flam3_nvariations; j++)
         cp->xform[i].var[j] = 0;
      cp->xform[i].color[1] = /* XXX */
      cp->xform[i].color[0] = (sym<3) ? 0.0 : ((k-1.0)/(sym-2.0));
      cp->xform[i].c[0][0] = round6(cos(k*a));
      cp->xform[i].c[0][1] = round6(sin(k*a));
      cp->xform[i].c[1][0] = round6(-cp->xform[i].c[0][1]);
      cp->xform[i].c[1][1] = cp->xform[i].c[0][0];
      cp->xform[i].c[2][0] = 0.0;
      cp->xform[i].c[2][1] = 0.0;

      result++;
   }

   qsort((char *) &cp->xform[cp->num_xforms-result], result,
      sizeof(flam3_xform), compare_xforms);

}

static int random_var() {
  return random() % flam3_nvariations;
}

static int random_varn(int n) {
   return random() % n;
}

void flam3_random(flam3_genome *cp, int *ivars, int ivars_n, int sym, int spec_xforms) {

   /** NEED TO ADD FINAL XFORM TO RANDOMNESS **/
   int i, nxforms, var, samed, multid, samepost, postid, addfinal=0;
   int finum = -1;
   int n;
   double sum;

   static int xform_distrib[] = {
     2, 2, 2, 2,
     3, 3, 3, 3,
     4, 4, 4,
     5, 5,
     6
   };

   clear_cp(cp,flam3_defaults_on);

   cp->hue_rotation = (random()&7) ? 0.0 : flam3_random01();
   cp->palette_index = flam3_get_palette(flam3_palette_random, cp->palette, cp->hue_rotation);
   cp->time = 0.0;
   cp->interpolation = flam3_interpolation_linear;
   cp->palette_interpolation = flam3_palette_interpolation_hsv;

   /* Choose the number of xforms */
   if (spec_xforms>0)
      nxforms = spec_xforms;
   else {
      nxforms = random_distrib(xform_distrib);
      /* Add a final xform 15% of the time */
      addfinal = flam3_random01() < 0.15;
      if (addfinal) {
         nxforms = nxforms + addfinal;
         cp->final_xform_enable=1;
         cp->final_xform_index = nxforms-1;
         finum = nxforms-1;
      }
   }

   flam3_add_xforms(cp,nxforms,0);

   /* If first input variation is 'flam3_variation_random' */
   /* choose one to use or decide to use multiple    */
   if (flam3_variation_random == ivars[0]) {
      if (flam3_random_bit()) {
         var = random_var();
      } else {
         var = flam3_variation_random;
      }
   } else {
      var = flam3_variation_random_fromspecified;
   }

   samed = flam3_random_bit();
   multid = flam3_random_bit();
   postid = flam3_random01() < 0.6;
   samepost = flam3_random_bit();

   /* Loop over xforms */
   for (i = 0; i < nxforms; i++) {
      int j, k;
      cp->xform[i].density = 1.0 / nxforms;
      cp->xform[i].color[0] = i&1;
      cp->xform[i].color[1] = (i&2)>>1;
      cp->xform[i].symmetry = 0.0;
      for (j = 0; j < 3; j++) {
         for (k = 0; k < 2; k++) {
            cp->xform[i].c[j][k] = flam3_random11();
            cp->xform[i].post[j][k] = (double)(k==j);
         }
      }

      if ( i != finum ) {

         if (!postid) {

            for (j = 0; j < 3; j++)
            for (k = 0; k < 2; k++) {
               if (samepost || (i==0))
                  cp->xform[i].post[j][k] = flam3_random11();
               else
                  cp->xform[i].post[j][k] = cp->xform[0].post[j][k];
            }
         }

         /* Clear all variation coefs */
         for (j = 0; j < flam3_nvariations; j++)
            cp->xform[i].var[j] = 0.0;

         if (flam3_variation_random != var &&
               flam3_variation_random_fromspecified != var) {

            /* Use only one variation specified for all xforms */
            cp->xform[i].var[var] = 1.0;

         } else if (multid && flam3_variation_random == var) {

           /* Choose a random var for this xform */
             cp->xform[i].var[random_var()] = 1.0;

         } else {

            if (samed && i > 0) {

               /* Copy the same variations from the previous xform */
               for (j = 0; j < flam3_nvariations; j++) {
                  cp->xform[i].var[j] = cp->xform[i-1].var[j];
                  flam3_copy_params(&(cp->xform[i]),&(cp->xform[i-1]),j);
               }

            } else {

               /* Choose a random number of vars to use, at least 2 */
               /* but less than flam3_nvariations.Probability leans */
               /* towards fewer variations.                         */
               n = 2;
               while ((flam3_random_bit()) && (n<flam3_nvariations))
                  n++;

               /* Randomly choose n variations, and change their weights. */
               /* A var can be selected more than once, further reducing  */
               /* the probability that multiple vars are used.            */
               for (j = 0; j < n; j++) {
                  if (flam3_variation_random_fromspecified != var)
                     cp->xform[i].var[random_var()] = flam3_random01();
                  else
                     cp->xform[i].var[ivars[random_varn(ivars_n)]] = flam3_random01();
               }

               /* Normalize weights to 1.0 total. */
               sum = 0.0;
               for (j = 0; j < flam3_nvariations; j++)
                  sum += cp->xform[i].var[j];
               if (sum == 0.0)
                  cp->xform[i].var[random_var()] = 1.0;
               else {
                  for (j = 0; j < flam3_nvariations; j++)
                     cp->xform[i].var[j] /= sum;
               }
            }
         }
      } else {
         /* Handle final xform randomness. */
         n = 1;
         if (flam3_random_bit()) n++;

         /* Randomly choose n variations, and change their weights. */
         /* A var can be selected more than once, further reducing  */
         /* the probability that multiple vars are used.            */
         for (j = 0; j < n; j++) {
            if (flam3_variation_random_fromspecified != var)
               cp->xform[i].var[random_var()] = flam3_random01();
            else
               cp->xform[i].var[ivars[random_varn(ivars_n)]] = flam3_random01();
         }

         /* Normalize weights to 1.0 total. */
         sum = 0.0;
         for (j = 0; j < flam3_nvariations; j++)
            sum += cp->xform[i].var[j];
         if (sum == 0.0)
            cp->xform[i].var[random_var()] = 1.0;
         else {
            for (j = 0; j < flam3_nvariations; j++)
               cp->xform[i].var[j] /= sum;
         }
      }


      if (cp->xform[i].var[VAR_WAVES] > 0) {
         waves_precalc(&(cp->xform[i]));
      }

      /* Generate random params for parametric variations, if selected. */
      if (cp->xform[i].var[VAR_BLOB] > 0) {
         /* Create random params for blob */
         cp->xform[i].blob_low = 0.2 + 0.5 * flam3_random01();
         cp->xform[i].blob_high = 0.8 + 0.4 * flam3_random01();
         cp->xform[i].blob_waves = (int)(2 + 5 * flam3_random01());
      }

      if (cp->xform[i].var[VAR_PDJ] > 0) {
         /* Create random params for PDJ */
         cp->xform[i].pdj_a = 3.0 * flam3_random11();
         cp->xform[i].pdj_b = 3.0 * flam3_random11();
         cp->xform[i].pdj_c = 3.0 * flam3_random11();
         cp->xform[i].pdj_d = 3.0 * flam3_random11();
      }

      if (cp->xform[i].var[VAR_FAN2] > 0) {
         /* Create random params for fan2 */
         cp->xform[i].fan2_x = flam3_random11();
         cp->xform[i].fan2_y = flam3_random11();
      }

      if (cp->xform[i].var[VAR_RINGS2] > 0) {
         /* Create random params for rings2 */
         cp->xform[i].rings2_val = 2*flam3_random01();
      }

      if (cp->xform[i].var[VAR_PERSPECTIVE] > 0) {

         /* Create random params for perspective */
         cp->xform[i].perspective_angle = flam3_random01();
         cp->xform[i].perspective_dist = 2*flam3_random01() + 1.0;

         /* Calculate the other params from these */
//         perspective_precalc(&(cp->xform[i]));
      }

      if (cp->xform[i].var[VAR_JULIAN] > 0) {

         /* Create random params for juliaN */
         cp->xform[i].juliaN_power = (int)(5*flam3_random01() + 2);
         cp->xform[i].juliaN_dist = 1.0;

         /* Calculate other params from these */
//         juliaN_precalc(&(cp->xform[i]));
      }

      if (cp->xform[i].var[VAR_JULIASCOPE] > 0) {

         /* Create random params for juliaScope */
         cp->xform[i].juliaScope_power = (int)(5*flam3_random01() + 2);
         cp->xform[i].juliaScope_dist = 1.0;

         /* Calculate other params from these */
//         juliaScope_precalc(&(cp->xform[i]));
      }

      if (cp->xform[i].var[VAR_RADIAL_BLUR] > 0) {

         /* Create random params for radialBlur */
         cp->xform[i].radialBlur_angle = (2 * flam3_random01() - 1);

         /* Calculate other params from this */
//         radial_blur_precalc(&(cp->xform[i]));
      }

      if (cp->xform[i].var[VAR_PIE] > 0) {
         /* Create random params for pie */
         cp->xform[i].pie_slices = (int) 10.0*flam3_random01();
         cp->xform[i].pie_thickness = flam3_random01();
         cp->xform[i].pie_rotation = 2.0 * M_PI * flam3_random11();
      }

      if (cp->xform[i].var[VAR_NGON] > 0) {
         /* Create random params for ngon */
         cp->xform[i].ngon_sides = (int) flam3_random01()* 10 + 3;
         cp->xform[i].ngon_power = 3*flam3_random01() + 1;
         cp->xform[i].ngon_circle = 3*flam3_random01();
         cp->xform[i].ngon_corners = 2*flam3_random01()*cp->xform[i].ngon_circle;
      }

      if (cp->xform[i].var[VAR_CURL] > 0) {
         /* Create random params for curl */
         cp->xform[i].curl_c1 = flam3_random01();
         cp->xform[i].curl_c2 = flam3_random01();
      }

      if (cp->xform[i].var[VAR_RECTANGLES] > 0) {
         /* Create random params for rectangles */
         cp->xform[i].rectangles_x = flam3_random01();
         cp->xform[i].rectangles_y = flam3_random01();
      }

      if (cp->xform[i].var[VAR_DISC2] > 0) {
      /* Create random params for disc2 */
      cp->xform[i].disc2_rot = 0.5 * flam3_random01();
      cp->xform[i].disc2_twist = 0.5 * flam3_random01();

         /* Calculate other params */
//         disc2_precalc(&(cp->xform[i]));
      }

      if (cp->xform[i].var[VAR_SUPER_SHAPE] > 0) {
         /* Create random params for supershape */
         cp->xform[i].supershape_rnd = flam3_random01();
         cp->xform[i].supershape_m = (int) flam3_random01()*6;
         cp->xform[i].supershape_n1 = flam3_random01()*40;
         cp->xform[i].supershape_n2 = flam3_random01()*20;
         cp->xform[i].supershape_n3 = cp->xform[i].supershape_n2;
         cp->xform[i].supershape_holes = 0.0;
      }

      if (cp->xform[i].var[VAR_FLOWER] > 0) {
         /* Create random params for flower */
         cp->xform[i].flower_petals = 4 * flam3_random01();
         cp->xform[i].flower_holes = flam3_random01();
      }

      if (cp->xform[i].var[VAR_CONIC] > 0) {
         /* Create random params for conic */
         cp->xform[i].conic_eccen = flam3_random01();
         cp->xform[i].conic_holes = flam3_random01();
      }

      if (cp->xform[i].var[VAR_PARABOLA] > 0) {
         /* Create random params for parabola */
         cp->xform[i].parabola_height = 0.5 + flam3_random01();
         cp->xform[i].parabola_width = 0.5 + flam3_random01();
      }
#if 0
      if (cp->xform[i].var[VAR_SPLIT] > 0) {
         /* Create random params for split */
         cp->xform[i].split_xsize = 2 * flam3_random11();
         cp->xform[i].split_ysize = 2 + flam3_random11();
         cp->xform[i].split_shift = flam3_random01();
      }

      if (cp->xform[i].var[VAR_MOVE] > 0) {
         /* Create random params for move */
         cp->xform[i].move_x = flam3_random11();
         cp->xform[i].move_y = flam3_random11();
      }
#endif
   }

   /* Randomly add symmetry (but not if we've already added a final xform) */
   if (sym || (!(random()%4) && !addfinal))
      flam3_add_symmetry(cp, sym);
   else
      cp->symmetry = 0;

   qsort((char *) cp->xform, (cp->num_xforms-addfinal), sizeof(flam3_xform), compare_xforms);


}


static int sort_by_x(const void *av, const void *bv) {
    double *a = (double *) av;
    double *b = (double *) bv;
    if (a[0] < b[0]) return -1;
    if (a[0] > b[0]) return 1;
    return 0;
}

static int sort_by_y(const void *av, const void *bv) {
    double *a = (double *) av;
    double *b = (double *) bv;
    if (a[1] < b[1]) return -1;
    if (a[1] > b[1]) return 1;
    return 0;
}


/* Memory helper functions because

    Python on Windows uses the MSVCR71.dll version of the C Runtime and
    mingw uses the MSVCRT.dll version. */

void *flam3_malloc(size_t size) {

   return (malloc(size));

}

void flam3_free(void *ptr) {

   free(ptr);

}

/*
 * find a 2d bounding box that does not enclose eps of the fractal density
 * in each compass direction.
 */
void flam3_estimate_bounding_box(flam3_genome *cp, double eps, int nsamples,
             double *bmin, double *bmax, randctx *rc) {
   int i;
   int low_target, high_target;
   double min[2], max[2];
   double *points;
   unsigned short xform_distrib[CHOOSE_XFORM_GRAIN];

   if (nsamples <= 0) nsamples = 10000;
   low_target = (int)(nsamples * eps);
   high_target = nsamples - low_target;

   points = (double *) malloc(sizeof(double) * 4 * nsamples);
   points[0] = flam3_random_isaac_11(rc);
   points[1] = flam3_random_isaac_11(rc);
   points[2] = 0.0;
   points[3] = 0.0;

   prepare_xform_fn_ptrs(cp,rc);
   flam3_create_xform_distrib(cp,xform_distrib);
   flam3_iterate(cp, nsamples, 20, points, xform_distrib, rc);

   min[0] = min[1] =  1e10;
   max[0] = max[1] = -1e10;

   for (i = 0; i < nsamples; i++) {
      double *p = &points[4*i];
      if (p[0] < min[0]) min[0] = p[0];
      if (p[1] < min[1]) min[1] = p[1];
      if (p[0] > max[0]) max[0] = p[0];
      if (p[1] > max[1]) max[1] = p[1];
   }

   if (low_target == 0) {
      bmin[0] = min[0];
      bmin[1] = min[1];
      bmax[0] = max[0];
      bmax[1] = max[1];
      free(points);
      return;
   }

   qsort(points, nsamples, sizeof(double) * 4, sort_by_x);
   bmin[0] = points[4 * low_target];
   bmax[0] = points[4 * high_target];

   qsort(points, nsamples, sizeof(double) * 4, sort_by_y);
   bmin[1] = points[4 * low_target + 1];
   bmax[1] = points[4 * high_target + 1];
   free(points);
}





typedef double bucket_double[4];
typedef double abucket_double[4];
typedef unsigned int bucket_int[4];
typedef unsigned int abucket_int[4];
typedef unsigned short bucket_short[4];
typedef unsigned short abucket_short[4];
typedef float bucket_float[4];
typedef float abucket_float[4];

#ifdef HAVE_GCC_64BIT_ATOMIC_OPS
inline void
double_atomic_add(double *dest, double delta)
{
	uint64_t *int_ptr = (uint64_t *)dest;
	union {
		double dblval;
		uint64_t intval;
	} old_val, new_val;
	int success;

	do {
		old_val.dblval = *dest;
		new_val.dblval = old_val.dblval + delta;
		success = __sync_bool_compare_and_swap(
			int_ptr, old_val.intval, new_val.intval);
	} while (!success);
}
#endif /* HAVE_GCC_64BIT_ATOMIC_OPS */

#ifdef HAVE_GCC_ATOMIC_OPS
inline void
float_atomic_add(float *dest, float delta)
{
	uint32_t *int_ptr = (uint32_t *)dest;
	union {
		float fltval;
		uint32_t intval;
	} old_val, new_val;
	int success;

	do {
		old_val.fltval = *dest;
		new_val.fltval = old_val.fltval + delta;
		success = __sync_bool_compare_and_swap(
			int_ptr, old_val.intval, new_val.intval);
	} while (!success);
}

inline void
uint_atomic_add(unsigned int *dest, unsigned int delta)
{
	unsigned int old_val, new_val;
	int success;

	do {
		old_val = *dest;
		if (UINT_MAX - old_val > delta)
			new_val = old_val + delta;
		else
			new_val = UINT_MAX;
		success = __sync_bool_compare_and_swap(
			dest, old_val, new_val);
	} while (!success);
}

inline void
ushort_atomic_add(unsigned short *dest, unsigned short delta)
{
	unsigned short old_val, new_val;
	int success;

	do {
		old_val = *dest;
		if (USHRT_MAX - old_val > delta)
			new_val = old_val + delta;
		else
			new_val = USHRT_MAX;
		success = __sync_bool_compare_and_swap(
			dest, old_val, new_val);
	} while (!success);
}
#endif /* HAVE_GCC_ATOMIC_OPS */

/* 64-bit datatypes */
#define B_ACCUM_T double
#define A_ACCUM_T double
#define bucket bucket_double
#define abucket abucket_double
#define abump_no_overflow(dest, delta) do {dest += delta;} while (0)
#define add_c_to_accum(acc,i,ii,j,jj,wid,hgt,c) do { \
   if ( (j) + (jj) >=0 && (j) + (jj) < (hgt) && (i) + (ii) >=0 && (i) + (ii) < (wid)) { \
   abucket *a = (acc) + ( (i) + (ii) ) + ( (j) + (jj) ) * (wid); \
   abump_no_overflow(a[0][0],(c)[0]); \
   abump_no_overflow(a[0][1],(c)[1]); \
   abump_no_overflow(a[0][2],(c)[2]); \
   abump_no_overflow(a[0][3],(c)[3]); \
   } \
} while (0)
/* single-threaded */
#define USE_LOCKS
#define bump_no_overflow(dest, delta)  do {dest += delta;} while (0)
#define render_rectangle render_rectangle_double
#define iter_thread iter_thread_double
#include "rect.c"
#ifdef HAVE_GCC_64BIT_ATOMIC_OPS
/* multi-threaded */
#undef USE_LOCKS
#undef bump_no_overflow
#undef render_rectangle
#undef iter_thread
#define bump_no_overflow(dest, delta)  double_atomic_add(&dest, delta)
#define render_rectangle render_rectangle_double_mt
#define iter_thread iter_thread_double_mt
#include "rect.c"
#else /* !HAVE_GCC_64BIT_ATOMIC_OPS */
#define render_rectangle_double_mt render_rectangle_double
#endif /* HAVE_GCC_64BIT_ATOMIC_OPS */
#undef render_rectangle
#undef iter_thread
#undef add_c_to_accum
#undef A_ACCUM_T
#undef B_ACCUM_T
#undef bucket
#undef abucket
#undef bump_no_overflow
#undef abump_no_overflow

/* 32-bit datatypes */
#define B_ACCUM_T unsigned int
#define A_ACCUM_T unsigned int
#define bucket bucket_int
#define abucket abucket_int
#define abump_no_overflow(dest, delta) do { \
   if (UINT_MAX - dest > delta) dest += delta; else dest = UINT_MAX; \
} while (0)
#define add_c_to_accum(acc,i,ii,j,jj,wid,hgt,c) do { \
   if ( (j) + (jj) >=0 && (j) + (jj) < (hgt) && (i) + (ii) >=0 && (i) + (ii) < (wid)) { \
   abucket *a = (acc) + ( (i) + (ii) ) + ( (j) + (jj) ) * (wid); \
   abump_no_overflow(a[0][0],(c)[0]); \
   abump_no_overflow(a[0][1],(c)[1]); \
   abump_no_overflow(a[0][2],(c)[2]); \
   abump_no_overflow(a[0][3],(c)[3]); \
   } \
} while (0)
/* single-threaded */
#define USE_LOCKS
#define bump_no_overflow(dest, delta) do { \
   if (UINT_MAX - dest > delta) dest += delta; else dest = UINT_MAX; \
} while (0)
#define render_rectangle render_rectangle_int
#define iter_thread iter_thread_int
#include "rect.c"
#ifdef HAVE_GCC_ATOMIC_OPS
/* multi-threaded */
#undef USE_LOCKS
#undef bump_no_overflow
#undef render_rectangle
#undef iter_thread
#define bump_no_overflow(dest, delta)  uint_atomic_add(&dest, delta)
#define render_rectangle render_rectangle_int_mt
#define iter_thread iter_thread_int_mt
#include "rect.c"
#else /* !HAVE_GCC_ATOMIC_OPS */
#define render_rectangle_int_mt render_rectangle_int
#endif /* HAVE_GCC_ATOMIC_OPS */
#undef iter_thread
#undef render_rectangle
#undef add_c_to_accum
#undef A_ACCUM_T
#undef B_ACCUM_T
#undef bucket
#undef abucket
#undef bump_no_overflow
#undef abump_no_overflow

/* experimental 32-bit datatypes (called 33) */
#define B_ACCUM_T unsigned int
#define A_ACCUM_T float
#define bucket bucket_int
#define abucket abucket_float
#define abump_no_overflow(dest, delta) do {dest += delta;} while (0)
#define add_c_to_accum(acc,i,ii,j,jj,wid,hgt,c) do { \
   if ( (j) + (jj) >=0 && (j) + (jj) < (hgt) && (i) + (ii) >=0 && (i) + (ii) < (wid)) { \
   abucket *a = (acc) + ( (i) + (ii) ) + ( (j) + (jj) ) * (wid); \
   abump_no_overflow(a[0][0],(c)[0]); \
   abump_no_overflow(a[0][1],(c)[1]); \
   abump_no_overflow(a[0][2],(c)[2]); \
   abump_no_overflow(a[0][3],(c)[3]); \
   } \
} while (0)
/* single-threaded */
#define USE_LOCKS
#define bump_no_overflow(dest, delta) do { \
   if (UINT_MAX - dest > delta) dest += delta; else dest = UINT_MAX; \
} while (0)
#define render_rectangle render_rectangle_float
#define iter_thread iter_thread_float
#include "rect.c"
#ifdef HAVE_GCC_ATOMIC_OPS
/* multi-threaded */
#undef USE_LOCKS
#undef bump_no_overflow
#undef render_rectangle
#undef iter_thread
#define bump_no_overflow(dest, delta)  uint_atomic_add(&dest, delta)
#define render_rectangle render_rectangle_float_mt
#define iter_thread iter_thread_float_mt
#include "rect.c"
#else /* !HAVE_GCC_ATOMIC_OPS */
#define render_rectangle_float_mt render_rectangle_float
#endif /* HAVE_GCC_ATOMIC_OPS */
#undef iter_thread
#undef render_rectangle
#undef add_c_to_accum
#undef A_ACCUM_T
#undef B_ACCUM_T
#undef bucket
#undef abucket
#undef bump_no_overflow
#undef abump_no_overflow


/* 16-bit datatypes */
#define B_ACCUM_T unsigned short
#define A_ACCUM_T unsigned short
#define bucket bucket_short
#define abucket abucket_short
#define MAXBUCKET (1<<14)
#define abump_no_overflow(dest, delta) do { \
   if (USHRT_MAX - dest > delta) dest += delta; else dest = USHRT_MAX; \
} while (0)
#define add_c_to_accum(acc,i,ii,j,jj,wid,hgt,c) do { \
   if ( (j) + (jj) >=0 && (j) + (jj) < (hgt) && (i) + (ii) >=0 && (i) + (ii) < (wid)) { \
   abucket *a = (acc) + ( (i) + (ii) ) + ( (j) + (jj) ) * (wid); \
   abump_no_overflow(a[0][0],(c)[0]); \
   abump_no_overflow(a[0][1],(c)[1]); \
   abump_no_overflow(a[0][2],(c)[2]); \
   abump_no_overflow(a[0][3],(c)[3]); \
   } \
} while (0)
/* single-threaded */
#define USE_LOCKS
#define bump_no_overflow(dest, delta) do { \
   if (USHRT_MAX - dest > delta) dest += delta; else dest = USHRT_MAX; \
} while (0)
#define render_rectangle render_rectangle_short
#define iter_thread iter_thread_short
#include "rect.c"
#ifdef HAVE_GCC_ATOMIC_OPS
/* multi-threaded */
#undef USE_LOCKS
#undef bump_no_overflow
#undef render_rectangle
#undef iter_thread
#define bump_no_overflow(dest, delta)  ushort_atomic_add(&dest, delta)
#define render_rectangle render_rectangle_short_mt
#define iter_thread iter_thread_short_mt
#include "rect.c"
#else /* !HAVE_GCC_ATOMIC_OPS */
#define render_rectangle_short_mt render_rectangle_short
#endif /* HAVE_GCC_ATOMIC_OPS */
#undef iter_thread
#undef render_rectangle
#undef add_c_to_accum
#undef A_ACCUM_T
#undef B_ACCUM_T
#undef bucket
#undef abucket
#undef bump_no_overflow
#undef abump_no_overflow

double flam3_render_memory_required(flam3_frame *spec)
{
  flam3_genome *cps = spec->genomes;
  int real_bits = spec->bits;

  if (33 == real_bits) real_bits = 32;

  /* note 4 channels * 2 buffers cancels out 8 bits per byte */
  /* does not yet include memory for density estimation filter */

  return
    (double) cps[0].spatial_oversample * cps[0].spatial_oversample *
    (double) cps[0].width * cps[0].height * real_bits;
}

void bits_error(flam3_frame *spec) {
      fprintf(stderr, "flam3: bits must be 16, 32, 33, or 64 not %d.\n",
         spec->bits);
      exit(1);
}

void flam3_render(flam3_frame *spec, void *out,
		  int out_width, int field, int nchan, int trans,
		  stat_struct *stats) {
  if (spec->nthreads == 1) {
    /* single-threaded */
    switch (spec->bits) {
    case 16:
      render_rectangle_short(spec, out, out_width, field, nchan, trans, stats);
      break;
    case 32:
      render_rectangle_int(spec, out, out_width, field, nchan, trans, stats);
      break;
    case 33:
      render_rectangle_float(spec, out, out_width, field, nchan, trans, stats);
      break;
    case 64:
      render_rectangle_double(spec, out, out_width, field, nchan, trans, stats);
      break;
    default:
      bits_error(spec);
      break;
    }
  } else {
    /* multi-threaded */
    switch (spec->bits) {
    case 16:
      render_rectangle_short_mt(spec, out, out_width, field, nchan, trans, stats);
      break;
    case 32:
      render_rectangle_int_mt(spec, out, out_width, field, nchan, trans, stats);
      break;
    case 33:
      render_rectangle_float_mt(spec, out, out_width, field, nchan, trans, stats);
      break;
    case 64:
      render_rectangle_double_mt(spec, out, out_width, field, nchan, trans, stats);
      break;
    default:
      bits_error(spec);
      break;
    }
  }
}

/*
 *   filter function definitions
 * from Graphics Gems III code
 * and ImageMagick resize.c
 */

double flam3_hermite_filter(double t) {
   /* f(t) = 2|t|^3 - 3|t|^2 + 1, -1 <= t <= 1 */
   if(t < 0.0) t = -t;
   if(t < 1.0) return((2.0 * t - 3.0) * t * t + 1.0);
   return(0.0);
}

double flam3_box_filter(double t) {
   if((t > -0.5) && (t <= 0.5)) return(1.0);
   return(0.0);
}

double flam3_triangle_filter(double t) {
   if(t < 0.0) t = -t;
   if(t < 1.0) return(1.0 - t);
   return(0.0);
}

double flam3_bell_filter(double t) {
   /* box (*) box (*) box */
   if(t < 0) t = -t;
   if(t < .5) return(.75 - (t * t));
   if(t < 1.5) {
      t = (t - 1.5);
      return(.5 * (t * t));
   }
   return(0.0);
}

double flam3_b_spline_filter(double t) {

   /* box (*) box (*) box (*) box */
   double tt;

   if(t < 0) t = -t;
   if(t < 1) {
      tt = t * t;
      return((.5 * tt * t) - tt + (2.0 / 3.0));
   } else if(t < 2) {
      t = 2 - t;
      return((1.0 / 6.0) * (t * t * t));
   }
   return(0.0);
}

double flam3_sinc(double x) {
   x *= M_PI;
   if(x != 0) return(sin(x) / x);
   return(1.0);
}

double flam3_blackman_filter(double x) {
  return(0.42+0.5*cos(M_PI*x)+0.08*cos(2*M_PI*x));
}

double flam3_catrom_filter(double x) {
  if (x < -2.0)
    return(0.0);
  if (x < -1.0)
    return(0.5*(4.0+x*(8.0+x*(5.0+x))));
  if (x < 0.0)
    return(0.5*(2.0+x*x*(-5.0-3.0*x)));
  if (x < 1.0)
    return(0.5*(2.0+x*x*(-5.0+3.0*x)));
  if (x < 2.0)
    return(0.5*(4.0+x*(-8.0+x*(5.0-x))));
  return(0.0);
}

double flam3_mitchell_filter(double t) {
   double tt;

   tt = t * t;
   if(t < 0) t = -t;
   if(t < 1.0) {
      t = (((12.0 - 9.0 * flam3_mitchell_b - 6.0 * flam3_mitchell_c) * (t * tt))
         + ((-18.0 + 12.0 * flam3_mitchell_b + 6.0 * flam3_mitchell_c) * tt)
         + (6.0 - 2 * flam3_mitchell_b));
      return(t / 6.0);
   } else if(t < 2.0) {
      t = (((-1.0 * flam3_mitchell_b - 6.0 * flam3_mitchell_c) * (t * tt))
         + ((6.0 * flam3_mitchell_b + 30.0 * flam3_mitchell_c) * tt)
         + ((-12.0 * flam3_mitchell_b - 48.0 * flam3_mitchell_c) * t)
         + (8.0 * flam3_mitchell_b + 24 * flam3_mitchell_c));
      return(t / 6.0);
   }
   return(0.0);
}

double flam3_hanning_filter(double x) {
  return(0.5+0.5*cos(M_PI*x));
}

double flam3_hamming_filter(double x) {
  return(0.54+0.46*cos(M_PI*x));
}

double flam3_lanczos3_filter(double t) {
   if(t < 0) t = -t;
   if(t < 3.0) return(flam3_sinc(t) * flam3_sinc(t/3.0));
   return(0.0);
}

double flam3_lanczos2_filter(double t) {
   if(t < 0) t = -t;
   if(t < 2.0) return(flam3_sinc(t) * flam3_sinc(t/2.0));
   return(0.0);
}

double flam3_gaussian_filter(double x) {
  return(exp((-2.0*x*x))*sqrt(2.0/M_PI));
}

double flam3_quadratic_filter(double x) {
  if (x < -1.5)
    return(0.0);
  if (x < -0.5)
    return(0.5*(x+1.5)*(x+1.5));
  if (x < 0.5)
    return(0.75-x*x);
  if (x < 1.5)
    return(0.5*(x-1.5)*(x-1.5));
  return(0.0);
}

void b64decode(char* instr, char *outstr)
{
    char *cur, *start;
    int d, dlast, phase;
    char c;
    static int table[256] = {
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 00-0F */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 10-1F */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,  /* 20-2F */
        52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,  /* 30-3F */
        -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,  /* 40-4F */
        15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,  /* 50-5F */
        -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,  /* 60-6F */
        41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,  /* 70-7F */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 80-8F */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 90-9F */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* A0-AF */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* B0-BF */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* C0-CF */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* D0-DF */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* E0-EF */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1   /* F0-FF */
    };

    d = dlast = phase = 0;
    start = instr;
    for (cur = instr; *cur != '\0'; ++cur )
    {
   // jer: this is my bit that treats line endings as physical breaks
   if(*cur == '\n' || *cur == '\r'){phase = dlast = 0; continue;}
        d = table[(int)*cur];
        if(d != -1)
        {
            switch(phase)
            {
            case 0:
                ++phase;
                break;
            case 1:
                c = ((dlast << 2) | ((d & 0x30) >> 4));
                *outstr++ = c;
                ++phase;
                break;
            case 2:
                c = (((dlast & 0xf) << 4) | ((d & 0x3c) >> 2));
                *outstr++ = c;
                ++phase;
                break;
            case 3:
                c = (((dlast & 0x03 ) << 6) | d);
                *outstr++ = c;
                phase = 0;
                break;
            }
            dlast = d;
        }
    }
}

/* Helper functions for After Effects Plugin */
/* Function to calculate the size of a 'flattened' genome (required by AE API) */
size_t flam3_size_flattened_genome(flam3_genome *cp) {

   size_t flatsize;

   flatsize = sizeof(flam3_genome);
   flatsize += cp->num_xforms * sizeof(flam3_xform);

   return(flatsize);
}

/* Function to flatten the contents of a genome into a buffer */
void flam3_flatten_genome(flam3_genome *cp, void *buf) {

   int i;
   char *bufoff;

   /* Copy genome first */
   memcpy(buf, (const void *)cp, sizeof(flam3_genome));

   /* Copy the xforms */
   bufoff = (char *)buf + sizeof(flam3_genome);
   for (i=0; i<cp->num_xforms; i++) {
      memcpy(bufoff, (const void *)(&cp->xform[i]), sizeof(flam3_xform));
      bufoff += sizeof(flam3_xform);
   }
}

/* Function to unflatten a genome buffer */
void flam3_unflatten_genome(void *buf, flam3_genome *cp) {

   int i;
   char *bufoff;

   /* Copy out the genome */
   memcpy((void *)cp, (const void *)buf, sizeof(flam3_genome));

   /* Allocate space for the xforms */
   cp->xform = (flam3_xform *)malloc(cp->num_xforms * sizeof(flam3_xform));

   /* Initialize the xforms (good habit to be in) */
   initialize_xforms(cp, 0);

   /* Copy out the xforms from the buffer */
   bufoff = (char *)buf + sizeof(flam3_genome);
   for (i=0; i<cp->num_xforms; i++) {
      memcpy(bufoff, (const void *)(&cp->xform[i]), sizeof(flam3_xform));
      bufoff += sizeof(flam3_xform);
   }
}

void flam3_srandom() {
    unsigned int seed;
    char *s = getenv("seed");

    if (s)
	seed = atoi(s);
    else
	seed = time(0) + getpid();

    srandom(seed);
}
