/*
    flame - cosmic recursive fractal flames
    Copyright (C) 2003  Scott Draves <source@flam3.com>

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

static char *libifs_c_id =
"@(#) $Id: flam3-convert.c,v 1.10 2007/12/20 20:48:25 reckase Exp $";

#include "private.h"


#define MAXARGS 1000
#define streql(x,y) (!strcmp(x,y))


/*
 * split a string passed in ss into tokens on whitespace.
 * # comments to end of line.  ; terminates the record
 */
void tokenize(ss, argv, argc)
   char **ss;
   char *argv[];
   int *argc;
{
   char *s = *ss;
   int i = 0, state = 0;

   while (*s != ';') {
      char c = *s;
      switch (state) {
       case 0:
	 if ('#' == c)
	    state = 2;
	 else if (!isspace(c)) {
	    argv[i] = s;
	    i++;
	    state = 1;
	 }
       case 1:
	 if (isspace(c)) {
	    *s = 0;
	    state = 0;
	 }
       case 2:
	 if ('\n' == c)
	    state = 0;
      }
      s++;
   }
   *s = 0;
   *ss = s+1;
   *argc = i;
}

/*
 * given a pointer to a string SS, fill fields of a control point CP.
 * return a pointer to the first unused char in SS.  totally barfucious,
 * must integrate with tcl soon...
 */

void parse_control_point_old(char **ss, flam3_genome *cp) {
   char *argv[MAXARGS];
   int argc, i, j;
   int set_cm = 0, set_image_size = 0, set_nbatches = 0, set_white_level = 0, set_cmap_inter = 0;
   int set_spatial_oversample = 0, set_hr = 0;
   double *slot, xf, cm, t, nbatches, white_level, spatial_oversample, cmap_inter;
   double image_size[2];

   memset(cp, 0, sizeof(flam3_genome));

   flam3_add_xforms(cp, flam3_nxforms);

   for (i = 0; i < flam3_nxforms; i++) {
      cp->xform[i].density = 0.0;
      cp->xform[i].color[0] = i&1;
      cp->xform[i].color[1] = (i&2)>>1;
      cp->xform[i].symmetry = 0;
      cp->xform[i].var[0] = 1.0;
      for (j = 1; j < flam3_nvariations; j++)
	 cp->xform[i].var[j] = 0.0;
      cp->xform[i].c[0][0] = 1.0;
      cp->xform[i].c[0][1] = 0.0;
      cp->xform[i].c[1][0] = 0.0;
      cp->xform[i].c[1][1] = 1.0;
      cp->xform[i].c[2][0] = 0.0;
      cp->xform[i].c[2][1] = 0.0;
   }

   tokenize(ss, argv, &argc);
   for (i = 0; i < argc; i++) {
      if (streql("xform", argv[i]))
	 slot = &xf;
      else if (streql("time", argv[i]))
	 slot = &cp->time;
      else if (streql("brightness", argv[i]))
	 slot = &cp->brightness;
      else if (streql("contrast", argv[i]))
	 slot = &cp->contrast;
      else if (streql("gamma", argv[i]))
	 slot = &cp->gamma;
      else if (streql("vibrancy", argv[i]))
	 slot = &cp->vibrancy;
      else if (streql("hue_rotation", argv[i])) {
	 slot = &cp->hue_rotation;
	 set_hr = 1;
      } else if (streql("zoom", argv[i]))
	 slot = &cp->zoom;
      else if (streql("image_size", argv[i])) {
	 slot = image_size;
	 set_image_size = 1;
      } else if (streql("center", argv[i]))
	 slot = cp->center;
      else if (streql("background", argv[i]))
	 slot = cp->background;
      else if (streql("pixels_per_unit", argv[i]))
	 slot = &cp->pixels_per_unit;
      else if (streql("spatial_filter_radius", argv[i]))
	 slot = &cp->spatial_filter_radius;
      else if (streql("sample_density", argv[i]))
	 slot = &cp->sample_density;
      else if (streql("nbatches", argv[i])) {
	 slot = &nbatches;
	 set_nbatches = 1;
      } else if (streql("white_level", argv[i])) {
	 slot = &white_level;
	 set_white_level = 1;
      } else if (streql("spatial_oversample", argv[i])) {
	 slot = &spatial_oversample;
	 set_spatial_oversample = 1;
      } else if (streql("cmap", argv[i])) {
	 slot = &cm;
	 set_cm = 1;
      } else if (streql("palette", argv[i])) {
	  slot = &cp->palette[0][0];
      } else if (streql("density", argv[i]))
	 slot = &cp->xform[(int)xf].density;
      else if (streql("color", argv[i]))
	 slot = &cp->xform[(int)xf].color[0];
      else if (streql("coefs", argv[i])) {
	 slot = cp->xform[(int)xf].c[0];
	 cp->xform[(int)xf].density = 1.0;
       } else if (streql("var", argv[i]))
	 slot = cp->xform[(int)xf].var;
      else if (streql("cmap_inter", argv[i])) {
	slot = &cmap_inter;
	set_cmap_inter = 1;
      } else
	 *slot++ = atof(argv[i]);
   }
   if (set_cm) {
       double hr = set_hr ? cp->hue_rotation : 0.0;
      cp->palette_index = (int) cm;
      flam3_get_palette(cp->palette_index, cp->palette, hr);
   }
   if (set_image_size) {
      cp->width  = (int) image_size[0];
      cp->height = (int) image_size[1];
   }
   if (set_nbatches)
      cp->nbatches = (int) nbatches;
   if (set_spatial_oversample)
      cp->spatial_oversample = (int) spatial_oversample;
   if (set_white_level) {
     /* ignore */
   }
   for (i = 0; i < flam3_nxforms; i++) {
      t = 0.0;
      for (j = 0; j < flam3_nvariations; j++)
	 t += cp->xform[i].var[j];
      t = 1.0 / t;
      for (j = 0; j < flam3_nvariations; j++)
	 cp->xform[i].var[j] *= t;
   }
}

int
main(int argc, char **argv)
{
  char *s, *ss;

#ifdef WIN32
   
  char *slashloc;
  char palpath[256],exepath[256];

    slashloc = strrchr(argv[0],'\\');
	if (NULL==slashloc) {
	   sprintf(palpath,"flam3_palettes=flam3-palettes.xml");
	} else {
       strncpy(exepath,argv[0],slashloc-argv[0]+1);
	   sprintf(palpath,"flam3_palettes=%sflam3-palettes.xml",exepath);
	}
	putenv(palpath);

#endif           
  
   if (1 != argc) {
     docstring();
     exit(0);
   }

  {
    int i, c, slen = 5000;
    s = malloc(slen);
    i = 0;
    do {
      c = getchar();
      if (EOF == c) goto done_reading;
      s[i++] = c;
      if (i == slen-1) {
	slen *= 2;
	s = realloc(s, slen);
      }
    } while (1);
  done_reading:
    s[i] = 0;
  }

  ss = s;
  s += strlen(s);
  printf("<conversions>\n");
  while (strchr(ss, ';')) {
    flam3_genome cp;
    parse_control_point_old(&ss, &cp);
    flam3_print(stdout, &cp, NULL, flam3_print_edits);
  }
  printf("</conversions>\n");
  return 0;
}
