FLAM3 - cosmic recursive fractal flames
see the file COPYING for the license covering this software.

This is free software to render fractal flames as described on
http://flam3.com.  Flam3-animate makes animations, and flam3-render
makes still images.  Flam3-genome creates and manipulates genomes
(parameter sets).  A C library is also installed.

Note: the following instructions are written for Linux users.  Windows
users may install the cygwin package to get the "env" command or set
the envars in your windows command prompt manually.  That means
instead of a command like

    env dtime=5 prefix=foo. in=test.flame flam3-animate

say

    set dtime=5
    set prefix=foo.
    set in=test.flame
    flam3-animate

As usual, to configure, build, and install:

    ./configure
    make
    sudo make install

This package depends on development packages for libz, libpng,
libjpeg, and libxml2.

To test it, run

    flam3-render < test.flam3

and it should produce 00000.jpg and 00001.jpg, one image for each
<flame> element in the parameter file.  To make an animation run

    flam3-animate < test.flam3

and it should produce 100 files named 00000.jpg through 00099.jpg that
interpolate between the two <flame> elements.


envar           default         meaning
=====           =======         =======
prefix          (empty)         prefix names of output files with this string.
begin           j               time of first frame to render (j=first time in input file) (animate only)
end             n-1             time of last frame to render (n=last time specified in the input file) (animate only)
time            NA              time of first and last frame (ie do one frame) (animate only)
frame           NA              synonym for "time" (animate only)
in              stdin           name of input file
out             NA              name of output file (bad idea if rending more than one, use prefix instead)
template        NA              apply defaults based on this genome (genome only)
dtime           1               time between frames (animate only)
fields          0               if 1 then render fields, ie odd scanlines at time+0.5
nstrips         1               number of strips, ie render fractions of a frame at once (render only)
qs              1               quality scale, multiply quality of all frames by this
ss              1               size scale, multiply size (in pixels) of all frames by this
jpeg            NA              jpeg quality for compression, default is native jpeg default
format          png             "jpg" or "ppm" or "png"
pixel_aspect    1.0             aspect ratio of pixels (width over height), eg 0.90909 for NTSC
isaac_seed      random          string to be used in generating random seed.  defaults to time(0)
seed            random          integer seed for random numbers, defaults to time+pid.  deprecated.
verbose         0               if non-zero then print progress meter on stderr
bits            33              also 16, 32, or 64: sets bit-width of internal buffers (33 means 32-bit floating-point)
bpc             8               bits per channel of color: only png supports 16 (render/animate)
image           filename        replace palette with png, jpg, or ppm image
use_vars        -1              comma sep list of variation #'s to use when generating a random flame (genome only)
dont_use_vars   unset           comma sep list of variation #'s to NOT use when generating a random flame. (genome only)
tries           50              number of tries to make to find a good genome.
method          NA              method for crossover: alternate, interpolate, or union.
symmetry        NA              set symmetry of result.
clone           NA              clone input (this is an alternative to mutate)
strip           NA              strip input, frame and nframes control which one.
transparency    0               make bknd transparent, if format supports it
name_enable     0               use 'name' attr in <flame> to name image output if present (render only)
nick            ""              nickname to use in <edit> tags / img comments
url             ""              url to use in <edit> tags / img comments
id		""		ID  to use in <edit> tags / img comments
comment         ""              comment string for <edit> tags (genome only)
use_mem         auto            floating point number of bytes of memory to use (render only)
nthreads        auto            number of threads to use (render and animate)
noedits         unset           omit edit tags from output (genome only)
print_edit_depth 0              depth to truncate <edit> tag structure.  0 prints all <edit> tags (genome only)
intpalette      unset           round palette entries for importing into older Apophysis versions (genome only)
insert_palette  unset           insert the palette into the image.
enable_jpeg_comments  1         enables comments in the jpeg header (render and animate)
enable_png_comments   1         enables comments in the png header (render and animate)

for example:

    env dtime=5 prefix=foo. in=test.flam3 flam3-animate

