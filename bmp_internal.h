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

#ifndef __BMP_INTERNAL_H
#define __BMP_INTERNAL_H

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "bmp.h"

/* this macro is used to convert byte order to big endian */
#define _BSWAP_32(x) \
        ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) | \
         (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))

#define BSWAP_32(x) \
	x = _BSWAP_32(x)

#define _BSWAP_16(x) \
         (((x) & 0xff00) >>  8) | (((x) & 0x00ff) << 8)

#define BSWAP_16(x) \
	x = _BSWAP_16(x)

extern int errno;

/* 
 * keep a reference of all bitmap structures allocated
 * so we can perform some garbage collection and protect
 * against misuse of NULL pointers.
 */
struct ref_s {
	struct ref_s *next;
	bitmap_s *bmp;
};

#endif	/* __BMP_INTERNAL_H */
