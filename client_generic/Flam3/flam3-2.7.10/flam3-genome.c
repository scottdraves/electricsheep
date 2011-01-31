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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

static char *flam3_genome_c_id =
"@(#) $Id: flam3-genome.c,v 1.106 2008/02/19 17:17:37 reckase Exp $";

#include "private.h"
#include "isaacs.h"
#include "config.h"

int verbose;

char notes[10000];

void note(char *s) {
  strcat(notes, s);
  strcat(notes, " ");
}

int note_int(int i) {
  char b[20];
  sprintf(b, "%d", i);
  note(b);
  return i;
}

char *get_extras() {
  char *e = getenv("extras");
  char *extras;
  if (strlen(notes) == 0) return e;
  if (NULL == e) e = "";
  extras = malloc(strlen(notes) + strlen(e) + 100);
  sprintf(extras, "%s notes=\"%s\"", e, notes);

  return extras; /* leaks */
}

void gprint(flam3_genome *cp, int extras) {
    if (getenv("noedits"))
        flam3_print(stdout, cp, extras ? get_extras() : NULL, flam3_dont_print_edits);
    else
        flam3_print(stdout, cp, extras ? get_extras() : NULL, flam3_print_edits);
}


void test_cp(flam3_genome *cp) {
   cp->time = 0.0;
   cp->interpolation = flam3_interpolation_linear;
   cp->palette_interpolation = flam3_palette_interpolation_hsv;
   cp->background[0] = 0.0;
   cp->background[1] = 0.0;
   cp->background[2] = 0.0;
   cp->center[0] = 0.0;
   cp->center[1] = 0.0;
   cp->rotate = 0.0;
   cp->pixels_per_unit = 64;
   cp->width = 128;
   cp->height = 128;
   cp->spatial_oversample = 1;
   cp->spatial_filter_radius = 0.5;
   cp->spatial_filter_func = gaussian_filter;
   cp->spatial_filter_support = gaussian_support;
   cp->zoom = 0.0;
   cp->sample_density = 1;
   cp->nbatches = 1;
   cp->ntemporal_samples = 1;
   cp->estimator = 0.0;
   cp->estimator_minimum = 0.0;
   cp->estimator_curve = 0.6;
}

flam3_genome *string_to_cp(char *s, int *n) {
  flam3_genome *cp;
  FILE *fp;

  fp = fopen(s, "rb");
  if (NULL == fp) {
    perror(s);
    exit(1);
  }
  cp = flam3_parse_from_file(fp, s, flam3_defaults_on, n);
  if (NULL == cp) {
      fprintf(stderr, "could not read genome from %s.\n", s);
      exit(1);
  }
  return cp;
}

double smoother(double t) {
  return 3*t*t - 2*t*t*t;
}

xmlDocPtr create_new_editdoc(char *action, flam3_genome *parent0, flam3_genome *parent1) {

   xmlDocPtr doc = NULL, comment_doc = NULL;
   xmlNodePtr root_node = NULL, node = NULL, nodecopy = NULL;
   xmlNodePtr root_comment = NULL;
   struct tm *localt;
   time_t mytime;
   char *ai;
   char timestring[100];
   char *nick = getenv("nick");
   char *url = getenv("url");
   char *id = getenv("id");
   char *comment = getenv("comment");
   int sheep_gen = argi("sheep_gen",-1);
   int sheep_id = argi("sheep_id",-1);
   char buffer[100];
   char comment_string[100];

   doc = xmlNewDoc( (const xmlChar *)"1.0");

   /* Create the root node, called "edit" */
   root_node = xmlNewNode(NULL, (const xmlChar *)"edit");
   xmlDocSetRootElement(doc,root_node);
   /* Add the edit attributes */

   /* date */
   mytime = time(NULL);
   localt = localtime(&mytime);
   /* XXX use standard time format including timezone */
   strftime(timestring, 100, "%a %b %e %H:%M:%S %Z %Y", localt);
   xmlNewProp(root_node, (const xmlChar *)"date", (const xmlChar *)timestring);

   /* nick */
   if (nick) {
      xmlNewProp(root_node, (const xmlChar *)"nick", (const xmlChar *)nick);
   }

   /* url */
   if (url) {
      xmlNewProp(root_node, (const xmlChar *)"url", (const xmlChar *)url);
   }

   if (id) {
      xmlNewProp(root_node, (const xmlChar *)"id", (const xmlChar *)id);
   }

   /* action */
   xmlNewProp(root_node, (const xmlChar *)"action", (const xmlChar *)action);

   /* sheep info */
   if (sheep_gen > 0 && sheep_id > 0) {
      /* Create a child node of the root node called sheep */
      node = xmlNewChild(root_node, NULL, (const xmlChar *)"sheep", NULL);

      /* Create the sheep attributes */
      sprintf(buffer, "%d", sheep_gen);
      xmlNewProp(node, (const xmlChar *)"generation", (const xmlChar *)buffer);

      sprintf(buffer, "%d", sheep_id);
      xmlNewProp(node, (const xmlChar *)"id", (const xmlChar *)buffer);
   }

   /* Check for the parents */
   /* If Parent 0 not specified, this is a randomly generated genome. */
   if (parent0) {
      if (parent0->edits) {
         /* Copy the node from the parent */
         node = xmlDocGetRootElement(parent0->edits);
         nodecopy = xmlCopyNode(node, 1);
         xmlNewProp(nodecopy,(const xmlChar *)"filename", (const xmlChar *)parent0->parent_fname);
         sprintf(buffer,"%d",parent0->genome_index);
         xmlNewProp(nodecopy,(const xmlChar *)"index", (const xmlChar *)buffer);
         xmlAddChild(root_node, nodecopy);
      } else {
         /* Insert a (parent has no edit) message */
         nodecopy = xmlNewChild(root_node, NULL, (const xmlChar *)"edit",NULL);
         xmlNewProp(nodecopy,(const xmlChar *)"filename", (const xmlChar *)parent0->parent_fname);
         sprintf(buffer,"%d",parent0->genome_index);
         xmlNewProp(nodecopy,(const xmlChar *)"index", (const xmlChar *)buffer);

      }
   }

   if (parent1) {

      if (parent1->edits) {
         /* Copy the node from the parent */
         node = xmlDocGetRootElement(parent1->edits);
         nodecopy = xmlCopyNode(node, 1);
         xmlNewProp(nodecopy,(const xmlChar *)"filename", (const xmlChar *)parent1->parent_fname);
         sprintf(buffer,"%d",parent1->genome_index);
         xmlNewProp(nodecopy,(const xmlChar *)"index", (const xmlChar *)buffer);
         xmlAddChild(root_node, nodecopy);
      } else {
         /* Insert a (parent has no edit) message */
         nodecopy = xmlNewChild(root_node, NULL, (const xmlChar *)"edit",NULL);
         xmlNewProp(nodecopy,(const xmlChar *)"filename", (const xmlChar *)parent1->parent_fname);
         sprintf(buffer,"%d",parent1->genome_index);
         xmlNewProp(nodecopy,(const xmlChar *)"index", (const xmlChar *)buffer);
      }
   }

   /* Comment string */
   /* This one's hard, since we have to treat the comment string as   */
   /* a valid XML document.  Create a new document using the comment  */
   /* string as the in-memory document, and then copy all children of */
   /* the root node into the edit structure                           */
   /* Parsing the comment string should be done once and then copied  */
   /* for each call to create_new_editdoc, but that's for later.      */
   if (comment) {

      sprintf(comment_string,"<comm>%s</comm>",comment);

      comment_doc = xmlReadMemory(comment_string, strlen(comment_string), "comment.env", NULL, XML_PARSE_NONET);

      /* Check for errors */
      if (comment_doc==NULL) {
         fprintf(stderr, "Failed to parse comment into XML!\n");
         exit(1);
      }

      /* Loop through the children of the new document and copy */
      /* them into the root_node */
      root_comment = xmlDocGetRootElement(comment_doc);

      for (node=root_comment->children; node; node = node->next) {

         nodecopy = xmlCopyNode(node,1);
         xmlAddChild(root_node, nodecopy);
      }

      /* Free the created document */
      xmlFreeDoc(comment_doc);
   }


   /* return the xml doc */
   return(doc);
}