means to render every 5th frame of parameter file foo.flam3, and store
the results in files named foo.XXXX.jpg.

the flam3-convert program reads from stdin the old format created by
the GIMP and writes to stdout the new xml format.

the flam3-genome program creates random parameter files. it also mutates,
rotates, and interpolates existing parameter files.  for example to
create 10 wholly new control points and render them at normal quality:

    env template=vidres.flam3 repeat=10 flam3-genome > new.flam3
    flam3-render < new.flam3

if you left out the "template=vidres.flam3" part then the size,
quality, etc parameters would be their default (small) values.  you
can set the symmetry group:

    env template=vidres.flam3 symmetry=3 flam3-genome > new3.flam3
    env template=vidres.flam3 symmetry=-2 flam3-genome > new-2.flam3
    flam3-render < new3.flam3
    flam3-render < new-2.flam3

Mutation is done by giving an input flame file to alter:

    env template=vidres.flam3 flam3-genome > parent.flam3
    env prefix=parent. flam3-render < parent.flam3
    env template=vidres.flam3 mutate=parent.flam3 repeat=10 flam3-genome > mutation.flam3
    flam3-render < mutation.flam3

Normally one wouldn't use the same file for the template and the file
to mutate.  Crossover is handled similarly:

    env template=vidres.flam3 flam3-genome > parent0.flam3
    env prefix=parent0. flam3-render < parent0.flam3
    env template=vidres.flam3 flam3-genome > parent1.flam3
    env prefix=parent1. flam3-render < parent1.flam3
    env template=vidres.flam3 cross0=parent0.flam3 cross1=parent1.flam3 flam3-genome > crossover.flam3
    flam3-render < crossover.flam3

flam3-genome has 3 ways to produce parameter files for animation in
the style of electric sheep.  the highest level and most useful from
the command line is the sequence method.  it takes a collection of
control points and makes an animation that has each flame do fractal
rotation for 360 degrees, then make a smooth transition to the next.
for example:

    env sequence=test.flam3 nframes=20 flam3-genome > seq.flam3
    flam3-animate < seq.flam3

creates and renders a 60 frame animation.  there are two flames in
test.flam3, so the animation consists three stages: the first one
rotating, then a transition, then the second one rotating.  each stage
has 20 frames as specified on the command line.  if you want to
render only some fraction of a whole animation file, specify the begin
and end times:

    env begin=20 end=40 flam3-animate < seq.flam3

the other two methods are harder to use because they produce files that
are only good for one frame of animation.  the output consists of 3
control points, one for the time requested, one before and one after.
that allows proper motion blur.  for example:

    env template=vidres.flam3 flam3-genome > rotme.flam3
    env rotate=rotme.flam3 frame=10 nframes=20 flam3-genome > rot10.flam3
    env frame=10 flam3-animate < rot10.flam3

the file rot10.flam3 specifies the animation for just one frame, in
this case 10 out of 20 frames in the complete animation.  C1
continuous electric sheep genetic crossfades are created like this:

    env inter=test.flam3 frame=10 nframes=20 flam3-genome > inter10.flam3
    env frame=10 flam3-animate < inter10.flam3

A preview of image fractalization is available by setting the image
envar to the name of a png (alpha supported), jpg, or ppm format file.
Note this interface will change!  This image is used as a 2D palette
to paint the flame.  The input image must be 256x256 pixels.  For
example:

    env image=star.png flam3-render < test.flam3

--

The complete list of variations:

   0. linear
   1. sinusoidal
   2. spherical
   3. swirl
   4. horseshoe
   5. polar
   6. handkerchief
   7. heart
   8. disc
   9. spiral
  10. hyperbolic
  11. diamond
  12. ex
  13. julia
  14. bent
  15. waves
  16. fisheye
  17. popcorn
  18. exponential
  19. power
  20. cosine
  21. rings
  22. fan
  23. blob
  24. pdj
  25. fan2
  26. rings2
  27. eyefish
  28. bubble
  29. cylinder
  30. perspective
  31. noise
  32. julian
  33. juliascope
  34. blur
  35. gaussian_blur
  36. radial_blur
  37. pie
  38. ngon
  39. curl
  40. rectangles
  41. arch
  42. tangent
  43. square
  44. rays
  45. blade
  46. secant2
  47. twintrian
  48. cross
  49. disc2
  50. super_shape
  51. flower
  52. conic
  53. parabola
  
