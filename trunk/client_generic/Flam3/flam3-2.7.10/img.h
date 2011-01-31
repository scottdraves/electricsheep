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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


static char *jpeg_h_id =
"@(#) $Id: img.h,v 1.7 2007/07/19 17:39:19 reckase Exp $";

#include <stdio.h>
#include "flam3.h"

#define FLAM3_PNG_COM 7

#ifdef WIN32
   #define snprintf _snprintf
#endif

void write_jpeg(FILE *file, unsigned char *image, int width, int height, flam3_img_comments *fpc);
void write_png(FILE *file, unsigned char *image, int width, int height, flam3_img_comments *fpc);

/* returns RGBA pixel array or NULL on failure */
unsigned char *read_png(FILE *file, int *width, int *height);
unsigned char *read_jpeg(FILE *file, int *width, int *height);