void offset(flam3_genome *g) {
    char *os = getenv("offset");
    double ox, oy;
    if (NULL == os) return;
    sscanf(os, "%lf:%lf", &ox, &oy);
    g->center[0] += ox / (g->pixels_per_unit * g->spatial_oversample);
    g->center[1] += oy / (g->pixels_per_unit * g->spatial_oversample);
}

void
spin(int frame, double blend, flam3_genome *parent, flam3_genome *templ)
{
  flam3_genome result;
  char action[50];
  xmlDocPtr doc;

  memset(&result, 0, sizeof(flam3_genome));
  flam3_copy(&result,parent);
  
  //  symm = (int *)calloc(result.num_xforms,sizeof(int));
  //  for (si=0;si<result.num_xforms;si++)
  //      symm[si] = 0.0;

  flam3_rotate(&result, blend*360.0);
  
  // free(symm);
  
  if (templ)
     flam3_apply_template(&result, templ);

  result.time = (double)frame;
  result.interpolation = flam3_interpolation_linear;
  result.palette_interpolation = flam3_palette_interpolation_hsv;

  sprintf(action,"rotate %g",blend*360.0);
  doc = create_new_editdoc(action, parent, (flam3_genome *)NULL);
  result.edits = doc;

  offset(&result);

  /* Make the name of the flame the time */
  sprintf(result.flame_name,"%f",result.time);

  gprint(&result, 1);

  xmlFreeDoc(result.edits);

  /* Free the result xform storage */
  free(result.xform);
}

void spin_inter(int frame, double blend, flam3_genome *parents, flam3_genome *templ) {
  flam3_genome spun[2];
  flam3_genome spun_prealign[2];
  flam3_genome result;
  char action[50];
  xmlDocPtr doc;

  memset(spun, 0, 2*sizeof(flam3_genome));
  memset(spun_prealign, 0, 2*sizeof(flam3_genome));
  memset(&result, 0, sizeof(flam3_genome));

  flam3_copy(&(spun_prealign[0]), &(parents[0]));
  flam3_copy(&(spun_prealign[1]), &(parents[1]));

  flam3_align(spun, spun_prealign, 2);

  if (0.0 == blend)
     /* Make sure we use the un-padded original for blend=0 */
     flam3_copy(&result, &(spun_prealign[0]) );
  else {
     spun[0].time = 0.0;
     spun[1].time = 1.0;

     /* Call this first to establish the asymmetric reference angles */
     establish_asymmetric_refangles(spun,2);  

     flam3_rotate(&spun[0], blend*360.0);
     flam3_rotate(&spun[1], blend*360.0);

     /* Now call the interpolation */
     flam3_interpolate(spun, 2, smoother(blend), &result);
  }
  
  if ((parents[0].palette_index != flam3_palette_random) &&
      (parents[1].palette_index != flam3_palette_random)) {
    result.palette_index = flam3_palette_interpolated;
    result.palette_index0 = parents[0].palette_index;
    result.hue_rotation0 = parents[0].hue_rotation;
    result.palette_index1 = parents[1].palette_index;
    result.hue_rotation1 = parents[1].hue_rotation;
    result.palette_blend = blend;
  }

  if (templ)
     flam3_apply_template(&result, templ);

  result.time = (double)frame;

  sprintf(action,"interpolate %g",blend*360.0);
  doc = create_new_editdoc(action, &parents[0], &parents[1]);

  result.edits = doc;

  offset(&result);
  
  /* Make the name of the flame the time */
  sprintf(result.flame_name,"%f",result.time);
  
  gprint(&result, 1);

  xmlFreeDoc(result.edits);

  /* Free xform storage */
  free(spun[0].xform);
  free(spun[1].xform);
  free(spun_prealign[0].xform);
  free(spun_prealign[1].xform);
  free(result.xform);
}

void add_to_action(char *action, char *addtoaction) {

   int alen = strlen(action);
   int addlen = strlen(addtoaction);

   if (alen+addlen < flam3_max_action_length)
      strcat(action,addtoaction);
   else
      fprintf(stderr,"action string too long, truncating...\n");
}