see http://flam3.com/flame.pdf for descriptions & formulas for each of
these.  note that, by default, if a random flame is requested and neither
'use_vars' or 'dont_use_vars' are specified, the following variations are
not used: noise, blur, gaussian_blur, radial_blur, ngon, square, rays, 
and cross.

======================================

todo:  eliminate all static storage.

======================================

changelog:

08/25/08 Added new interpolation types 'old' and 'older', for use in 
    recreating old animations.  'linear' mode now does not rotate padded
    xforms (results in prettier symmetric singularities). switched to
    using a 'padding' flag instead of a 'just initialized' flag; padding 
    flag used for implementation of 'old' and 'older' types.  
    interpolation_space now deprecated, instead use interpolation_type. 
    flam3_align is now idempotent (multiple applications do not change
    the control points.)  Default number of temporal samples bumped to 
    1000.  Removed CVS headers from source code (now using SVN).
    Default interpolation mode now log. Removed 'move' and 'split' vars.
    changes to flam3-genome: sequence mode now returns linear 
    interpolation mode for all control points except first/last of edges 
    - these cps will use the original interpolation mode;  inter and 
    rotate modes will now return padded genomes for all control points,
    all with linear interpolation specified.  instead of centering 
    sometimes reframe by golden mean plus noise. Release as 2.7.15.

07/21/08 Add configuration option for atomic-ops.  bug fix: do not
    truncate floating point palettes.  new motion blur features: add
    temporal_filter_type, can be "box" (default) or "gaussian" or
    "exp".  Temporal_filter_width and temporal_filter_exp are parms to
    it.  'blur' env var no longer used.  Small bug fix: iteration
    count depends only on the size of the output image, not the padded
    image (the gutter).  When interpolating, only do -pi/pi adjustment
    for non-asymmetric cases.  Julian/juliascope variations use the
    alternate inverted identity for interpolation (reduces wedge
    effect).  Add python script for regression and consistency
    checking. Add svn revision number to version string (in the
    software not of the package). Release as 2.7.14.

05/28/08 Restored upper limit on particle coordinates.  Release as
    2.7.13.

05/16/08 Added man pages.  Removed upper limit on particle
    coordinates.  fixed FSF address in comments.  update to automake
    1.10.1.  added eps to denom of perspective fraction to avoid
    infinities (thanks fred).  Put contents of 'id' env var in png/jpg
    comment block.  Release as 2.7.12.

04/05/08 Added 16 bit per channel support to PNG via bpc envar.
    isaac.h is now installed with the flam3 headers.  Strip indexing
    now done with size_t's to fix bug in large images (thanks Paul).
    Progress callback now returns ETA, and per-thread verbose flag
    functionality fixed.  Enumerated spatial filter types now used in
    flam3.h, taking the place of function pointers (simplifies the
    API).  Fix bug by moving precalculation of variation variables to
    flam3_iterate (thanks david).  Release as 2.7.11.

03/15/08 fixed interpolation bug when magnitude of rotation/scaling
    component of affine transform is 0.  replaced secant variation
    with more flame-friendly secant2 (eliminates gap in y direction,
    scales y-coordinate by weight).  warning message now printed when 
    unrecognized variation is present in an xform.  fixed bad 
    inequality when checking for -pi/pi discontinuity during complex 
    interpolation.  release as 2.7.10.

02/08/08 non-zero weights for final xforms no longer allowed, and now 
    have no effect.  recompiled windows exes with mingw gcc 4.1 to 
    take advantage of scalability improvements in flam3 2.7.8 (was 
    compiling with mingw gcc 3.4 until now). for fedora package 
    compliance, flam3.pc.in patched by ian weller and moved use of 
    config.h to c files only. release as 2.7.9.

