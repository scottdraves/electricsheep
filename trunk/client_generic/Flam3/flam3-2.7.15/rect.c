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

/* this file is included into flam3.c once for each buffer bit-width */

/*
 * for batch
 *   generate de filters
 *   for temporal_sample_batch
 *     interpolate
 *     compute colormap
 *     for subbatch
 *       compute samples
 *       buckets += cmap[samples]
 *   accum += time_filter[temporal_sample_batch] * log[buckets] * de_filter
 * image = filter(accum)
 */


/* allow this many iterations for settling into attractor */
#define FUSE 15

/* clamp spatial filter to zero at this std dev (2.5 ~= 0.0125) */
#define FILTER_CUTOFF 1.8

#define PREFILTER_WHITE 255
#define WHITE_LEVEL 255
#define SUB_BATCH_SIZE 10000

#if defined(HAVE_LIBPTHREAD) && defined(USE_LOCKS)
  /* mutex for bucket accumulator */
  pthread_mutex_t bucket_mutex;
#endif

static void iter_thread(void *fth) {
   double sub_batch;
   int j;
   flam3_thread_helper *fthp = (flam3_thread_helper *)fth;
   flam3_iter_constants *ficp = fthp->fic;
   /* Timer information needs to be persistent - only updated by 1 thread*/
   static time_t progress_timer;
   static time_t progress_timer_history[64];
   static double progress_history[64];
   static int progress_history_mark = 0;
   double eta = 0.0;
   bucket cmap_scaled[CMAP_SIZE];
   
   if (fthp->timer_initialize) {
   	progress_timer = 0;
   	memset(progress_timer_history,0,64*sizeof(time_t));
   	memset(progress_history,0,64*sizeof(double));
   	progress_history_mark = 0;
   }
   
   for (sub_batch = 0; sub_batch < ficp->batch_size; sub_batch+=SUB_BATCH_SIZE) {
      int sub_batch_size, badcount;
      time_t newt = time(NULL);
      /* sub_batch is double so this is sketchy */
      sub_batch_size = (sub_batch + SUB_BATCH_SIZE > ficp->batch_size) ?
	(ficp->batch_size - sub_batch) : SUB_BATCH_SIZE;

      if (fthp->first_thread && newt != progress_timer) {
         double percent = 100.0 *
             ((((sub_batch / (double) ficp->batch_size) + ficp->temporal_sample_num)
             / ficp->ntemporal_samples) + ficp->batch_num)/ficp->nbatches;
         int old_mark = 0;
         int ticker;

	 if (ficp->spec->verbose)
	     fprintf(stderr, "\rchaos: %5.1f%%", percent);
         progress_timer = newt;
         if (progress_timer_history[progress_history_mark] &&
                progress_history[progress_history_mark] < percent)
            old_mark = progress_history_mark;

         if (percent > 0) {
            eta = (100 - percent) * (progress_timer - progress_timer_history[old_mark])
                  / (percent - progress_history[old_mark]);

	    if (ficp->spec->verbose) {
		ticker = (progress_timer&1)?':':'.';
		if (eta < 1000)
		    ticker = ':';
		if (eta > 100)
		    fprintf(stderr, "  ETA%c %.1f minutes", ticker, eta / 60);
		else
		    fprintf(stderr, "  ETA%c %ld seconds ", ticker, (long) ceil(eta));
		fprintf(stderr, "              \r");
		fflush(stderr);
	    }
         }

         progress_timer_history[progress_history_mark] = progress_timer;
         progress_history[progress_history_mark] = percent;
         progress_history_mark = (progress_history_mark + 1) % 64;
      }

      if (ficp->spec->progress) {
	if (fthp->first_thread) {
	  if ((*ficp->spec->progress)(ficp->spec->progress_parameter,
				      sub_batch/(double)ficp->batch_size, 0, eta)) {
	    ficp->aborted = 1;
            #ifdef HAVE_LIBPTHREAD
	      pthread_exit((void *)0);
            #else
              return;
            #endif
	  }
	} else {
          #ifdef HAVE_LIBPTHREAD
	    if (ficp->aborted) pthread_exit((void *)0);
          #else
            if (ficp->aborted) return;
          #endif
	}
      }

      /* Seed iterations */
      fthp->iter_storage[0] = flam3_random_isaac_11(&(fthp->rc));
      fthp->iter_storage[1] = flam3_random_isaac_11(&(fthp->rc));
      fthp->iter_storage[2] = flam3_random_isaac_01(&(fthp->rc));
      fthp->iter_storage[3] = flam3_random_isaac_01(&(fthp->rc));

      /* Execute iterations */
      badcount = flam3_iterate(&(fthp->cp), sub_batch_size, FUSE, fthp->iter_storage, ficp->xform_distrib, &(fthp->rc));

      #if defined(HAVE_LIBPTHREAD) && defined(USE_LOCKS)
        /* Lock mutex for access to accumulator */
        pthread_mutex_lock(&bucket_mutex);
      #endif

      /* Add the badcount to the counter */
      ficp->badvals += badcount;

      /* Premultiply the cmap by the color_scalar, if necessary */
      if (!ficp->fname_specified) {
         bucket_double *dmap = (bucket_double *)(ficp->dmap);
         int c,ch;
         for (c=0;c<CMAP_SIZE;c++) {
            for (ch=0;ch<4;ch++) {
               cmap_scaled[c][ch] = ficp->color_scalar * dmap[c][ch];
            }
         }
      }
                  


      /* Put them in the bucket accumulator */
      for (j = 0; j < sub_batch_size*4; j+=4) {
         double p0, p1, p00, p11;
         int k, color_index0, color_index1;
         double *p = &(fthp->iter_storage[j]);
         bucket *b;

         if (fthp->cp.rotate != 0.0) {
            p00 = p[0] - fthp->cp.rot_center[0];
            p11 = p[1] - fthp->cp.rot_center[1];
            p0 = p00 * ficp->rot[0][0] + p11 * ficp->rot[0][1] + fthp->cp.rot_center[0];
            p1 = p00 * ficp->rot[1][0] + p11 * ficp->rot[1][1] + fthp->cp.rot_center[1];
         } else {
            p0 = p[0];
            p1 = p[1];
         }

         if (p0 >= ficp->bounds[0] && p1 >= ficp->bounds[1] && p0 <= ficp->bounds[2] && p1 <= ficp->bounds[3]) {

            if (ficp->fname_specified) {
               int ci;
               unsigned char *cmap = (unsigned char *)(ficp->cmap);
               bucket *buckets = (bucket *)(ficp->buckets);

               color_index0 = (int) (p[2] * CMAP_SIZE);
               color_index1 = (int) (p[3] * CMAP_SIZE);

               if (color_index0 < 0)
                  color_index0 = 0;
               else if (color_index0 > (CMAP_SIZE-1))
                  color_index0 = CMAP_SIZE-1;

               if (color_index1 < 0)
                  color_index1 = 0;
               else if (color_index1 > (CMAP_SIZE-1))
                  color_index1 = CMAP_SIZE-1;

               b = buckets + (int)(ficp->ws0 * p0 - ficp->wb0s0) +
                   ficp->width * (int)(ficp->hs1 * p1 - ficp->hb1s1);

               ci = 4 * (CMAP_SIZE * color_index1 + color_index0);

               for (k = 0; k < 4; k++)
                  bump_no_overflow(b[0][k], cmap[ci+k]);

            } else {

//               bucket *cmap = (bucket *)(ficp->cmap);
               bucket *buckets = (bucket *)(ficp->buckets);

               color_index0 = (int) (p[2] * CMAP_SIZE);
               if (color_index0 < 0)
                  color_index0 = 0;
               else if (color_index0 > CMAP_SIZE_M1)
                  color_index0 = CMAP_SIZE_M1;

               b = buckets + (int)(ficp->ws0 * p0 - ficp->wb0s0) +
                   ficp->width * (int)(ficp->hs1 * p1 - ficp->hb1s1);

               bump_no_overflow(b[0][0], cmap_scaled[color_index0][0]);
               bump_no_overflow(b[0][1], cmap_scaled[color_index0][1]);
               bump_no_overflow(b[0][2], cmap_scaled[color_index0][2]);
               bump_no_overflow(b[0][3], cmap_scaled[color_index0][3]);

            }
         }
      }
      
      #if defined(HAVE_LIBPTHREAD) && defined(USE_LOCKS)
        /* Release mutex */
        pthread_mutex_unlock(&bucket_mutex);
      #endif

   }
   #ifdef HAVE_LIBPTHREAD
     pthread_exit((void *)0);
   #endif
}

