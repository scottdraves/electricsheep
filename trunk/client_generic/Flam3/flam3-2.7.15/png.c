/*
    flame - cosmic recursive fractal flames
    Copyright (C) 2002-2003  Scott Draves <source@flam3.com>

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


#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <setjmp.h>

#include "config.h"
#include "img.h"
#include "flam3.h"
#include "private.h"

#ifdef WIN32
#include <winsock.h> /* for htons */
#endif

void write_png(FILE *file, void *image, int width, int height, flam3_img_comments *fpc, int bpc) {
  png_structp  png_ptr;
  png_infop    info_ptr;
  png_text     text[FLAM3_PNG_COM];
  size_t i;
  unsigned short testbe = 1;
  void **rows = malloc(sizeof(void *) * height);
  char *nick = getenv("nick");
  char *url = getenv("url");
  char *id = getenv("id");
  char *ai; /* For argi */
  int pngcom_enable = argi("enable_png_comments", 1);

  text[0].compression = PNG_TEXT_COMPRESSION_NONE;
  text[0].key = "flam3_version";
  text[0].text = flam3_version();

  text[1].compression = PNG_TEXT_COMPRESSION_NONE;
  text[1].key = "flam3_nickname";
  text[1].text = nick;

  text[2].compression = PNG_TEXT_COMPRESSION_NONE;
  text[2].key = "flam3_url";
  text[2].text = url;
  
  text[3].compression = PNG_TEXT_COMPRESSION_NONE;
  text[3].key = "flam3_id";
  text[3].text = id;

  text[4].compression = PNG_TEXT_COMPRESSION_NONE;
  text[4].key = "flam3_error_rate";
  text[4].text = fpc->badvals;

  text[5].compression = PNG_TEXT_COMPRESSION_NONE;
  text[5].key = "flam3_samples";
  text[5].text = fpc->numiters;

  text[6].compression = PNG_TEXT_COMPRESSION_NONE;
  text[6].key = "flam3_time";
  text[6].text = fpc->rtime;

  text[7].compression = PNG_TEXT_COMPRESSION_zTXt;
  text[7].key = "flam3_genome";
  text[7].text = fpc->genome;

  for (i = 0; i < height; i++)
    rows[i] = image + i * width * 4 * bpc;
      
  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
				    NULL, NULL, NULL);
  info_ptr = png_create_info_struct(png_ptr);

  if (setjmp(png_jmpbuf(png_ptr))) {
     fclose(file);
     png_destroy_write_struct(&png_ptr, &info_ptr);
     perror("writing file");
     return;
  }
  png_init_io(png_ptr, file);

  png_set_IHDR(png_ptr, info_ptr, width, height, 8*bpc,
	       PNG_COLOR_TYPE_RGBA,
	       PNG_INTERLACE_NONE,
	       PNG_COMPRESSION_TYPE_BASE,
	       PNG_FILTER_TYPE_BASE);
	       
  /* Swap the bytes if we're doing 16bpc and on little-endian platform */       
  if (2==bpc && testbe != htons(testbe)) {
    unsigned short *im = (unsigned short *)image;
    for (i=0; i<height*width*4; i++) {
       im[i] = htons(im[i]);
    }
  }
      
  

  if (pngcom_enable==1)
	  png_set_text(png_ptr, info_ptr, text, FLAM3_PNG_COM);

  png_write_info(png_ptr, info_ptr);
  png_write_image(png_ptr, (png_bytepp) rows);
  png_write_end(png_ptr, info_ptr);
  png_destroy_write_struct(&png_ptr, &info_ptr);
  free(rows);
}

#define SIG_CHECK_SIZE 8

unsigned char *read_png(FILE *ifp, int *width, int *height) {
  unsigned char sig_buf [SIG_CHECK_SIZE];
  png_struct *png_ptr;
  png_info *info_ptr;
  png_byte **png_image = NULL;
  unsigned int linesize, x, y;
  unsigned char *p, *q;

  if (fread (sig_buf, 1, SIG_CHECK_SIZE, ifp) != SIG_CHECK_SIZE) {
    fprintf (stderr, "input file empty or too short\n");
    return 0;
  }
  if (png_sig_cmp (sig_buf, (png_size_t) 0, (png_size_t) SIG_CHECK_SIZE) != 0) {
    fprintf (stderr, "input file not a PNG file\n");
    return 0;
  }

  png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png_ptr == NULL) {
    fprintf (stderr, "cannot allocate LIBPNG structure\n");
    return 0;
  }
  if (setjmp(png_jmpbuf(png_ptr))) {
     if (png_image) {
	 for (y = 0 ; y < info_ptr->height ; y++)
	     free (png_image[y]);
	 free (png_image);
     }
     png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
     perror("reading file");
     return 0;
  }
  info_ptr = png_create_info_struct (png_ptr);
  if (info_ptr == NULL) {
    png_destroy_read_struct (&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
    fprintf (stderr, "cannot allocate LIBPNG structures\n");
    return 0;
  }

  png_init_io (png_ptr, ifp);
  png_set_sig_bytes (png_ptr, SIG_CHECK_SIZE);
  png_read_info (png_ptr, info_ptr);

  if (8 != info_ptr->bit_depth) {
    fprintf(stderr, "bit depth type must be 8, not %d.\n",
	    info_ptr->bit_depth);
    return 0;
  }

  *width = info_ptr->width;
  *height = info_ptr->height;
  p = q = malloc(4 * *width * *height);
  png_image = (png_byte **)malloc (info_ptr->height * sizeof (png_byte*));

  linesize = info_ptr->width;
  switch (info_ptr->color_type) {
    case PNG_COLOR_TYPE_RGB:
      linesize *= 3;
      break;
    case PNG_COLOR_TYPE_RGBA:
      linesize *= 4;
      break;
  default:
    fprintf(stderr, "color type must be RGB or RGBA not %d.\n",
	    info_ptr->color_type);
    return 0;
  }

  for (y = 0 ; y < info_ptr->height ; y++) {
    png_image[y] = malloc (linesize);
  }
  png_read_image (png_ptr, png_image);
  png_read_end (png_ptr, info_ptr);

  for (y = 0 ; y < info_ptr->height ; y++) {
    unsigned char *s = png_image[y];
    for (x = 0 ; x < info_ptr->width ; x++) {

      switch (info_ptr->color_type) {
      case PNG_COLOR_TYPE_RGB:
	p[0] = s[0];
	p[1] = s[1];
	p[2] = s[2];
	p[3] = 255;
	s += 3;
	p += 4;
	break;
      case PNG_COLOR_TYPE_RGBA:
	p[0] = s[0];
	p[1] = s[1];
	p[2] = s[2];
	p[3] = s[3];
	s += 4;
	p += 4;
	break;
      }
    }
  }

  for (y = 0 ; y < info_ptr->height ; y++)
    free (png_image[y]);
  free (png_image);
  png_destroy_read_struct (&png_ptr, &info_ptr, (png_infopp)NULL);  

  return q;
}