01/26/08 better scalability across multiple CPUs by using compare and
    swap (from Tim Hockin).  fix bug in large images (>2GB).  add
    flam3_srandom and flam3_malloc/free to better support wrapping
    with python on windows, which links with a different C
    runtime. add --enable-shared option to configure script (from
    Bobby R Ward). release as 2.7.8.

12/16/07 fixed bug (rare random crash) identified by david burnett.
    initialize new xforms with better defaults for parametric
    variations (based on interpolated-against xforms).  add id envar,
    like nick/url.  add clone_action envar.  return to xform
    interpolation based on complex logs, but make consistent decisions
    about counter/clockwise rotation across the sheep edges.
    interpolation_space attribute to flame element can be set to
    "linear" to go back to the simpler method, or "log" to use this
    new (default) method. release as 2.7.7

10/20/07 fixed action string overflow when many xforms are present.
    added 'print_edit_depth' env var to control how many levels of
    <edit> tags are saved when using flam3-genome.  Fixed wrong
    placement of random improve_colors code in flam3-genome.  go to
    five digit filenames.  fixed bug in supershape variation.  API
    cleanup, thanks to david bitseff.  release 2.7.6.

08/14/07 various code updates to assist with compiling under msvc++.
    now can specify --disable-pthread or --enable-pthread to configure
    to better control compilation.  fixed bug preventing renders of
    flames with more than 128 xforms.  release 2.7.5.

07/12/07 fixed bug in split variation, now compatible with both
    versions of Apo.  added insert_palette option.  switched density
    estimation kernel from Epanichnikov to Gaussian.  genetic cross
    now crosses palettes rather than selecting one of the parent 
    palettes.  remove noisy variations from random generation if 
    use_vars or dont_use_vars not specified.  fixed metrics calculated
    on small test render for genetic operations.  reduced memory 
    requirements for density estimation filters.  64-bit linux distros
    now supported.  release 2.7.4.


06/21/07 flam3 version, rendered genome, some render statistics and
    optionally nick/url stored in jpeg/png headers.  fixed two bugs
    in isaac rng code (strongly affected temporal blur).  prevent 
    final xform rotation for sheep animation.  fixed interpolation 
    when only one flame has final xform.  added Supershape, Flower, 
    Conic, Parabola, Move and Split variations.  Shape combined with 
    Supershape via 'rnd' parameter.  flam3-genome now writes 'name' 
    attribute for rotate and sequence modes. oversample attribute 
    deprecated; supersample now preferred. new build process for 
    windows exes using MinGW/MSYS.  added 'intpalette' env var to 
    round floating point palettes to allow older versions of Apophysis 
    to read them. default image type is now PNG, transparency off.  
    density estimation code revised to be more consistent between 
    different supersample levels, which required change to default 
    de params.  limit number of de filters to conserve memory.  fixed 
    julia variation dependency on non-thread-safe random bit function.
    removed random number storage for radial blur variation.
    release 2.7.3.
    07/26/08: Note, incompat. change made to direction of cp->rotate
    (as of 2.7.3, rotates shortest distance instead of clockwise) 

02/09/07 use isaac random number generator to avoid differences
	 between mac/pc/linux.  use multiple threads to take advantage
	 of multiple CPUs or cores.  new variations: disc2, Arch,
	 Tangent, Square, Rays, Blade, Secant, Twintrian, Cross.
	 fixed bug reading hex color format - must remove \n prior to
	 start of colors if present.  use EPS in spiral, hyperbolic
	 and spherical to eliminate black spots.  fix numbering of
	 disc2, change parameter to estimate_bounding_box.  made blur
	 kernel type templatable.  removed extraneous 'enabled="1"' in
	 writing out final xform information (no longer needed).
	 release 2.7.2.

12/31/06 fixed bug in waves precalculation code.  curl and rectangles
	 variations from joel faber.  remove visibility of
	 rotation_center attribute, fix strips operator to handle
	 rotation without it.  added motion exponent and gamma lin
	 thresh to flam3_print. list of variations in the docstring
	 automatically generated.  flam3-genome: envar noedits
	 suppresses output of edit history. release 2.7.1.

