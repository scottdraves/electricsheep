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
#include <jpeglib.h>
#include <string.h>

#include "config.h"
#include "img.h"
#include "flam3.h"
#include "private.h"

void
write_jpeg(FILE *file, unsigned char *image, int width, int height, flam3_img_comments *fpc) {
   struct jpeg_compress_struct info;
   struct jpeg_error_mgr jerr;
   size_t i;
   char *nick = getenv("nick");
   char *url = getenv("url");
   char *id = getenv("id");
   char *ai; /* For argi */
   int jpegcom_enable = argi("enable_jpeg_comments",1);

   char nick_string[64], url_string[128], id_string[128];
   char bv_string[64],ni_string[64],rt_string[64];
   char genome_string[65536], ver_string[64];
   
   /* Create the mandatory comment strings */
   snprintf(genome_string,65536,"flam3_genome: %s",fpc->genome);
   snprintf(bv_string,64,"flam3_error_rate: %s",fpc->badvals);
   snprintf(ni_string,64,"flam3_samples: %s",fpc->numiters);
   snprintf(rt_string,64,"flam3_time: %s",fpc->rtime);
   snprintf(ver_string,64,"flam3_version: %s",flam3_version());

   info.err = jpeg_std_error(&jerr);
   jpeg_create_compress(&info);
   jpeg_stdio_dest(&info, file);
   info.in_color_space = JCS_RGB;
   info.input_components = 3;
   info.image_width = width;
   info.image_height = height;
   jpeg_set_defaults(&info);

   if (getenv("jpeg")) {
      int quality = atoi(getenv("jpeg"));
	   jpeg_set_quality(&info, quality, TRUE);
   }
   
   jpeg_start_compress(&info, TRUE);
    
   /* Write comments to jpeg */
   if (jpegcom_enable==1) {
        jpeg_write_marker(&info, JPEG_COM, (unsigned char *)ver_string, (int)strlen(ver_string));

        if (0 != nick) {
            snprintf(nick_string,64,"flam3_nickname: %s",nick);
            jpeg_write_marker(&info, JPEG_COM, (unsigned char *)nick_string, (int)strlen(nick_string));
        }

        if (0 != url) {
            snprintf(url_string,128,"flam3_url: %s",url);
            jpeg_write_marker(&info, JPEG_COM, (unsigned char *)url_string, (int)strlen(url_string));
        }
        
        if (0 != id) {
            snprintf(id_string,128,"flam3_id: %s",id);
            jpeg_write_marker(&info, JPEG_COM, (unsigned char *)id_string, (int)strlen(id_string));
        }

        jpeg_write_marker(&info, JPEG_COM, (unsigned char *)bv_string, (int)strlen(bv_string));
        jpeg_write_marker(&info, JPEG_COM, (unsigned char *)ni_string, (int)strlen(ni_string));
        jpeg_write_marker(&info, JPEG_COM, (unsigned char *)rt_string, (int)strlen(rt_string));
        jpeg_write_marker(&info, JPEG_COM, (unsigned char *)genome_string, (int)strlen(genome_string));
    }

    for (i = 0; i < height; i++) {
	JSAMPROW row_pointer[1];
	row_pointer[0] = (unsigned char *) image + (3 * width * i);
	jpeg_write_scanlines(&info, row_pointer, 1);
    }
    jpeg_finish_compress(&info);
    jpeg_destroy_compress(&info);
}

unsigned char *read_jpeg(FILE *ifp, int *width, int *height) {
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    int num_scanlines;
    unsigned char *p, *q, *t;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, ifp);
    (void) jpeg_read_header(&cinfo, TRUE);
    (void) jpeg_start_decompress(&cinfo);

    /* or image_width? */
    *width = cinfo.output_width;
    *height = cinfo.output_height;

    if (3 != cinfo.output_components) {
	printf("can only read RGB JPEG files, not %d components.\n",
	       cinfo.output_components);
	return 0;
    }
    p = q = malloc(4 * *width * *height);
    t = malloc(3 * *width);

    while (cinfo.output_scanline < cinfo.output_height) {
	unsigned char *s = t;
	int i;
	num_scanlines = jpeg_read_scanlines(&cinfo, &t, 1);
	for (i = 0; i < *width; i++) {
	    p[0] = s[0];
	    p[1] = s[1];
	    p[2] = s[2];
	    p[3] = 255;
	    s += 3;
	    p += 4;
	}
    }

    (void) jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    free(t);
    return q;
}