void truncate_variations(flam3_genome *g, int max_vars, char *action) {
   int i, j, nvars, smallest;
   double sv=0;
   char trunc_note[30];

   for (i = 0; i < g->num_xforms; i++) {
      double d = g->xform[i].density;

/*      if (0.0 < d && d < 0.001) */

      if (d < 0.001 && (g->final_xform_index != i)) {
         sprintf(trunc_note," trunc_density %d",i);
         //strcat(action,trunc_note);
         add_to_action(action,trunc_note);
         flam3_delete_xform(g, i);

/*         g->xform[i].density = 0.0;
      } else if (d > 0.0) {
*/
      } else {
         do {
            nvars = 0;
            smallest = -1;
            for (j = 0; j < flam3_nvariations; j++) {
               double v = g->xform[i].var[j];
               if (v != 0.0) {
                  nvars++;
                  if (-1 == smallest || fabs(v) < sv) {
                     smallest = j;
                     sv = fabs(v);
                  }
               }
            }
            if (nvars > max_vars) {
               sprintf(trunc_note," trunc %d %d",i,smallest);
               //strcat(action,trunc_note);
               add_to_action(action,trunc_note);
               g->xform[i].var[smallest] = 0.0;
            }
         } while (nvars > max_vars);
      }
   }
}

double try_colors(flam3_genome *g, int color_resolution) {
    int *hist;
    int i, hits, res = color_resolution;
    int res3 = res * res * res;
    flam3_frame f;
    unsigned char *image, *p;
    flam3_genome saved;
    stat_struct stats;

    memset(&saved, 0, sizeof(flam3_genome));

    flam3_copy(&saved, g);

    g->sample_density = 1;
    g->spatial_oversample = 1;
    g->estimator = 0.0;
    g->width = 100; // XXX keep aspect ratio
    g->height = 100;
    g->pixels_per_unit = 50;
    g->nbatches = 1;
    g->ntemporal_samples = 1;

    f.temporal_filter_radius = 0.0;
    f.bits = 32;
    f.verbose = 0;
    f.genomes = g;
    f.ngenomes = 1;
    f.pixel_aspect_ratio = 1.0;
    f.progress = 0;
    f.nthreads = 1;
    irandinit(&f.rc,1);
        
    image = (unsigned char *) calloc(g->width * g->height, 3);
    flam3_render(&f, image, g->width, flam3_field_both, 3, 0, &stats);

    hist = calloc(sizeof(int), res3);
    p = image;
    for (i = 0; i < g->height * g->width; i++) {
       hist[(p[0] * res / 256) +
            (p[1] * res / 256) * res +
            (p[2] * res / 256) * res * res]++;
       p += 3;
    }

    if (0) {
       int j, k;
       for (i = 0; i < res; i++) {
          fprintf(stderr, "\ni=%d: \n", i);
          for (j = 0; j < res; j++) {
             for (k = 0; k < res; k++) {
                fprintf(stderr, " %5d", hist[i * res * res + j * res + k]);
             }
             fprintf(stderr, "\n");
          }
       }
    }

    hits = 0;
    for (i = 0; i < res3; i++) {
       if (hist[i]) hits++;
    }

    free(hist);
    free(image);

    g->sample_density = saved.sample_density;
    g->width = saved.width;
    g->height = saved.height;
    g->spatial_oversample = saved.spatial_oversample;
    g->pixels_per_unit = saved.pixels_per_unit;
    g->nbatches = saved.nbatches;
    g->ntemporal_samples = saved.ntemporal_samples;
    g->estimator = saved.estimator;

    /* Free xform storage */
    free(saved.xform);

    return (double) hits / res3;
}

int random_xform(flam3_genome *g, int excluded) {
   int ntries = 0;
   while (ntries++ < 100) {
      int i = random() % g->num_xforms;
      if (g->xform[i].density > 0.0 && i != excluded)
         return i;
   }
   return -1;
}

void change_colors(flam3_genome *g, int change_palette) {
   int i;
   int x0, x1;
   if (change_palette) {
      g->hue_rotation = 0.0;
      g->palette_index = flam3_get_palette(flam3_palette_random, g->palette, 0.0);
   }
   for (i = 0; i < g->num_xforms; i++) {
      g->xform[i].color[0] = flam3_random01();
   }
   x0 = random_xform(g, -1);
   x1 = random_xform(g, x0);
   if (x0 >= 0 && (random()&1)) g->xform[x0].color[0] = 0.0;
   if (x1 >= 0 && (random()&1)) g->xform[x1].color[0] = 1.0;
}

void improve_colors(flam3_genome *g, int ntries, int change_palette, int color_resolution) {
   int i;
   double best, b;
   flam3_genome best_genome;

   memset(&best_genome, 0, sizeof(flam3_genome));

   best = try_colors(g, color_resolution);
   flam3_copy(&best_genome,g);
   for (i = 0; i < ntries; i++) {
      change_colors(g, change_palette);
      b = try_colors(g, color_resolution);
      if (b > best) {
         best = b;
         flam3_copy(&best_genome,g);
      }
   }
   flam3_copy(g,&best_genome);

   free(best_genome.xform);
}

static void rotate_by(double *p, double *center, double by) {
    double r[2];
    double th = by * 2 * M_PI / 360.0;
    double c = cos(th);
    double s = -sin(th);
    p[0] -= center[0];
    p[1] -= center[1];
    r[0] = c * p[0] - s * p[1];
    r[1] = s * p[0] + c * p[1];
    p[0] = r[0] + center[0];
    p[1] = r[1] + center[1];
}