09/27/06 print out palette as floating point with up to 6 digits
	 rather than integers (for very slow smooth transitions).
	 Added optimization to only recalculate the xform distrib
	 array when necessary (Thanks Joel Faber).  Added settable
	 kernel for downrez.  Added XML reading code for new <palette>
	 format for gradients.  Functionalized hex gradient read.
	 Removed color shift experimental feature.  New
	 'motion_exponent' attribute controls fade of temporal steps.
	 added gaussian_blur and radial_blur vars from apo.  added pie
	 and ngon variations from joel faber.  add rotation_center
	 attribute. added but disabled image variation from joel.
	 release as 2.7.

06/26/06 use new libpng api, handle libpng errors properly (from nix
	 at esperi.org.uk).  print out palette as floating point with
	 6 digits rather than integers (for very slow smooth
	 transitions).

06/11/06 disable density estimation for 16 and 32 bit buffers because
	 it doesn't work.  Apo team removed "enabled" from final xform
	 specification, so now it defaults to on.  make default
	 temporal samples be 60 so that motion blur is the default,
	 but explicitly ignore temporal samples when rendering still
	 frames.  add strip genetic operator to flam3-genome for
	 breaking a flame into strips for parallel rendering.  add
	 subpixel offsets for antialiasing by averaging multiple
	 images.  release as 2.7b8.

05/25/06 change attribute name "batches" to "passes" because many old
         genomes have it set, and with density estimation it is
         harmful.

04/28/06 remove debugging code, release as 2.7b7.

04/28/06 add attribute interpolation="smooth" for catmull-rom
         interpolation.  known bug: some frames come out dark.
         release as 2.7b6.

04/15/06 rename XML attribute "estimator" to "estimator_radius".  fix
         the progress callback.  add use_mem envar.  move palette
         database to external file.  cleanup namespace.  add new
         variations blur, julian, and juliascope.  fix flam3-convert.
         add experimental color shifting via "shift" envar.  supports
         finalxform.  can read hex palette format. avoids the "square"
         that appears from NaNs.  release as 2.7b5.

01/11/06 unlimited number of xforms per flame.  new variations
	 cylinder and perspective.  rename rename de_max_filter to
	 estimator, de_min_filter to estimator_minimum, de_alpha
	 to estimator_curve, and jitters to temporal_samples.
	 release as 2.7b4.

11/28/05 performance optimization, fix symmetry singularities, added
	 bubble variation, fixed handkerchief variation, templates
	 apply to sequence and rotate flam3-genome commands, added
	 envar "name_enable" to render filenames specified in the
	 "name" attribute of the flame element, small bugfixes,
	 release as 2.7b3.

11/04/05 verbose on by default.  Added 'transparency' envar for png
         renders without transparency channel.  Fixed non-black
         background jpeg renders.  add improve_colors mutation,
         rewrite most of estimate_bounding_box.  added new variations
         blob pdjd fan2 rings2 eyefish.  added '33bit' method for
         32-bit floats.  <edit> tags track history.  alter the gamma
         curve to be linear near 0 to avoid singularity and reduce
         noise, set with gamma_threshold. release as v2.7b2.

09/24/05 new density estimator and temporal jitter code from Erik
         Reckase (note removal of todo items above :).  allow gamma,
         vibrancy, contrast, pixels_per_unit, and brightness to vary
         as part of the genome rather than as part of the rendering
         parameters.  flam3-genome does not retry when cloning.  ditch
         2nd color coordinates.  release as v2.7b1.

06/27/05 in flam3_dimension, give up and return NaN if 90% or more of
	 the samples are being clipped.  release v2.6.

06/22/05 add envar to control the number of tries.

06/06/05 add new form of mutation that introduces post xforms.

05/20/05 fix memory trashing bug resulting from xform overflow.  put
	 regular xforms before symmetry in printed genomes.  enforce
	 weights non-negative & at least one xform.  remove
	 nan-protection from popcorn variation.  truncate xforms with
	 very small weight.

