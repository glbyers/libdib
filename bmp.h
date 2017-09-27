/*
    This file is part of LibDIB.
    Copyright (C) 2003 Grant Byers.

    Version: 0.5
    Last modified: 30th June, 2003
    Author: Grant Byers <glbyers@students.latrobe.edu.au>

    Aims to be a small, easy to read library for reading Microsoft
    Windows Device Independant Bitmaps [DIB], known to most as Bitmap
    images, or just BMP.

    LibDIB is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __BMP_H
#define __BMP_H

/* required for PATH_MAX */
/*#include <limits.h> */
#define PATH_MAX	256


/** file header structure. */
typedef struct {
	char signature[2];		/* bitmap signature */
	unsigned int file_size;		/* file size */
	unsigned int reserved;		/* not used */
	unsigned int dib_offset;	/* offset of bitmap data */
} file_hdr_s;

/** info header structure. */
typedef struct {
	unsigned int ih_size;		/* info header size */
	unsigned int w;			/* width of image */
	unsigned int h;			/* height of image */
	unsigned short planes;		/* number of planes in bitmap */
	unsigned short bpp;		/* bits per pixel */
	unsigned int compress;		/* compression type */
	unsigned int img_size;		/* size of image in bytes */
	unsigned int hres;		/* horiz resolution, pixels / metre */
	unsigned int vres;		/* vert resolution, pixels / metre */
	unsigned int num_colors;	/* number of colours in bitmap */
	unsigned int num_important;	/* number of important colours */
} info_hdr_s;

/** actual bitmap structure. */
typedef struct {
	char name[PATH_MAX]; 	/* name of image file */
	file_hdr_s fh;		/* file header */
	info_hdr_s ih;		/* image header */
	unsigned char *palette;	/* palette for <= 8 bit images */
	unsigned char *img;	/* DIB raster image */
} bitmap_s;

/* shelter users from misuse. */
typedef bitmap_s *bitmap;


/*!
 *  WARNING - any call to the functions provided may result in exhausting
 *  physical resources, such as memory or storage. On the event that this
 *  occurs, and control is within our process, the program will attempt to
 *  abort with a 'Fatal error occured: ...' message on stderr.
 */


/*!
 *  bmp_load reads a representation of a bitmap into memory.
 *  fname specifies the filename to be loaded. The filename
 *  may be relative to the Current Working Directory (cwd) or be absolute.
 *
 *  if successful, a fully allocated bitmap will be returned, ready for use. 
 *  If the call fails, due to a corrupted .bmp file, file permissions, an 
 *  invalid path or other constraints, a message stating why the call failed 
 *  will be sent to stderr and a null pointer will be returned. A further 
 *  descriptive message may also be available immediately through the
 *  ferror call - (man (3) ferror).
 */
extern bitmap bmp_load(const char *fname);		

/*!
 *  bmp_get_img returns a pointer to an area of memory containing the actual
 *  image. 
 *  bmp is a a non-null, validated bitmap. If either of these conditions are 
 *  false, a null pointer will be returned and a 'Warning: ...' message will be
 *  sent to stderr.
 */
extern void * bmp_get_img(bitmap bmp);

/*!
 *  bmp_get_width returns the width of the image represented by the bitmap bmp.
 *  bmp must be a non-null, validated bitmap. If either of these conditions are
 *  false, behaviour is undefined.
 */
extern int bmp_get_width(bitmap bmp);

/*!
 *  bmp_get_height returns the height of the image represented by the bitmap
 *  bmp.
 *  bmp must be a non-null, validated bitmap. If either of these conditions are
 *  false, behaviour is undefined.
 */
extern int bmp_get_height(bitmap bmp);

/*!
 *  bmp_destroy should be used to free any memory allocated to the bitmap bmp. 
 *  null is gauranteed to be returned, which should be assigned back to the 
 *  bitmap to protect against future missuse. if bmp is null or does not refer 
 *  to a previously allocated bitmap, the fault is silently ignored so no harm 
 *  can be done to the current operating state.
 */
extern void * bmp_destroy(bitmap bmp);

/*!
 *  bmp_gc will deallocate all memory used by any previous bitmap allocated,
 *  which has not been previously free'd through a call to bmp_destroy. 
 *  A good idea would be to pass this function as a parameter to the atexit
 *  call to free any memory used by any previously allocated bitmap.
 *  (man (3) atexit).
 * 
 *  eg.  atexit(bmp_gc);
 */
extern void bmp_gc(void);

/*!
 *  bmp_info will cause an informative message about the structure of a given 
 *  bitmap to be displayed on stdout. passing a null bitmap will result in no
 *  message being displayed. Behaviour for an invalid or corrupted bitmap is
 *  undefined.
 */
extern void bmp_info(bitmap bmp);

/*!
 *  bmp_write will write a bitmap out to file, who's name is specified in the
 *  bitmap header itself, as bitmap->name. Passing a null bitmap will result in
 *  a return, with no file written. Attempting to write a bitmap with an invalid
 *  name field will also result in a return, with nothing written. An error will
 *  be sent to stdout.
 */
extern void bmp_write(bitmap bmp);

/*!
 *  bmp_convert24to32 converts a 24 bit bitmap to 32 bits, updating the header
 *  to reflect these changes. name is the name to call the new bitmap, in case
 *  the user wants to later write this bitmap out to file. bmp must be a fully
 *  initialised and validated bitmap. A null bitmap will result in the call
 *  returning NULL. Calling with an invalid or corrupted bitmap will result in
 *  undefined behaviour. On success, a new 32 bit bitmap will be returned.
 */
extern bitmap bmp_convert24to32(bitmap bmp, char *name);

/*!
 *  bmp_convert24to16 converts a 24 bit bitmap to 16 bits, updating the header
 *  to reflect these changes. name is the name to call the new bitmap, in case
 *  the user wants to later write this bitmap out to file. bmp must be a fully
 *  initialised and validated bitmap. A null bitmap will result in the call
 *  returning NULL. Calling with an invalid or corrupted bitmap will result in
 *  undefined behaviour. On success, a new 16 bit bitmap will be returned.
 */
extern bitmap bmp_convert24to16(bitmap bmp, char *name);

#endif /* __BMP_H */	