static void render_rectangle(flam3_frame *spec, void *out,
			     int out_width, int field, int nchan,
			     int transp, stat_struct *stats) {
   long nbuckets;
   int i, j, k, batch_num, temporal_sample_num;
   double nsamples, batch_size;
   bucket  *buckets;
   abucket *accumulate;
   double *points;
   double *filter, *temporal_filter, *temporal_deltas, *motion_filter;
   double ppux=0, ppuy=0;
   int image_width, image_height;    /* size of the image to produce */
   int filter_width;
   int bytes_per_channel = spec->bytes_per_channel;
   int oversample = spec->genomes[0].spatial_oversample;
   int nbatches = spec->genomes[0].nbatches;
   /* ntemporal_samples ( & nbatches?) should be computed inside the batch
      loop the same place the DE parms are.  problem is that batches now
      depends on times.  fix by making all batches happen at the same time */
   int ntemporal_samples = spec->genomes[0].ntemporal_samples;
   bucket cmap[CMAP_SIZE];
   bucket_double dmap[CMAP_SIZE];
   unsigned char *cmap2=NULL;
   int gutter_width;
   double vibrancy = 0.0;
   double gamma = 0.0;
   double background[3];
   int vib_gam_n = 0;
   double keep_thresh=100.0;
   time_t progress_timer = 0, progress_began=0;
   int verbose = spec->verbose;
   char *fname = getenv("image");
   int gnm_idx,max_gnm_de_fw,de_offset;
   flam3_genome cp;
   unsigned short xform_distrib[CHOOSE_XFORM_GRAIN];
   char *ai;
   flam3_iter_constants fic;
   flam3_thread_helper *fth;
#ifdef HAVE_LIBPTHREAD
   pthread_attr_t pt_attr;
   pthread_t *myThreads=NULL;
#endif
   int thread_status;
   int thi;
   time_t tstart,tend;
   
   double sumfilt;

   tstart = time(NULL);

   fic.badvals = 0;
   fic.aborted = 0;

   stats->num_iters = 0;

   memset(&cp,0, sizeof(flam3_genome));

   if (nbatches < 1) {
       fprintf(stderr, "nbatches must be positive," " not %d.\n", nbatches);
       exit(1);
   }

   if (oversample < 1) {
       fprintf(stderr, "oversample must be positive," " not %d.\n", oversample);
       exit(1);
   }

   /* Initialize the thread helper structures */
   fth = (flam3_thread_helper *)malloc(spec->nthreads * sizeof(flam3_thread_helper));
   for (i=0;i<spec->nthreads;i++) {
      fth[i].cp.num_xforms=0;
   }

   image_width = spec->genomes[0].width;
   if (field) {
      image_height = spec->genomes[0].height / 2;
      if (field == flam3_field_odd)
    out += nchan * bytes_per_channel * out_width;
      out_width *= 2;
   } else
      image_height = spec->genomes[0].height;


   /* Spatial Filter kernel creation */


   if (1) {
   int sf_sel = spec->genomes[0].spatial_filter_select;
   double sf_supp = flam3_spatial_support[sf_sel];
   double fw =  (2.0 * sf_supp * oversample *
         spec->genomes[0].spatial_filter_radius /
         spec->pixel_aspect_ratio);
   double adjust;

   filter_width = ((int) fw) + 1;
   /* make sure it has same parity as oversample */
   if ((filter_width ^ oversample) & 1)
      filter_width++;

   if (fw > 0.0)
      adjust = sf_supp * filter_width / fw;
   else
      adjust = 1.0;

#if 0
   fprintf(stderr, "fw = %g filter_width = %d adjust=%g\n",
         fw, filter_width, adjust);
#endif

   filter = (double *) malloc(sizeof(double) * filter_width * filter_width);
   /* fill in the coefs */
   for (i = 0; i < filter_width; i++)
      for (j = 0; j < filter_width; j++) {
         double ii = ((2.0 * i + 1.0) / filter_width - 1.0)*adjust;
         double jj = ((2.0 * j + 1.0) / filter_width - 1.0)*adjust;

         if (field) jj *= 2.0;

         jj /= spec->pixel_aspect_ratio;

         filter[i + j * filter_width] = 
            flam3_spatial_filter(sf_sel,ii) * flam3_spatial_filter(sf_sel,jj);
      }


   if (normalize_vector(filter, filter_width * filter_width)) {
      fprintf(stderr, "spatial filter value is too small: %g.\n",
         spec->genomes[0].spatial_filter_radius);
      exit(1);
   }
#if 0
      printf("vvvvvvvvvvvvvvvvvvvvvvvvvvvv\n");
      for (j = 0; j < filter_width; j++) {
    for (i = 0; i < filter_width; i++) {
      printf(" %5d", (int)(10000 * filter[i + j * filter_width]));
    }
    printf("\n");
      }
      printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
      fflush(stdout);
#endif
   }

   temporal_filter = (double *) malloc(sizeof(double) * nbatches * ntemporal_samples);
   temporal_deltas = (double *) malloc(sizeof(double) * nbatches * ntemporal_samples);

   if (nbatches*ntemporal_samples > 1) {

      double nn = nbatches*ntemporal_samples;
      double maxfilt = 0.0;
      sumfilt=0.0;

      /* Define temporal deltas from center time */
      for (i = 0; i < nbatches*ntemporal_samples; i++)
            temporal_deltas[i] = (2.0 * ((double) i / ((nbatches*ntemporal_samples) - 1)) - 1.0)
                                  * (spec->genomes[0].temporal_filter_width/2.0);

      /* fill in the coefs */
      if (flam3_temporal_exp == spec->genomes[0].temporal_filter_type) {
         for (i = 0; i < nbatches*ntemporal_samples; i++) {
            double slpx;

            if (spec->genomes[0].temporal_filter_exp>=0)
               slpx = ((double)i+1.0)/(nbatches*ntemporal_samples);
            else
               slpx = (double)(nbatches*ntemporal_samples - i) / (nbatches*ntemporal_samples);

            /* Scale the color based on these values */
            temporal_filter[i] = pow(slpx,fabs(spec->genomes[0].temporal_filter_exp));
	    if (maxfilt < temporal_filter[i]) maxfilt = temporal_filter[i];
	    
         }

      } else if (flam3_temporal_gaussian == spec->genomes[0].temporal_filter_type) {

         double nn2 = nn/2.0;
         for (i = 0; i < nbatches*ntemporal_samples; i++) {
            temporal_filter[i] = flam3_spatial_filter(flam3_gaussian_kernel,
                            flam3_spatial_support[flam3_gaussian_kernel]*fabs(i - nn2)/nn2);
	    if (maxfilt < temporal_filter[i]) maxfilt = temporal_filter[i];
         }

      } else { // (flam3_temporal_box == spec->genomes[0].temporal_filter_type)
         for (i=0; i<nn; i++)
            temporal_filter[i] = 1.0;
	    
	 maxfilt = 1.0;
      }

      /* Adjust the filter so that the max is 1.0, and */
      /* calculate the K2 scaling factor  */
      for (i=0; i<nn; i++) {
         temporal_filter[i] /= maxfilt;
	 sumfilt = sumfilt + temporal_filter[i];
      }
      sumfilt = sumfilt / nn;

   } else {
      sumfilt = 1.0;
      temporal_filter[0] = 1.0;
      temporal_deltas[0] = 0.0;
   }

   /* the number of additional rows of buckets we put at the edge so
      that the filter doesn't go off the edge */

   gutter_width = (filter_width - oversample) / 2;

    /* Check the size of the density estimation filter. */
    /* If the 'radius' of the density estimation filter is greater than the */
    /* gutter width, we have to pad with more.  Otherwise, we can use the same value. */
   max_gnm_de_fw=0;
   for (gnm_idx = 0; gnm_idx < spec->ngenomes; gnm_idx++) {

      int this_width = (int)ceil(spec->genomes[gnm_idx].estimator) * oversample;
      if (this_width > max_gnm_de_fw)
         max_gnm_de_fw = this_width;
   }

   /* Add OS-1 for the averaging at the edges, if it's > 0 already */
   if (max_gnm_de_fw>0)
      max_gnm_de_fw += (oversample-1);

   /* max_gnm_de_fw is now the number of pixels of additional gutter      */
   /* necessary to appropriately perform the density estimation filtering */
   /* Check to see if it's greater than the gutter_width                  */

   if (max_gnm_de_fw > gutter_width) {
      de_offset = max_gnm_de_fw - gutter_width;
      gutter_width = max_gnm_de_fw;
   } else
      de_offset = 0;

   fic.height = oversample * image_height + 2 * gutter_width;
   fic.width  = oversample * image_width  + 2 * gutter_width;

   nbuckets = (long)fic.width * (long)fic.height;
   if (1) {

     char *last_block = NULL;
     size_t memory_rqd = (sizeof(bucket) * nbuckets +
             sizeof(abucket) * nbuckets +
             4 * sizeof(double) * (size_t)SUB_BATCH_SIZE * spec->nthreads);
     last_block = (char *) malloc(memory_rqd);
     if (NULL == last_block) {
       fprintf(stderr, "render_rectangle: cannot malloc %ld bytes.\n", memory_rqd);
       fprintf(stderr, "render_rectangle: h=%d w=%d nb=%ld.\n", fic.width, fic.height, nbuckets);
       exit(1);
     }
     /* else fprintf(stderr, "render_rectangle: mallocked %dMb.\n", Mb); */

     buckets = (bucket *) last_block;
     accumulate = (abucket *) (last_block + sizeof(bucket) * nbuckets);
     points = (double *)  (last_block + (sizeof(bucket) + sizeof(abucket)) * nbuckets);
   }

   if (verbose) {
     fprintf(stderr, "chaos: ");
     progress_began = time(NULL);
   }

   if (fname) {
      int len = (int)strlen(fname);
      FILE *fin = fopen(fname, "rb");
      int w, h;
      if (len > 4) {
         char *ending = fname + len - 4;
         if (!strcmp(ending, ".png")) {
            cmap2 = read_png(fin, &w, &h);
         } else if  (!strcmp(ending, ".jpg")) {
            cmap2 = read_jpeg(fin, &w, &h);
         }
      }
      if (NULL == cmap2) {
         perror(fname);
         exit(1);
      }
      if (256 != w || 256 != h) {
         fprintf(stderr, "image must be 256 by 256, not %d by %d.\n", w, h);
         exit(1);
      }
   }

   background[0] = background[1] = background[2] = 0.0;
   memset((char *) accumulate, 0, sizeof(abucket) * nbuckets);

   for (batch_num = 0; batch_num < nbatches; batch_num++) {
      double de_time;
      double sample_density=0.0;
      int de_row_size, de_kernel_index=0, de_half_size;
      int de_cutoff_val=0;
      double *de_filter_coefs=NULL,*de_filter_widths=NULL;
      double num_de_filters_d;
      int num_de_filters=0,filtloop,de_max_ind;
      double comp_max_radius,comp_min_radius;
      double k1, area, k2;

      de_time = spec->time + temporal_deltas[batch_num*ntemporal_samples];

      memset((char *) buckets, 0, sizeof(bucket) * nbuckets);

      /* interpolate and get a control point                      */
      /* ONLY FOR DENSITY FILTER WIDTH PURPOSES                   */
      /* additional interpolation will be done in the temporal_sample loop */
      flam3_interpolate(spec->genomes, spec->ngenomes, de_time, &cp);

      /* if instructed to by the genome, create the density estimation */
      /* filter kernels.  Check boundary conditions as well.           */
      if (cp.estimator < 0.0 || cp.estimator_minimum < 0.0) {
         fprintf(stderr,"density estimator filter widths must be >= 0\n");
         exit(1);
      }

      if (spec->bits <= 32) {
     if (cp.estimator > 0.0) {
         fprintf(stderr, "warning: density estimation disabled with %d bit buffers.\n", spec->bits);
         cp.estimator = 0.0;
     }
      }

      if (cp.estimator > 0.0) {


    if (cp.estimator_curve <= 0.0) {
       fprintf(stderr,"estimator curve must be > 0\n");
       exit(1);
    }

         if (cp.estimator < cp.estimator_minimum) {
            fprintf(stderr,"estimator must be larger than estimator_minimum.\n");
            fprintf(stderr,"(%f > %f) ? \n",cp.estimator,cp.estimator_minimum);
            exit(1);
         }

         /* We should scale the filter width by the oversample          */
         /* The '+1' comes from the assumed distance to the first pixel */
         comp_max_radius = cp.estimator*oversample + 1;
         comp_min_radius = cp.estimator_minimum*oversample + 1;

         /* Calculate how many filter kernels we need based on the decay function */
         /*                                                                       */
         /*    num filters = (de_max_width / de_min_width)^(1/estimator_curve)    */
         /*                                                                       */
         num_de_filters_d = pow( comp_max_radius/comp_min_radius, (1.0/cp.estimator_curve) );
         num_de_filters = ceil(num_de_filters_d);
         
         /* Condense the smaller kernels to save space */
         if (num_de_filters>keep_thresh) 
	   de_max_ind = ceil(keep_thresh + pow(num_de_filters-keep_thresh,cp.estimator_curve))+1;
         else
            de_max_ind = num_de_filters;

         /* Allocate the memory for these filters */
         /* and the hit/width lookup vector       */
         de_row_size = 2*ceil(comp_max_radius)-1;
         de_half_size = (de_row_size-1)/2;
         de_kernel_index = (de_half_size+1)*(2+de_half_size)/2;

         de_filter_coefs = (double *) malloc (de_max_ind * de_kernel_index * sizeof(double));
         de_filter_widths = (double *) malloc (de_max_ind * sizeof(double));

         /* Generate the filter coefficients */
         de_cutoff_val = 0;
         for (filtloop=0;filtloop<de_max_ind;filtloop++) {

            double de_filt_sum=0.0, de_filt_d;
            double de_filt_h;
            double dej,dek,adjloop;
            int filter_coef_idx;

            if (filtloop<keep_thresh)
               de_filt_h = (comp_max_radius / pow(filtloop+1,cp.estimator_curve));
            else {
	       adjloop = pow(filtloop-keep_thresh,(1/cp.estimator_curve))+keep_thresh;
               de_filt_h = (comp_max_radius / pow(adjloop+1,cp.estimator_curve));
            }

            if (de_filt_h <= comp_min_radius) {
               de_filt_h = comp_min_radius;
               de_cutoff_val = filtloop;
            }

            de_filter_widths[filtloop] = de_filt_h;

            /* Calculate norm of kernel separately (easier) */
            for (dej=-de_half_size; dej<=de_half_size; dej++) {
               for (dek=-de_half_size; dek<=de_half_size; dek++) {
                  de_filt_d = sqrt( (double)(dej*dej+dek*dek) ) / de_filt_h;

                  if (de_filt_d <= 1.0) {
if (1) {
                     /* Gaussian */
                     de_filt_sum += flam3_spatial_filter(flam3_gaussian_kernel,
                        flam3_spatial_support[flam3_gaussian_kernel]*de_filt_d);
                     //de_filt_sum += flam3_gaussian_filter(flam3_gaussian_support*de_filt_d);
} else {
                     /* Epanichnikov */
                     de_filt_sum += (1.0 - (de_filt_d * de_filt_d));
}
                  }
               }
            }

            filter_coef_idx = filtloop*de_kernel_index;

            /* Calculate the unique entries of the kernel */
            for (dej=0; dej<=de_half_size; dej++) {
               for (dek=0; dek<=dej; dek++) {
                  de_filt_d = sqrt( (double)(dej*dej+dek*dek) ) / de_filt_h;

                  if (de_filt_d>1.0)
                     de_filter_coefs[filter_coef_idx] = 0.0;
                  else {
if (1) {
                     /* Gaussian */
                     de_filter_coefs[filter_coef_idx] =
                         flam3_spatial_filter(flam3_gaussian_kernel,
                            flam3_spatial_support[flam3_gaussian_kernel]*de_filt_d)/de_filt_sum; 
                         //flam3_gaussian_filter(flam3_gaussian_support*de_filt_d)/de_filt_sum;
} else {
                     /* Epanichnikov */
                     de_filter_coefs[filter_coef_idx] = (1.0 - (de_filt_d * de_filt_d))/de_filt_sum;
}
                  }
                  
                  filter_coef_idx ++;
               }
            }

            if (de_cutoff_val>0)
               break;
         }

         if (de_cutoff_val==0)
            de_cutoff_val = num_de_filters-1;

      }
      
#if 0
      printf("DE Filters: %d\n",de_max_ind);
      if (0) { int dej,dek1,dek2,fci;
      fci=0;
      for (dej=0; dej<de_max_ind;dej++) {
         printf("de_filter_widths[%d] = %f\n",dej,de_filter_widths[dej]);
         for (dek1=0;dek1<=de_half_size;dek1++) {
            for (dek2=0;dek2<=dek1;dek2++) {
               printf("%f ",de_filter_coefs[fci++]);
            }
            printf("\n");
         }
         printf("--------------------------\n");
      }
      
      }
#endif
      for (temporal_sample_num = 0; temporal_sample_num < ntemporal_samples; temporal_sample_num++) {
         double temporal_sample_time;
         double color_scalar = temporal_filter[batch_num*ntemporal_samples + temporal_sample_num];

         temporal_sample_time = spec->time +
        temporal_deltas[batch_num*ntemporal_samples + temporal_sample_num];

         /* Interpolate and get a control point */
         flam3_interpolate(spec->genomes, spec->ngenomes, temporal_sample_time, &cp);

         prepare_xform_fn_ptrs(&cp, &spec->rc);

         /* compute the colormap entries.                             */
         /* the input colormap is 256 long with entries from 0 to 1.0 */
         if (!fname) {
            for (j = 0; j < CMAP_SIZE; j++) {
               for (k = 0; k < 3; k++) {
#if 1
//                  cmap[j][k] = (int) (cp.palette[(j * 256) / CMAP_SIZE][k] * WHITE_LEVEL);
                  dmap[j][k] = (cp.palette[(j * 256) / CMAP_SIZE][k] * WHITE_LEVEL);
#else
                  /* monochrome if you don't have any cmaps */
//                  dmap[j][k] = WHITE_LEVEL;
#endif
               }
//               cmap[j][3] = WHITE_LEVEL;
               dmap[j][3] = WHITE_LEVEL;
            }
         }

         /* compute camera */
         if (1) {
            double t0, t1, shift=0.0, corner0, corner1;
            double scale;

            if (cp.sample_density <= 0.0) {
              fprintf(stderr,
                 "sample density (quality) must be greater than zero,"
                 " not %g.\n", cp.sample_density);
              exit(1);
            }

            scale = pow(2.0, cp.zoom);
            sample_density = cp.sample_density * scale * scale;

            ppux = cp.pixels_per_unit * scale;
            ppuy = field ? (ppux / 2.0) : ppux;
            ppux /=  spec->pixel_aspect_ratio;
            switch (field) {
               case flam3_field_both: shift =  0.0; break;
               case flam3_field_even: shift = -0.5; break;
               case flam3_field_odd:  shift =  0.5; break;
            }
            shift = shift / ppux;
            t0 = (double) gutter_width / (oversample * ppux);
            t1 = (double) gutter_width / (oversample * ppuy);
            corner0 = cp.center[0] - image_width / ppux / 2.0;
            corner1 = cp.center[1] - image_height / ppuy / 2.0;
            fic.bounds[0] = corner0 - t0;
            fic.bounds[1] = corner1 - t1 + shift;
            fic.bounds[2] = corner0 + image_width  / ppux + t0;
            fic.bounds[3] = corner1 + image_height / ppuy + t1 + shift;
            fic.size[0] = 1.0 / (fic.bounds[2] - fic.bounds[0]);
            fic.size[1] = 1.0 / (fic.bounds[3] - fic.bounds[1]);
            fic.rot[0][0] = cos(cp.rotate * 2 * M_PI / 360.0);
            fic.rot[0][1] = -sin(cp.rotate * 2 * M_PI / 360.0);
            fic.rot[1][0] = -fic.rot[0][1];
            fic.rot[1][1] = fic.rot[0][0];
            fic.ws0 = fic.width * fic.size[0];
            fic.wb0s0 = fic.ws0 * fic.bounds[0];
            fic.hs1 = fic.height * fic.size[1];
            fic.hb1s1 = fic.hs1 * fic.bounds[1];

         }

//         nsamples = sample_density * nbuckets / (oversample * oversample);
         nsamples = sample_density * image_width * image_height;
#if 0
         fprintf(stderr, "sample_density=%g nsamples=%g nbuckets=%ld time=%g\n",
       sample_density, nsamples, nbuckets, temporal_sample_time);
#endif

         batch_size = nsamples / (nbatches * ntemporal_samples);

         stats->num_iters += batch_size;

         /* Set up the xform_distrib array */
         flam3_create_xform_distrib(&cp,xform_distrib);

         /* Fill in the iter constants */
         fic.xform_distrib = xform_distrib;
         fic.spec = spec;
         fic.batch_size = batch_size / (double)spec->nthreads;
         fic.temporal_sample_num = temporal_sample_num;
         fic.ntemporal_samples = ntemporal_samples;
         fic.batch_num = batch_num;
         fic.nbatches = nbatches;

         if (fname) {
            fic.fname_specified = 1;
            fic.cmap = (void *)cmap2;
         } else {
            fic.fname_specified = 0;
//            fic.cmap = (void *)cmap;
            fic.dmap = (void *)dmap;
         }

         fic.color_scalar = color_scalar;
         fic.buckets = (void *)buckets;

         /* Initialize the thread helper structures */
         for (thi = 0; thi < spec->nthreads; thi++)
         {
            int rk;
            /* Create a new isaac state for this thread */
            for (rk = 0; rk < RANDSIZ; rk++)
               fth[thi].rc.randrsl[rk] = irand(&spec->rc);

            irandinit(&(fth[thi].rc),1);

            if (0==thi) {

               fth[thi].first_thread=1;
               if (0==batch_num && 0==temporal_sample_num)
               	fth[thi].timer_initialize = 1;
               else
               	fth[thi].timer_initialize = 0;
               	
            } else {
               fth[thi].first_thread=0;
	         	fth[thi].timer_initialize = 0;
            }

            fth[thi].iter_storage = &(points[thi*SUB_BATCH_SIZE*4]);
            fth[thi].fic = &fic;
            flam3_copy(&(fth[thi].cp),&cp);

         }

#ifdef HAVE_LIBPTHREAD
         /* Let's make some threads */
         myThreads = (pthread_t *)malloc(spec->nthreads * sizeof(pthread_t));

         #if defined(USE_LOCKS)
         pthread_mutex_init(&bucket_mutex, NULL);
         #endif

         pthread_attr_init(&pt_attr);
         pthread_attr_setdetachstate(&pt_attr,PTHREAD_CREATE_JOINABLE);

         for (thi=0; thi <spec->nthreads; thi ++)
            pthread_create(&myThreads[thi], &pt_attr, (void *)iter_thread, (void *)(&(fth[thi])));

         pthread_attr_destroy(&pt_attr);

         /* Wait for them to return */
         for (thi=0; thi < spec->nthreads; thi++)
            pthread_join(myThreads[thi], (void **)&thread_status);

         #if defined(USE_LOCKS)
         pthread_mutex_destroy(&bucket_mutex);
         #endif
#else
         for (thi=0; thi < spec->nthreads; thi++)
            iter_thread( (void *)(&(fth[thi])) );
#endif

    /* We have to free the xform data allocated in the flam3_copy call above */
         for (thi=0; thi < spec->nthreads; thi++) {
            free(fth[thi].cp.xform);
            fth[thi].cp.num_xforms=0;
         }
             
	 if (fic.aborted) {
	   if (verbose) fprintf(stderr, "\naborted!\n");
	   goto done;
	 }

         vibrancy += cp.vibrancy;
         gamma += cp.gamma;
         background[0] += cp.background[0];
         background[1] += cp.background[1];
         background[2] += cp.background[2];
         vib_gam_n++;

      }

      k1 =(cp.contrast * cp.brightness *
      PREFILTER_WHITE * 268.0 ) / 256;
      area = image_width * image_height / (ppux * ppuy);
      k2 = (oversample * oversample * nbatches) /
   (cp.contrast * area * WHITE_LEVEL * sample_density * sumfilt);
#if 0
      printf("iw=%d,ih=%d,ppux=%f,ppuy=%f\n",image_width,image_height,ppux,ppuy);
      printf("contrast=%f, brightness=%f, PREFILTER=%d, temporal_filter=%f\n",
        cp.contrast, cp.brightness, PREFILTER_WHITE, temporal_filter[batch_num]);
      printf("oversample=%d, nbatches=%d, area = %f, WHITE_LEVEL=%d, sample_density=%f\n",
        oversample, nbatches, area, WHITE_LEVEL, sample_density);
      printf("k1=%f,k2=%15.12f\n",k1,k2);
#endif

      if (num_de_filters == 0) {

         for (j = 0; j < fic.height; j++) {
            for (i = 0; i < fic.width; i++) {

               abucket *a = accumulate + i + j * fic.width;
               bucket *b = buckets + i + j * fic.width;
               double c[4], ls;

               c[0] = (double) b[0][0];
               c[1] = (double) b[0][1];
               c[2] = (double) b[0][2];
               c[3] = (double) b[0][3];
               if (0.0 == c[3])
                  continue;

               ls = (k1 * log(1.0 + c[3] * k2))/c[3];
               c[0] *= ls;
               c[1] *= ls;
               c[2] *= ls;
               c[3] *= ls;

               abump_no_overflow(a[0][0], c[0]);
               abump_no_overflow(a[0][1], c[1]);
               abump_no_overflow(a[0][2], c[2]);
               abump_no_overflow(a[0][3], c[3]);
            }
         }
      } else {
      
         int ss = (int)floor(oversample / 2.0);
         int scf = !((int)oversample & 1);
         double scfact = pow(oversample/(oversample+1.0), 2.0);
         
         /* Density estimation code */
         /* Remember, we already padded with an extra pixel at the beginning */
         for (j = oversample-1; j < fic.height-(oversample-1); j++) {
            for (i = oversample-1; i < fic.width-(oversample-1); i++) {

               int ii,jj;
               double f_select=0.0;
               int f_select_int,f_coef_idx;
               int arr_filt_width;
               bucket *b;
               double c[4],ls;

               b = buckets + i + j*fic.width;

               /* Don't do anything if there's no hits here */
               if (b[0][3] == 0)
                  continue;

               /* Count density in ssxss area   */
               /* Scale if OS>1 for equal iters */
               for (ii=-ss; ii<=ss; ii++) {
                  for (jj=-ss; jj<=ss; jj++) {
                     b = buckets + (i + ii) + (j + jj)*fic.width;
                     f_select += b[0][3]/255.0;
                  }
               }
               
               if (scf)
                  f_select *= scfact;
                  
               if (f_select<=keep_thresh)
                  f_select_int = (int)ceil(f_select)-1;
               else
		 f_select_int = (int)keep_thresh +
		   (int)floor(pow(f_select-keep_thresh,cp.estimator_curve));

               /* If the filter selected below the min specified clamp it to the min */
               if (f_select_int >= de_cutoff_val)
                  f_select_int = de_cutoff_val;

               /* We only have to calculate the values for ~1/8 of the square */
               f_coef_idx = f_select_int*de_kernel_index;

               arr_filt_width = (int)ceil(de_filter_widths[f_select_int])-1;

               b = buckets + i + j*fic.width;

               for (jj=0; jj<=arr_filt_width; jj++) {
                  for (ii=0; ii<=jj; ii++) {

                     /* Skip if coef is 0 */
                     if (de_filter_coefs[f_coef_idx]==0.0) {
                        f_coef_idx++;
                        continue;
                     }
                     
                     c[0] = (double) b[0][0];
                     c[1] = (double) b[0][1];
                     c[2] = (double) b[0][2];
                     c[3] = (double) b[0][3];

                     ls = de_filter_coefs[f_coef_idx]*(k1 * log(1.0 + c[3] * k2))/c[3];

                     c[0] *= ls;
                     c[1] *= ls;
                     c[2] *= ls;
                     c[3] *= ls;

                     if (jj==0 && ii==0) {
                        add_c_to_accum(accumulate,i,ii,j,jj,fic.width,fic.height,c);
                     }
                     else if (ii==0) {
                        add_c_to_accum(accumulate,i,jj,j,0,fic.width,fic.height,c);
                        add_c_to_accum(accumulate,i,-jj,j,0,fic.width,fic.height,c);
                        add_c_to_accum(accumulate,i,0,j,jj,fic.width,fic.height,c);
                        add_c_to_accum(accumulate,i,0,j,-jj,fic.width,fic.height,c);
                     } else if (jj==ii) {
                        add_c_to_accum(accumulate,i,ii,j,jj,fic.width,fic.height,c);
                        add_c_to_accum(accumulate,i,-ii,j,jj,fic.width,fic.height,c);
                        add_c_to_accum(accumulate,i,ii,j,-jj,fic.width,fic.height,c);
                        add_c_to_accum(accumulate,i,-ii,j,-jj,fic.width,fic.height,c);
                     } else {
                        add_c_to_accum(accumulate,i,ii,j,jj,fic.width,fic.height,c);
                        add_c_to_accum(accumulate,i,-ii,j,jj,fic.width,fic.height,c);
                        add_c_to_accum(accumulate,i,ii,j,-jj,fic.width,fic.height,c);
                        add_c_to_accum(accumulate,i,-ii,j,-jj,fic.width,fic.height,c);
                        add_c_to_accum(accumulate,i,jj,j,ii,fic.width,fic.height,c);
                        add_c_to_accum(accumulate,i,-jj,j,ii,fic.width,fic.height,c);
                        add_c_to_accum(accumulate,i,jj,j,-ii,fic.width,fic.height,c);
                        add_c_to_accum(accumulate,i,-jj,j,-ii,fic.width,fic.height,c);
                     }

                     f_coef_idx++;

                  }
               }

	       if (0) {
		 abucket *a = accumulate + i + j * fic.width;
		 a[0][0] = f_select_int;
	       }
            }
       if (verbose && time(NULL) != progress_timer) {
      progress_timer = time(NULL);
      fprintf(stderr, "\rdensity estimation: %d/%d          ", j, fic.height);
      fflush(stderr);
       }


      if (fic.spec->progress) {
	if ((*fic.spec->progress)(fic.spec->progress_parameter,
				  j/(double)fic.height, 1, 0.0)) {
	   if (verbose) fprintf(stderr, "\naborted!\n");
	  goto done;
	  }
      }

         }
         
      } /* End density estimation loop */


      /* If allocated, free the de filter memory for the next batch */
      if (num_de_filters > 0) {
         free(de_filter_coefs);
         free(de_filter_widths);
      }

   }

   if (verbose) {
     fprintf(stderr, "\rchaos: 100.0%%  took: %ld seconds   \n", time(NULL) - progress_began);
     fprintf(stderr, "filtering...");
   }
   

   /* filter the accumulation buffer down into the image */
   if (1) {
      int x, y;
      double t[4];
      double g = 1.0 / (gamma / vib_gam_n);
      double tmp;

      double linrange = cp.gam_lin_thresh;
      double funcval = pow(linrange,g);
      double frac;

      vibrancy /= vib_gam_n;
      background[0] /= vib_gam_n/256.0;
      background[1] /= vib_gam_n/256.0;
      background[2] /= vib_gam_n/256.0;
      y = de_offset;

      for (j = 0; j < image_height; j++) {
         x = de_offset;
         for (i = 0; i < image_width; i++) {
            int ii, jj;
            void *p;
            unsigned short *p16;
            unsigned char *p8;
            double ls, a;
            double alpha;
            t[0] = t[1] = t[2] = t[3] = 0.0;
            for (ii = 0; ii < filter_width; ii++) {
               for (jj = 0; jj < filter_width; jj++) {
                  double k = filter[ii + jj * filter_width];
                  abucket *ac = accumulate + x + ii + (y + jj) * fic.width;

                  t[0] += k * ac[0][0];
                  t[1] += k * ac[0][1];
                  t[2] += k * ac[0][2];
                  t[3] += k * ac[0][3];

               }
            }

            p = out + nchan * bytes_per_channel * (i + j * out_width);
            p8 = (unsigned char *)p;
            p16 = (unsigned short *)p;

            if (t[3] > 0.0) {

               tmp = t[3]/PREFILTER_WHITE;

               if (tmp<=linrange) {
                  /* Small Gamma Linearization */
                  frac = tmp/linrange;
                  alpha = (1.0-frac) * tmp * (funcval / linrange) + frac * pow(tmp,g);
               } else {
                  /* Standard */
                  alpha = pow(tmp, g);
               }

               ls = vibrancy * 256.0 * alpha / tmp;

               if (alpha < 0.0)
                  alpha = 0.0;
               else if (alpha > 1.0)
                  alpha = 1.0;



            } else {
               ls = 0.0;
               alpha = 0.0;
            }

            /* Clamp negatives to 0 (for slightly negative kernels) */
            if (1) {
               if (t[0]<0)
                  t[0]=0;
               if (t[1]<0)
                  t[1]=0;
               if (t[2]<0)
                  t[2]=0;
            }

            /* apply to rgb channels the relative scale from gamma of alpha channel */
            /* red */
            a = ls * ((double) t[0] / PREFILTER_WHITE);
            a += (1.0-vibrancy) * 256.0 * pow((double) t[0] / PREFILTER_WHITE, g);
            if (nchan<=3 || transp==0)
               a += ((1.0 - alpha) * background[0]);
            else
               a /= alpha;

            if (a > 255)
               a = 255;
            if (a < 0)
               a = 0;
               
            if (2==bytes_per_channel) {
               a *= 256.0; /* Scales to [0-65280] */
               p16[0] = (unsigned short) a;
            } else {
               p8[0] = (unsigned char) a;
            }

            //p[0] = (unsigned char) a;

            /* green */
            a = ls * ((double) t[1] / PREFILTER_WHITE);
            a += (1.0-vibrancy) * 256.0 * pow((double) t[1] / PREFILTER_WHITE, g);
            if (nchan<=3 || transp==0)
               a += ((1.0 - alpha) * background[1]);
            else
               a /= alpha;

            if (a > 255)
               a = 255;
            if (a < 0)
               a = 0;

            if (2==bytes_per_channel) {
               a *= 256.0; /* Scales to [0-65280] */
               p16[1] = (unsigned short) a;
            } else {
               p8[1] = (unsigned char) a;
            }

//            p[1] = (unsigned char) a;

            /* blue */
            a = ls * ((double) t[2] / PREFILTER_WHITE);
            a += (1.0-vibrancy) * 256.0 * pow((double) t[2] / PREFILTER_WHITE, g);
            if (nchan<=3 || transp==0)
               a += ((1.0 - alpha) * background[2]);
            else
               a /= alpha;

            if (a > 255)
               a = 255;
            if (a < 0)
               a = 0;

            if (2==bytes_per_channel) {
               a *= 256.0; /* Scales to [0-65280] */
               p16[2] = (unsigned short) a;
            } else {
               p8[2] = (unsigned char) a;
            }

//            p[2] = (unsigned char) a;

            /* alpha */
            if (nchan>3) {
               if (transp==1) {
                  if (2==bytes_per_channel)
                     p16[3] = (unsigned short) (alpha * 65535.999);
                  else
                     p8[3] = (unsigned char) (alpha * 255.999);
               } else {
                  if (2==bytes_per_channel)
                     p16[3] = 65535;
                  else
                     p8[3] = 255;
               }
            }

            x += oversample;
         }
         y += oversample;
      }
   }

 done:

   stats->badvals = fic.badvals;

   free(temporal_filter);
   free(temporal_deltas);
//   free(motion_filter);
#ifdef HAVE_LIBPTHREAD
   free(myThreads);
#endif
   free(filter);
   free(buckets);
//   free(accumulate);
//   free(points);
   free(fth);
   /* Free the xform in cp */
   free(cp.xform);
   cp.num_xforms=0;
   if (fname) free(cmap2);

   if (getenv("insert_palette")) {
     int ph = 100;
     if (ph >= image_height) ph = image_height;
     /* insert the palette into the image */
     for (j = 0; j < ph; j++) {
       for (i = 0; i < image_width; i++) {
	 unsigned char *p = out + nchan * (i + j * out_width);
	 p[0] = (unsigned char)cmap[i * 256 / image_width][0];
	 p[1] = (unsigned char)cmap[i * 256 / image_width][1];
	 p[2] = (unsigned char)cmap[i * 256 / image_width][2];
       }
     }
   }

   tend = time(NULL);
   stats->render_seconds = (int)(tend-tstart);

}