05/13/05 fix bug reported by erik reckase where fan variation could
         blow up to NaN because the domain of atan2 was not protected.
         remove protection from all atan2 calls and instead detect NaN
         results and replace them with noise.  count really large
         values as bad too to prevent blowing up to infinity.  enforce
         0<=hue<1, release v2.6b1.

05/05/05 report choices made during genome generation in notes
	 attribute.  flam3_dimension no longer hangs when most of the
	 attractor is outside the camera.  limit number of variations
	 produced by genetic operators to 5.  reduce rate of
	 interpolation method of crossover.

03/17/05 put cloning back in (found by James Rantanen).

03/08/05 change sawtooth variation (incompatible!).  add fan
	 variation.  rename sawtooth to rings.  release as v2.5.

03/01/05 fix rotation when nstrips > 1.  add flam3_dimension().  minor
	 bugfixes.  release as v2.4.

01/25/05 release as v2.3.

01/18/05 support post xforms (idea from eric nguyen).  support camera
	 rotation. 

12/28/04 release as v2.2.

12/27/04 preview implementation of image fractalization by adding a
         color coordinate.  changed how random/default color
         coordinates are selected (they alternate 0 and 1 instead of
         being distributed between 0 and 1).  WARNING: incompatible
         format change to samples argument of flam3_iterate.

12/20/04 allow mutation and crossover from files with multiple
         genomes.  a random genome is selected for each repetition.

12/11/04 fix bug printing out interpolated non-built-in palettes.
	 warn if any images sizes in animation do not match.

12/01/04 remove debugging code left in flam3-convert, thanks to Paul
	 Fishwick for reporting this.  add cosine variation.  add
	 sawtooth variation.  handle nstrips that do not divide the
	 height.  write partial results as strips are computed.  fix
	 old bug where in 32 bit mode one of the terms appeared to be
	 calculated at 16 bits.  release as v2.1.

10/28/04 fix docstring bug. release as v2.0.1.

10/28/04 renaming, cleanup, and modularization.  now exports
         libflam3.a and flam3.h, all names prefixed with flam3_.
         binaries named with flam3- prefix, genome files use flam3
	 suffix.  create and install a flam3.pc pkg-config file.
	 release as v2.0.

09/21/04 fix bug where code for integer rounding was left in 64-bit
	 floating point version.  round remaining time up so we do not
	 say ETA in 0 seconds.  do not use static allocation to hold
	 onto memory, just malloc and free what we need every frame.
	 enforce positive oversampling.  fix bug in hqi on sequences
	 of images of different sizes.  release as v1.15.

09/17/04 change name of envar to control jpeg compression quality from
	 "quality" to "jpeg".  check for bad nbatches values.  release
	 as v1.14.

08/23/04 add about 600 cmaps from Jackie, Pat Phillips, Classylady,
         and BTomchek.  use 64-bit (double) sized buffers.  remove
	 white_level. add new variations: exponential & power.  fix
	 bug where hue_rotation was left uninitialized.  add clone
	 option to pick-flame which just applies template to the input
	 without modifying genome.  random_control_point can now put
	 multiple variations in one xform.  remove altivec code because
	 it is incompatible with 64-bit buffers.  verbose (progress bar
	 on stderr) from Jon Morton.  control random number seeds via
	 seed envar.  support buffer sizes 16, 32, or 64 bits with
	 three versions of rect.c included into libifs.c.

03/28/04 fix bug interpolating between flames with different numbers
	 of xforms introduced by the new de/parsing.  add modified
	 version of the popcorn variation from apophysis.  fix small
	 bug in waves variation.  make distribution of variations
	 even.  add altivec code from R.J. Berry of warwick.ac.uk.
	 release as v1.13.

03/26/04 add wave variation.  allow negative variational coefs.  do
	 not truncate filter width to an integer.  add fisheye
	 variation.  make variations print by name instead of using a
	 vector, that is say spherical="1" instead of var="0 0 1" or
	 var1="2".  it should still read the old format.

