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
#include "flam3.h"

#define FLAM3_PNG_COM 8

#ifdef WIN32
   #define snprintf _snprintf
#endif

void write_jpeg(FILE *file, unsigned char *image, int width, int height, flam3_img_comments *fpc);
void write_png(FILE *file, void *image, int width, int height, flam3_img_comments *fpc, int bpc);

/* returns RGBA pixel array or NULL on failure */
unsigned char *read_png(FILE *file, int *width, int *height);
unsigned char *read_jpeg(FILE *file, int *width, int *height);