int
main(argc, argv)
   int argc;
   char **argv;
{
   int debug = 0;
   int count;
   char *ai;
   unsigned char *image;
   flam3_genome *templ = NULL;
   flam3_genome cp_orig, cp_save;
   int i, j;
   double avg_pix, fraction_black, fraction_white;
   double avg_thresh = argf("avg", 20.0);
   double black_thresh = argf("black", 0.01);
   double white_limit = argf("white", 0.05);
   int nframes = argi("nframes", 100);
   int sym = argi("symmetry", 0);
   int enclosed = argi("enclosed", 1);
   char *clone = getenv("clone");
   char *mutate = getenv("mutate");
   char *cross0 = getenv("cross0");
   char *cross1 = getenv("cross1");
   char *method = getenv("method");
   char *inter = getenv("inter");
   char *rotate = getenv("rotate");
   char *strip = getenv("strip");
   char *sequence = getenv("sequence");
   int loops = argi("loops", 1);
   int frame = argi("frame", 0);
   int rep, repeat = argi("repeat", 1);
   double speed = argf("speed", 0.1);
   int bits = argi("bits", 33);
   int ntries = argi("tries", 10);
   int seed = argi("seed", 0);
   char *use_vars = getenv("use_vars");
   char *dont_use_vars = getenv("dont_use_vars");
   flam3_genome *parent0=NULL, *parent1=NULL;
   flam3_genome selp0, selp1;
   flam3_genome *aselp0, *aselp1;
   int parent0_n, parent1_n;
   int num_threads = 1;
   int ncp;

   int ivars[max_specified_vars];
   int novars[max_specified_vars];
   int num_ivars = 0;
   int num_novars = 0;
   char *var_tok;

   flam3_frame f;
   char action[flam3_max_action_length];  
   
   stat_struct stats;



#ifdef WIN32
   
   char *slashloc;
   char exepath[256];
   char palpath[256];
   slashloc = strrchr(argv[0],'\\');
	if (NULL==slashloc) {
	   sprintf(palpath,"flam3_palettes=flam3-palettes.xml");
	} else {
       strncpy(exepath,argv[0],slashloc-argv[0]+1);
	   sprintf(palpath,"flam3_palettes=%sflam3-palettes.xml",exepath);
	}
	putenv(palpath);

#endif         


   verbose = argi("verbose", 0);

   memset(&cp_orig, 0, sizeof(flam3_genome));
   memset(&cp_save, 0, sizeof(flam3_genome));
   memset(&selp0, 0, sizeof(flam3_genome));
   memset(&selp1, 0, sizeof(flam3_genome));

   if (1 != argc) {
       docstring();
       exit(0);
   }

   /* Init random number generators */
   flam3_init_frame(&f);
   flam3_srandom();

   f.temporal_filter_radius = 0.0;
   f.bits = bits;
   f.verbose = 0;
   f.genomes = &cp_orig;
   f.ngenomes = 1;
   f.pixel_aspect_ratio = 1.0;
   f.progress = 0;
   f.nthreads = num_threads;
   test_cp(&cp_orig);  // just for the width & height
   image = (unsigned char *) malloc(3 * cp_orig.width * cp_orig.height);

   /* Are the variations to be used specified? */
   if (use_vars && dont_use_vars) {
      fprintf(stderr,"use_vars and dont_use_vars cannot both be specified.  Terminating.\n");
      exit(-1);
   }
   
   /* Specify reasonable defaults if nothing is specified */
   if (!use_vars && !dont_use_vars) {
      novars[num_novars++] = VAR_NOISE;
      novars[num_novars++] = VAR_BLUR;
      novars[num_novars++] = VAR_GAUSSIAN_BLUR;
      novars[num_novars++] = VAR_RADIAL_BLUR;
      novars[num_novars++] = VAR_NGON;
      novars[num_novars++] = VAR_SQUARE;
      novars[num_novars++] = VAR_RAYS;
      novars[num_novars++] = VAR_CROSS;

      /* Loop over the novars and set ivars to the complement */
      for (i=0;i<flam3_nvariations;i++) {
         for (j=0;j<num_novars;j++) {
            if (novars[j] == i)
               break;
         }
         if (j==num_novars)
            ivars[num_ivars++] = i;
      }

   } else {
   
        if (use_vars) {
           /* Parse comma-separated list of variations to use */
           var_tok = strtok(use_vars,",");
           ivars[num_ivars++] = atoi(var_tok);
           while(1) {
              var_tok = strtok(NULL,",");

              if (var_tok==NULL)
                 break;

              ivars[num_ivars++] = atoi(var_tok);

              if (num_ivars==max_specified_vars) {
                 fprintf(stderr,"Maximum number of user-specified variations exceeded.  Truncating.\n");
                 break;
              }
           }

           /* Error checking */
           for (i=0;i<num_ivars;i++) {
              if (ivars[i]<0 || ivars[i]>=flam3_nvariations) {
                 fprintf(stderr,"specified variation list includes bad value. (%d)\n",ivars[i]);
                 exit(1);
              }
           }
        } else if (dont_use_vars) {
           /* Parse comma-separated list of variations NOT to use */
           var_tok = strtok(dont_use_vars,",");
           novars[num_novars++] = atoi(var_tok);
           while(1) {
              var_tok = strtok(NULL,",");

              if (var_tok==NULL)
                 break;

              novars[num_novars++] = atoi(var_tok);

              if (num_novars==max_specified_vars) {
                 fprintf(stderr,"Maximum number of user-specified variations exceeded.  Truncating.\n");
                 break;
              }
           }
           
           /* Loop over the novars and set ivars to the complement */
           for (i=0;i<flam3_nvariations;i++) {
              for (j=0;j<num_novars;j++) {
                 if (novars[j] == i)
                    break;
              }
              if (j==num_novars)
                 ivars[num_ivars++] = i;
           }
        }
   }
   
   if (1 < (!!mutate + !!(cross0 || cross1) +
       !!inter + !!rotate + !!clone + !!strip)) {
      fprintf(stderr,
      "can only specify one of mutate, clone, cross, "
         "rotate, strip, or inter.\n");
      exit(1);
   }
   if ( (!cross0) ^ (!cross1) ) {
      fprintf(stderr, "must specify both crossover arguments.\n");
      exit(1);
   }

   if (method && (!cross0)) {
      fprintf(stderr, "cannot specify method unless doing crossover.\n");
      exit(1);
   }

   if (getenv("template")) {
      char *tf = getenv("template");
      FILE *template = fopen(tf, "rb");
      if (0 == template) {
         perror(tf);
         exit(1);
      }
      templ = flam3_parse_from_file(template, tf, flam3_defaults_off, &ncp);
      if (1 < ncp) {
         fprintf(stderr, "more than one control point in template, "
            "ignoring all but first.\n");
      } else if (0 == ncp) {
         fprintf(stderr, "no control points in template.\n");
      }
   }


   if (clone) parent0 = string_to_cp(clone, &parent0_n);
   if (mutate) parent0 = string_to_cp(mutate, &parent0_n);
   if (cross0) parent0 = string_to_cp(cross0, &parent0_n);
   if (cross1) parent1 = string_to_cp(cross1, &parent1_n);
   if (strip) parent0 = string_to_cp(strip, &parent0_n);

   if (sequence) {
      flam3_genome *cp;
      double blend, spread;
      int framecount;
      FILE *fp;

      if (nframes <= 0) {
         fprintf(stderr, "nframes must be positive, not %d.\n", nframes);
         exit(1);
      }

      fp = fopen(sequence, "rb");
      if (NULL == fp) {
         perror(sequence);
         exit(1);
      }
      cp = flam3_parse_from_file(fp, sequence, flam3_defaults_on, &ncp);
      if (NULL == cp) exit(1);
      if (enclosed) printf("<sequence version=\"FLAM3-" VERSION "\">\n");
      spread = 1.0/nframes;
      framecount = 0;
#if 1
      for (i = 0; i < ncp; i++) {
         if (loops) {
            for (frame = 0; frame < nframes; frame++) {
               blend = frame/(double)nframes;
               spin(framecount++, blend, &cp[i], templ);
            }
         }
         if (i < ncp-1)
        for (frame = 0; frame < nframes; frame++) {
       blend = frame/(double)nframes;
       spin_inter(framecount++, blend, &cp[i], templ);
        }
      }
      spin(framecount, 0.0, &cp[ncp-1], templ);
#else
      if (1) {
     flam3_genome res;
     memset(&res, 0, sizeof(flam3_genome));
     res.final_xform_index = -1;
     flam3_add_xforms(&res, cp[0].num_xforms);

     if (ncp < 4) {
         fprintf(stderr, "catmull-rom requires 4 or more control points.\n");
         exit(1);
     }
     for (i = 0; i < ncp - 3; i++) {
         for (frame = 0; frame < nframes; frame++) {
        blend = frame/(double)nframes;
        interpolate_catmull_rom(cp+i, blend, &res);
        res.time = frame + i * nframes;
        gprint(&res, 0);
        fflush(stdout);
         }
     }
      }
#endif
      if (enclosed) printf("</sequence>\n");
      exit(0);
   }

   if (inter || rotate) {
      flam3_genome *cp;
      double blend, spread;
      char *fname = inter?inter:rotate;
      FILE *fp;

      if (nframes <= 0) {
         fprintf(stderr, "nframes must be positive, not %d.\n", nframes);
         exit(1);
      }

      blend = frame/(double)nframes;
      spread = 1.0/nframes;

      fp = fopen(fname, "rb");
      if (NULL == fp) {
         perror(fname);
         exit(1);
      }
      cp = flam3_parse_from_file(fp, fname, flam3_defaults_on, &ncp);
      if (NULL == cp) exit(1);
      if (enclosed) printf("<pick version=\"FLAM3-" VERSION "\">\n");
      if (rotate) {
         if (1 != ncp) {
            fprintf(stderr, "rotation requires one control point, not %d.\n", ncp);
            exit(1);
         }
         spin(frame - 1, blend - spread, cp, templ);
         spin(frame    , blend         , cp, templ);
         spin(frame + 1, blend + spread, cp, templ);
      } else {
         if (2 != ncp) {
            fprintf(stderr, "interpolation requires two control points, not %d.\n", ncp);
            exit(1);
         }
         spin_inter(frame - 1, blend - spread, cp, templ);
         spin_inter(frame    , blend         , cp, templ);
         spin_inter(frame + 1, blend + spread, cp, templ);
      }
      if (enclosed) printf("</pick>\n");
      exit(0);
   }

   if (strip) {
      flam3_genome *cp;
      FILE *fp;

      if (nframes <= 0) {
         fprintf(stderr, "nframes must be positive, not %d.\n", nframes);
         exit(1);
      }

      fp = fopen(strip, "rb");
      if (NULL == fp) {
         perror(strip);
         exit(1);
      }
      cp = flam3_parse_from_file(fp, strip, flam3_defaults_on, &ncp);
      if (NULL == cp) exit(1);
      if (enclosed) printf("<pick version=\"FLAM3-" VERSION "\">\n");
      for (i = 0; i < ncp; i++) {
   double old_center[2];
   old_center[0] = cp[i].center[0];
   old_center[1] = cp[i].center[1];
   cp[i].height /= nframes;
   cp[i].center[1] = cp[i].center[1] - ((nframes - 1) * cp[i].height) /
     (2 * cp[i].pixels_per_unit * pow(2.0, cp[i].zoom));
   cp[i].center[1] += cp[i].height * frame / cp[i].pixels_per_unit;
   rotate_by(cp[i].center, old_center, cp[i].rotate);

   if (templ) flam3_apply_template(&cp[i], templ);
   offset(&cp[i]);
   gprint(&cp[i], 1);
      }
      if (enclosed) printf("</pick>\n");
      exit(0);
   }

   /* pick a control point until it looks good enough */
   if (repeat <= 0) {
     fprintf(stderr, "repeat must be positive, not %d.\n", repeat);
     exit(1);
   }

   if (enclosed) printf("<pick version=\"FLAM3-" VERSION "\">\n");

   for (rep = 0; rep < repeat; rep++) {
     notes[0] = 0;

       if (verbose) {
      fprintf(stderr, "flame = %d/%d..", rep+1, repeat);
       }

      count = 0;

      if (clone) {

         /* Action is 'clone' with trunc_vars concat */
         sprintf(action,"clone");
	 if (getenv("clone_action"))
	   sprintf(action,"clone %s", getenv("clone_action"));

         flam3_copy(&selp0, &(parent0[random()%parent0_n]));
         flam3_copy(&cp_save, &selp0);
         aselp0 = &selp0;
         aselp1 = NULL;
         truncate_variations(&cp_save, 5, action);

         cp_save.edits = create_new_editdoc(action, aselp0, aselp1);

      } else {
         int did_color;

         do {
	   notes[0] = 0;
            if (verbose) fprintf(stderr, ".");
            did_color = 0;
            f.time = (double) 0.0;

            if (mutate) {

               flam3_genome mutation;
               double r = flam3_random01();

               memset(&mutation, 0, sizeof(flam3_genome));

               flam3_copy(&selp0, &(parent0[random()%parent0_n]));
               flam3_copy(&cp_orig, &selp0);
               aselp0 = &selp0;
               aselp1 = NULL;

               if (r < 0.1) {
                  int done = 0;
                  if (debug) fprintf(stderr, "mutating variation\n");
                  sprintf(action,"mutate variation");
                  // randomize the variations, usually a large visual effect
                  do {
                     /* Create a random flame, and use the variations */
                     /* to replace those in the original              */
                     flam3_random(&mutation, ivars, num_ivars, sym, cp_orig.num_xforms);
                     for (i = 0; i < cp_orig.num_xforms; i++) {

                        /* Replace if density > 0 or this is the final xform */
                        /*if (cp_orig.xform[i].density > 0.0 || (cp_orig.final_xform_enable==1 && cp_orig.final_xform_index==i)) {*/
                           for (j = 0; j < flam3_nvariations; j++) {
                              if (cp_orig.xform[i].var[j] != mutation.xform[i].var[j]) {
                                 cp_orig.xform[i].var[j] = mutation.xform[i].var[j];

                                 /* Copy parameters for this variation only */
                                 flam3_copy_params(&(cp_orig.xform[i]),&(mutation.xform[i]),j);

                                 done = 1;
                              }
                           }
                        /*}*/
                     }
                  } while (!done);
               } else if (r < 0.3) {
                  // change one xform coefs but not the vars
                  int xf, nxf = 0;
                  if (debug) fprintf(stderr, "mutating one xform\n");
                  sprintf(action,"mutate xform");
                  flam3_random(&mutation, ivars, num_ivars, sym, 2);
                  /*for (i = 0; i < cp_orig.num_xforms; i++) {
                     if (cp_orig.xform[i].density > 0.0) {
                        nxf++;
                     }
                  }*/

                  nxf = cp_orig.num_xforms;

                  if (0 == nxf) {
                     fprintf(stderr, "no xforms in control point.\n");
                     exit(1);
                  }
                  xf = random()%nxf;

                  // if only two xforms, then change only the translation part
                  if (2 == nxf) {
                     for (j = 0; j < 2; j++)
                        cp_orig.xform[xf].c[2][j] = mutation.xform[0].c[2][j];
                  } else {
                     for (i = 0; i < 3; i++)
                        for (j = 0; j < 2; j++)
                        cp_orig.xform[xf].c[i][j] = mutation.xform[0].c[i][j];
                  }
               } else if (r < 0.5) {
                  if (debug) fprintf(stderr, "adding symmetry\n");
                  sprintf(action,"mutate symmetry");
                  flam3_add_symmetry(&cp_orig, 0);
               } else if (r < 0.6) {
                  int b = 1 + random()%6;
                  int same = random()&3;
                  if (debug) fprintf(stderr, "setting post xform\n");
                  sprintf(action,"mutate post");
                  for (i = 0; i < cp_orig.num_xforms; i++) {
                     int copy = (i > 0) && same;
/*                     if (cp_orig.xform[i].density == 0.0) continue;*/
                     if (copy) {
                        for (j = 0; j < 3; j++) {
                           cp_orig.xform[i].post[j][0] = cp_orig.xform[0].post[j][0];
                           cp_orig.xform[i].post[j][1] = cp_orig.xform[0].post[j][1];
                        }
                     } else {
                        if (b&1) {
                           double f = M_PI * flam3_random11();
                           double t[2][2];
                           t[0][0] = (cp_orig.xform[i].c[0][0] * cos(f) +
                              cp_orig.xform[i].c[0][1] * -sin(f));
                           t[0][1] = (cp_orig.xform[i].c[0][0] * sin(f) +
                              cp_orig.xform[i].c[0][1] * cos(f));
                           t[1][0] = (cp_orig.xform[i].c[1][0] * cos(f) +
                              cp_orig.xform[i].c[1][1] * -sin(f));
                           t[1][1] = (cp_orig.xform[i].c[1][0] * sin(f) +
                              cp_orig.xform[i].c[1][1] * cos(f));

                           cp_orig.xform[i].c[0][0] = t[0][0];
                           cp_orig.xform[i].c[0][1] = t[0][1];
                           cp_orig.xform[i].c[1][0] = t[1][0];
                           cp_orig.xform[i].c[1][1] = t[1][1];

                           f *= -1.0;

                           t[0][0] = (cp_orig.xform[i].post[0][0] * cos(f) +
                              cp_orig.xform[i].post[0][1] * -sin(f));
                           t[0][1] = (cp_orig.xform[i].post[0][0] * sin(f) +
                              cp_orig.xform[i].post[0][1] * cos(f));
                           t[1][0] = (cp_orig.xform[i].post[1][0] * cos(f) +
                              cp_orig.xform[i].post[1][1] * -sin(f));
                           t[1][1] = (cp_orig.xform[i].post[1][0] * sin(f) +
                              cp_orig.xform[i].post[1][1] * cos(f));

                           cp_orig.xform[i].post[0][0] = t[0][0];
                           cp_orig.xform[i].post[0][1] = t[0][1];
                           cp_orig.xform[i].post[1][0] = t[1][0];
                           cp_orig.xform[i].post[1][1] = t[1][1];

                        }
                        if (b&2) {
                           double f = 0.2 + flam3_random01();
                           double g;
                           if (random()&1) f = 1.0 / f;
                           if (random()&1) {
                              g = 0.2 + flam3_random01();
                              if (random()&1) g = 1.0 / g;
                           } else
                           g = f;
                           cp_orig.xform[i].c[0][0] /= f;
                           cp_orig.xform[i].c[0][1] /= f;
                           cp_orig.xform[i].c[1][1] /= g;
                           cp_orig.xform[i].c[1][0] /= g;
                           cp_orig.xform[i].post[0][0] *= f;
                           cp_orig.xform[i].post[1][0] *= f;
                           cp_orig.xform[i].post[0][1] *= g;
                           cp_orig.xform[i].post[1][1] *= g;
                        }
                        if (b&4) {
                           double f = flam3_random11();
                           double g = flam3_random11();
                           cp_orig.xform[i].c[2][0] -= f;
                           cp_orig.xform[i].c[2][1] -= g;
                           cp_orig.xform[i].post[2][0] += f;
                           cp_orig.xform[i].post[2][1] += g;
                        }
                     }
                  }
               } else if (r < 0.7) {
                  double s = flam3_random01();
                  did_color = 1;
                  // change just the color
                  if (debug) fprintf(stderr, "mutating color\n");
                  if (s < 0.4) {
                     improve_colors(&cp_orig, 100, 0, 10);
                     sprintf(action,"mutate color coords");
                  } else if (s < 0.8) {
                     improve_colors(&cp_orig, 25, 1, 10);
                     sprintf(action,"mutate color all");
                  } else {
                     cp_orig.palette_index = flam3_get_palette(flam3_palette_random, cp_orig.palette, cp_orig.hue_rotation);
                     sprintf(action,"mutate color palette");
                  }
               } else if (r < 0.8) {
                  int nx = 0;
                  if (debug) fprintf(stderr, "deleting an xform\n");
                  sprintf(action,"mutate delete");
/*                  for (i = 0; i < cp_orig.num_xforms; i++) {
                     if (cp_orig.xform[i].density > 0.0)
                        nx++;
                  }*/

                  nx = cp_orig.num_xforms;

                  if (nx > 1) {

                     nx = random()%nx;
                     flam3_delete_xform(&cp_orig,nx);

/*                  if (nx > 1) {
                     nx = random()%nx;
                     for (i = 0; i < cp_orig.num_xforms; i++) {
                        if (nx == ny) {
                           cp_orig.xform[i].density = 0;
                           break;
                        }
                        if (cp_orig.xform[i].density > 0.0)
                           ny++;
                     }*/
                  } else {
                     if (verbose)
                        fprintf(stderr, "not enough xforms to delete one.\n");
                  }
               } else {
                  int x;
                  if (debug) fprintf(stderr, "mutating all coefs\n");
                  sprintf(action,"mutate all");
                  flam3_random(&mutation, ivars, num_ivars, sym, cp_orig.num_xforms);

                  // change all the coefs by a little bit
                  for (x = 0; x < cp_orig.num_xforms; x++) {
/*                     if (cp_orig.xform[x].density > 0.0) {*/
                        for (i = 0; i < 3; i++) {
                           for (j = 0; j < 2; j++) {
                              cp_orig.xform[x].c[i][j] += speed * mutation.xform[x].c[i][j];
                           /* Eventually, we can mutate the parametric variation coefs here. */
                           }
                        }
/*                     }*/
                  }
               }

               if (random()&1) {
                  double bmin[2], bmax[2];
        flam3_estimate_bounding_box(&cp_orig, 0.01, 100000, bmin, bmax, &f.rc);
                  cp_orig.center[0] = (bmin[0] + bmax[0]) / 2.0;
                  cp_orig.center[1] = (bmin[1] + bmax[1]) / 2.0;
                  cp_orig.rot_center[0] = cp_orig.center[0];
                  cp_orig.rot_center[1] = cp_orig.center[1];
                  cp_orig.pixels_per_unit = cp_orig.width / (bmax[0] - bmin[0]);
                  //strcat(action," reframed");
                  add_to_action(action," reframed");
                  
               }

            } else if (cross0) {
               int i0, i1, rb, used_parent;
               char ministr[10];


               if (NULL == getenv("method")) {
                  double s = flam3_random01();
                  if (s < 0.1)
                     method = "union";
                  else if (s < 0.2)
                     method = "interpolate";
                  else
                     method = "alternate";
               }

               if (strcmp(method, "alternate") &&
                   strcmp(method, "interpolate") &&
                   strcmp(method, "union")) {
                  fprintf(stderr,
                  "method must be either alternate, interpolate, "
                  "or union, not %s.\n", method);
                  exit(1);
               }

               sprintf(action,"cross %s",method);
               i0 = random()%parent0_n;
               i1 = random()%parent1_n;

               flam3_copy(&selp0, &(parent0[i0]));
               flam3_copy(&selp1, &(parent1[i1]));

               aselp0 = &selp0;
               aselp1 = &selp1;

	       note("cross");
	       note_int(i0);
	       note_int(i1);

               if (!strcmp(method, "alternate")) {
         int got0, got1;
         char *trystr;

         trystr = calloc(4 * (parent1[i1].num_xforms + parent0[i0].num_xforms), sizeof(char));

                  /* each xform from a random parent,
                  possible for one to be excluded */
         do {

             trystr[0] = 0;
             got0 = got1 = 0;
             rb = rbit();
             sprintf(ministr," %d:",rb);
             strcat(trystr,ministr);

             /* Copy the parent, sorting the final xform to the end if it's present. */
             if (rb)
            flam3_copyx(&cp_orig, &(parent1[i1]),
                   parent1[i1].num_xforms - (parent1[i1].final_xform_index > 0),
                   parent1[i1].final_xform_enable);
             else
            flam3_copyx(&cp_orig, &(parent0[i0]),
                   parent0[i0].num_xforms - (parent0[i0].final_xform_index > 0),
                   parent0[i0].final_xform_enable);

             used_parent = rb;

             /* Only replace non-final xforms */

             for (i = 0; i < cp_orig.num_xforms - cp_orig.final_xform_enable; i++) {
            rb = rbit();

            /* Replace xform if bit is 1 */
            if (rb==1) {
                if (used_parent==0) {
               if (i < parent1[i1].num_xforms && parent1[i1].xform[i].density > 0) {
                   cp_orig.xform[i] = parent1[i1].xform[i];
                   sprintf(ministr," 1");
                   got1 = 1;
               } else {
                   sprintf(ministr," 0");
                   got0 = 1;
               }
                } else {
               if (i < parent0[i0].num_xforms && parent0[i0].xform[i].density > 0) {
                   cp_orig.xform[i] = parent0[i0].xform[i];
                   sprintf(ministr," 0");
                   got0 = 1;
               } else {
                   sprintf(ministr," 1");
                   got1 = 1;
               }
                }
            } else {
                sprintf(ministr," %d",used_parent);
                if (used_parent)
               got1 = 1;
                else
               got0 = 1;
            }
            strcat(trystr,ministr);
             }
         } while ((i > 1) && !(got0 && got1));

         add_to_action(action,trystr);
         free(trystr);

               } else if (!strcmp(method, "interpolate")) {
                  /* linearly interpolate somewhere between the two */
                  flam3_genome parents[2];
                  double t = flam3_random01();

                  memset(parents, 0, 2*sizeof(flam3_genome));


                  sprintf(ministr," %g",t);
                  //strcat(action,ministr);
                  add_to_action(action,ministr);

                  flam3_copy(&(parents[0]), &(parent0[i0]));
                  flam3_copy(&(parents[1]), &(parent1[i1]));
                  parents[0].time = 0.0;
                  parents[1].time = 1.0;
                  flam3_interpolate(parents, 2, t, &cp_orig);

                  /* except pick a simple palette */
                  rb = rbit();
                  sprintf(ministr," %d",rb);
                  //strcat(action,ministr);
                  add_to_action(action,ministr);
                  cp_orig.palette_index = rb ? parent1[i1].palette_index : parent0[i0].palette_index;

                  free(parents[0].xform);
                  free(parents[1].xform);

               } else {
                  /* union */
                  flam3_copy(&cp_orig, &(parent0[i0]));

                  i = 0;
                  for (j = 0; j < parent1[i1].num_xforms; j++) {
                     /* Skip over the final xform, if it's present.    */
                     /* Default behavior keeps the final from parent0. */
                     if (parent1[i1].final_xform_index == j)
                        continue;
                     flam3_add_xforms(&cp_orig, 1);
                     cp_orig.xform[cp_orig.num_xforms-1] = parent1[i1].xform[j];
                  }
               }

               /* reset color coords */
               if (cp_orig.num_xforms > 0) {
                  for (i = 0; i < cp_orig.num_xforms; i++) {
                     cp_orig.xform[i].color[0] = i&1;
                     cp_orig.xform[i].color[1] = (i&2)>>1;
                  }
               }
               
               /* Potentially genetically cross the two colormaps together */
               if (flam3_random01() < 0.4) {
                              
                  /* Select the starting parent */
                  int startParent=flam3_random_isaac_bit(&f.rc);
                  int ci;
                  
                  if (debug)
                     fprintf(stderr,"crossing maps...\n");
                  
                  //strcat(action," cmap_cross");
                  add_to_action(action," cmap_cross");
                  sprintf(ministr," %d:",startParent);
                  //strcat(action,ministr);
                  add_to_action(action,ministr);
                  
                  /* Loop over the entries, switching to the other parent 1% of the time */
                  for (ci=0;ci<256;ci++) {
                     if (flam3_random_isaac_01(&f.rc)<.01) {
                        startParent = 1-startParent;
                        sprintf(ministr," %d",ci);
                        //strcat(action,ministr);
                        add_to_action(action,ministr);
                     }
                     
                     if (startParent==0) {
                        cp_orig.palette[ci][0] = parent0[i0].palette[ci][0];
                        cp_orig.palette[ci][1] = parent0[i0].palette[ci][1];
                        cp_orig.palette[ci][2] = parent0[i0].palette[ci][2];
                     } else {
                        cp_orig.palette[ci][0] = parent1[i1].palette[ci][0];
                        cp_orig.palette[ci][1] = parent1[i1].palette[ci][1];
                        cp_orig.palette[ci][2] = parent1[i1].palette[ci][2];
                     }
                  }
               }

            } else {
               sprintf(action,"random");
               flam3_random(&cp_orig, ivars, num_ivars, sym, 0);

               /* Adjust bounding box by default */
               if (1) {
                  double bmin[2], bmax[2];
                  flam3_estimate_bounding_box(&cp_orig, 0.01, 100000, bmin, bmax, &f.rc);
                  cp_orig.center[0] = (bmin[0] + bmax[0]) / 2.0;
                  cp_orig.center[1] = (bmin[1] + bmax[1]) / 2.0;
                  cp_orig.rot_center[0] = cp_orig.center[0];
                  cp_orig.rot_center[1] = cp_orig.center[1];
                  cp_orig.pixels_per_unit = cp_orig.width / (bmax[0] - bmin[0]);
                  add_to_action(action," reframed");
               }

               aselp0 = NULL;
               aselp1 = NULL;
            }

            truncate_variations(&cp_orig, 5, action);

	    if (!did_color && random()&1) {
	        if (debug)
	           fprintf(stderr,"improving colors...\n");
	        improve_colors(&cp_orig, 100, 0, 10);
	        //strcat(action," improved colors");
	        add_to_action(action," improved colors");
	    }

            cp_orig.edits = create_new_editdoc(action, aselp0, aselp1);
            flam3_copy(&cp_save, &cp_orig);
            test_cp(&cp_orig);
            flam3_render(&f, image, cp_orig.width, flam3_field_both, 3, 0, &stats);

            if (1) {
               int n, tot, totb, totw;
               n = cp_orig.width * cp_orig.height;
               tot = 0;
               totb = 0;
               totw = 0;
               for (i = 0; i < 3*n; i+=3) {
               
                  tot += (image[i]+image[i+1]+image[i+2]);
                  if (0 == image[i] && 0 == image[i+1] && 0 == image[i+2]) totb++;
                  if (255 == image[i] && 255 == image[i+1] && 255 == image[i+2]) totw++;

                  // printf("%d ", image[i]);
               }

               avg_pix = (tot / (double)(3*n));
               fraction_black = totb / (double)n;
               fraction_white = totw / (double)n;

               if (debug)
                  fprintf(stderr,
                     "avg_pix=%g fraction_black=%g fraction_white=%g n=%g\n",
                     avg_pix, fraction_black, fraction_white, (double)n);

            } else {
               avg_pix = avg_thresh + 1.0;
               fraction_black = black_thresh + 1.0;
               fraction_white = white_limit - 1.0;
            }

            count++;
         } while ((avg_pix < avg_thresh ||
                   fraction_black < black_thresh ||
                   fraction_white > white_limit) &&
                   count < ntries);

         if (ntries == count) {
            fprintf(stderr, "warning: reached maximum attempts, giving up.\n");
         }

      }

      if (templ)
         flam3_apply_template(&cp_save,templ);

      cp_save.time = (double)rep;

      gprint(&cp_save, 1);
      fflush(stdout);

      /* Free created documents */
      /* (Only free once, since the copy is a ptr to the original) */
      xmlFreeDoc(cp_save.edits);

      if (verbose) {
         fprintf(stderr, "\ndone.  action = %s\n", action);
      }

   }
   if (enclosed) printf("</pick>\n");

   return 0;
}