03/07/04 fix bug printing out result of interpolating between
	 symmetries. release as v1.11.

03/03/04 add new means of crossover: a random point between the
	 parents as defined by linear interpolation.  in all kinds of
	 crossover, reset the color coordinates to make sure they are 
	 spread out well.  somehow lost part of the extra_attributes
	 patch, so put it in again.  add pixel_aspect_ratio envar.
	 decrease filter cutoff threshold.  the edges of the filter
	 were almost zeros, so making the filter smaller saves time
	 with almost no effect on the results.  do not print out the
	 attributes of control points that have default values.
	 release as v1.10. 

02/26/04 remove prefix argument to print_control_point, and add
	 extra_attributes.  allow any value for vibrancy parameter.
	 allow variation blending coefs to have any values.  do not
	 normalize them.  on windows nstrips is computated
	 automatically to fit within memory (leaves at least 20%
	 unused).  support png image format and if output is png then
	 compute & output alpha channel. release as v1.9.

02/01/04 add julia variation, and put bent variation back in.  change
         how colors are computed in presense of symmetry: xforms that
         come from a symmetry do not change the color index.  this
         prevents the colors from washing out.  since an xform may be
         interpolated between a symmetry and not, this this is a
         blending factor.  add more documentation.  add function to
         compute lyapunov coefs. allow control over symmetries
         produced by pick-flame.  release as v1.8.

07/20/03 cleanup, update documentation, release as v1.7.

07/15/03 fix bug in interpolation where in last frame when going from
	 non-symmetric to symmetric it left out some xforms.  drop
	 support for "cmap_inter".  add var1 as a abbreviation to var
	 in xml output, and do not print trailing 0s in var string.

07/09/03 change matrix interpolation to be linear rather than complex
	 log to avoid the discontinuity at 180 degrees.  this was
	 causing jumpiness in the C1 continuous algorithm.  this means
	 that rotation has to be handled with pick-flame.  put direct
	 support for symmetries in the de/parser to make control
	 points smaller and easier to understand.  support
	 combinations of bilateral & rotational symmetry.

06/22/03 bug in colormap interpolation.  release as v1.6.

06/20/03 fix some bugs, in particular remove sorting of the xforms
	 when control points are read.  they are only sorted when they
	 are generated.  updated dates on copyright notices.  added
	 time parameter to anim, shorthand for begin & end being the
	 same. added a fairly terrible hack to allow palettes to be
	 specified as blending of two basic palettes.  this requires
	 much less bandwidth than sending 256 rgb triples.  in
	 pick-flame change default to be enclosed xml output.

06/09/03 add C1 continuous interpolation to pick-flame (suggested by
	 Cassidy Curtis).  added new variations from Ultra Fractal
	 version by Mark Townsend.  added symmetry xforms.

06/02/03 add convert-flame which reads the old file format and writes 
	 the new one. release as v1.5.

03/22/03 fix bug in hqi & anim.  somewhere along the way (prolly jpeg)
	 nstrips was broken.  add qs and ss params to hqi.
	 discontinue strips for anim because the implementation is a
	 bit problematic (animating zoom??).  bump version to 1.4.

03/05/03 add pick-flame.c to the project, and extend it with mutation
	 and crossover capability.  add parse_control_points_from_file
	 (and use it). rename non-xml variants of de/parsers to *_old
	 and rename xml variants to remove _xml.  add
	 rotate_control_point.  bump version to 1.3.

02/10/03 increase CHOOSE_XFORM_GRAIN by 10x and use chars instead of
         ints.  remove extra -ljpeg from Makefile.  lose hack that
         ignored density xforms after interpolation.  what was that
         for??  it makes a difference, and just makes interpolation
         less smooth as far as i can tell.  bump version to 1.2.

01/17/03 release as v1.1.

01/16/03 support output in jpeg format; this is the default.  support
	 win32.

01/08/03 by default don't render the last frame so that animations
	 dove-tail and loop

01/06/03 fix how too many xforms are detected so that one more xform
	 is allowed.

12/22/02 first release as independent package.  release as v1.0.
