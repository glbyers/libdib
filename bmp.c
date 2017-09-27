/*
    This file is part of LibDIB.
    Copyright (C) 2003 Grant Byers.
	
    Version: 0.5
    Last modified: 30th June, 2003
    Author: Grant Byers

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

#ifdef __cplusplus
extern "C" {
#endif

#include "bmp_internal.h"

/*
 * internal housekeeping used to manage the dynamic allocation
 * and deallocation of bitmap_s structures.
 */
static int ref_count;
static struct ref_s *ref_head;


/*
 * unrecoverable error. best to abort here.
 */
static void
fatal(const char *msg)
{
	fprintf(stderr,"Fatal error occured: %s",msg);
	if (errno) {
		perror(NULL);
	}	
	abort();
}


/*
 * provide a warning message as to why some call failed.
 */
static void
warn(const char *msg, const char *fname)
{
	fprintf(stderr,"Warning: %s: %s\n",msg,fname);
	if (errno) {
		perror(NULL);
	}	
	errno = 0;
}


/*
 * this should never be called without first checking that the
 * reference actually exists. quite simply, this can be done by
 * a single call to ref_exists.
 */
static void 
ref_free(struct ref_s *ref)
{
	free(ref->bmp->img);
	free(ref->bmp->palette);
	free(ref->bmp);
	free(ref);
}


/*
 * check if this bitmap_s pointer is still referenced. we don't
 * want to use or free a pointer that hasn't yet been initialised, 
 * or has previously been deallocated.
 */
static struct ref_s * 
ref_exists(bitmap_s *bmp)
{
	struct ref_s *cur = ref_head;

	while (cur && cur->bmp != bmp) {
			cur = cur->next;
	}

	return cur;
}


/*
 * a new reference is allocated, added to the front of the list, 
 * then ref_head updated to point to this new reference.
 */
static void
ref_add(bitmap_s *bmp)
{
	struct ref_s *ref;
	
	if (!(ref = malloc(sizeof *ref))) {
		fatal("memory exhausted");
	}	
	ref->bmp  = bmp;
	ref->next = ref_head;
	ref_head  = ref;
	ref_count++;
}


/* 
 * locate the reference that contains this bitmap_s pointer and
 * deallocate it.
 */
static void
ref_del(struct ref_s *ref)
{
	struct ref_s *cur = ref_head;

	if (ref == ref_head) {
		ref_head = ref_head->next;
	} else {
		while (cur->next != ref) {
			cur = cur->next;
		}
	}	
	cur->next = ref->next;
	ref_free(ref);
	ref_count--;
}


/*
 * allocate memory for a new bitmap, initialise it's 
 * fields and set the image name. returns NULL on error.
 */
static void *
init(const char *name)
{
	bitmap_s *bmp;
	
	if ((bmp = malloc(sizeof *bmp))) {
		strncpy(bmp->name,name,PATH_MAX);	
		bmp->name[PATH_MAX-1] = '\0';	
		bmp->img = NULL;
		bmp->palette = NULL;
		ref_add(bmp);
	}
	return bmp;
}
	

/*
 * validate the signature field of the bitmap file header.
 */
static int
valid_sig(char *sig)
{
	return (strncmp(sig,"BM",2) == 0);
}


/*
 * read and validate the bitmaps file header.
 * returns boolean equivalent to true on success, 0 on failure.
 */
static int 
get_fh(bitmap_s *bmp, FILE *f)
{
	int x = 1;
	size_t n = 0;
	
	n  = fread(bmp->fh.signature  ,1,2,f);
	n += fread(&bmp->fh.file_size ,4,1,f); 
	n += fread(&bmp->fh.reserved  ,4,1,f);
	n += fread(&bmp->fh.dib_offset,4,1,f);

	/* is this a big endian machine? */
	if (*(char *)&x != 1) {
		BSWAP_32(bmp->fh.file_size);
		BSWAP_32(bmp->fh.reserved);
		BSWAP_32(bmp->fh.dib_offset);
	}

	return (n == 5) & (valid_sig(bmp->fh.signature)); 
}


/*
 * read and validate the bitmaps info header.
 */
static int 
get_ih(bitmap_s *bmp, FILE *f)
{
	int x = 1;
	size_t n = 0;
	info_hdr_s *ih = &bmp->ih;

#if 0	
	/* ih offset should be 14 bytes from start of file */
	if (fseek(f,14,SEEK_SET) != -1) {
		t  = fread(&ih->ih_size      ,4,1,f);
		t += fread(&ih->w            ,4,1,f);
		t += fread(&ih->h            ,4,1,f);
		t += fread(&ih->planes       ,2,1,f);
		t += fread(&ih->bpp          ,2,1,f);
		t += fread(&ih->compress     ,4,1,f);
		t += fread(&ih->img_size     ,4,1,f);
		t += fread(&ih->hres         ,4,1,f);
		t += fread(&ih->vres         ,4,1,f);
		t += fread(&ih->num_colors   ,4,1,f);
		t += fread(&ih->num_important,4,1,f);
	}
#endif
	
	if (fseek(f,14,SEEK_SET) != -1) {
		n = fread(ih,40,1,f);
	}

	/* is this a big endian machine? */
	if (*(char *)&x != 1) {
		BSWAP_32(ih->ih_size);
		BSWAP_32(ih->w);
		BSWAP_32(ih->h);
		BSWAP_16(ih->planes);
		BSWAP_16(ih->bpp);
		BSWAP_32(ih->compress);
		BSWAP_32(ih->img_size);
		BSWAP_32(ih->hres);
		BSWAP_32(ih->vres);
		BSWAP_32(ih->num_colors);
		BSWAP_32(ih->num_important);
	}

	return (n == 1);
}


/* 
 * read and validate the bitmaps raster image.
 * scanlines are positioned in memory upside down. each scanline is
 * equivalent to the image width plus padding to a double word boundary.
 *
 * addr | line | width | bpp
 * ----------------------------
 * 0    | 5    | 128   | 3
 * 384  | 4    | 128   | 3
 * 768  | 3    | 128   | 3
 * 1152 | 2    | 128   | 3
 * 1536 | 1    | 128   | 3
 * 1920 | 0    | 128   | 3
 *    
 */
static int
get_dib(bitmap_s *bmp, FILE *f)
{
	/* we want bytes per pixel, not bits per pixel here */
	size_t sz = bmp->ih.img_size;
	unsigned char *img = malloc(sz); 
	int w = sz / bmp->ih.h;

	if (img) {
		while (sz) {
			/* start at last scanline */
			if (fseek(f,bmp->fh.dib_offset + sz-w,SEEK_SET)!= -1) {
				sz -= fread(img + bmp->ih.img_size - sz,1,w,f);
			} else {
				break;
			}	
		}
	}
	bmp->img = img;

	return (sz == 0);
}


/*---------------- BEGIN PUBLIC INTERFACE ----------------
 *
 * these functions are well documented in the header file.
 * please refer to bmp.h for verbose details.
 *
 */


/* return pointer to image, NULL on error. */
void *
bmp_get_img(bitmap bmp)
{
	if (ref_exists(bmp)) {
		return bmp->img;
	}
	return NULL;
}	


/* returns an initialised and validated bitmap_s struct, NULL on error.  */
bitmap 
bmp_load(const char *fname)
{
	FILE *f;
	int error = 1;
	bitmap_s *bmp = NULL;
		
	if (!(f = fopen(fname,"rb"))) {
		warn("failed to open",fname);
	} else if (!(bmp = init(fname))) {
		fatal("Memory exhausted");
	} else if (!get_fh(bmp,f)) {
		warn("invalid image file",fname);
	} else if (!get_ih(bmp,f)) {
		warn("image header corrupt",fname);
	} else if (!get_dib(bmp,f)) {
		warn("image data corrupt",fname);
	} else {	
		fclose(f);
		error = 0;
	}	
	
	if (error) {
		bmp = bmp_destroy(bmp);
	}	
	
	return (bitmap)bmp;
}


/* return image width from valid bitmap_s struct. */
int
bmp_get_width(bitmap bmp)
{
	return bmp->ih.w;
}


/* return image height from valid bitmap_s struct. */
int
bmp_get_height(bitmap bmp)
{
	return bmp->ih.h;
}


/* returns a new bitmap_s stuct containing a 32 bit image, converted from 
   given bitmap_s struct containing 24 bit image. name is the new filename
   for the image. */
bitmap
bmp_convert24to32(bitmap bmp24, char *name)
{
	bitmap_s *bmp32;
	int pad, i, j;

	if (!bmp24) {
		return NULL;
	}
	
	if (!(bmp32 = init(name))) {
		fatal("memory exhausted");
	}	
	
	bmp32->fh = bmp24->fh;
	bmp32->ih = bmp24->ih;
	bmp32->ih.bpp = 32;

	/* each scanline of the image must end on a double word boundary. */
	pad = bmp24->ih.w % sizeof(int);
	bmp32->ih.img_size = ((bmp24->ih.w + pad) * bmp24->ih.h << 2);

	/* calculate total file size for new bitmap. */
	bmp32->fh.file_size = bmp32->fh.dib_offset + bmp32->ih.img_size;
	
	if (!(bmp32->img = malloc(bmp32->ih.img_size))) {
		fatal("memory exhausted");
	}		

	for (i = 0, j = 0; i < bmp24->ih.w * bmp24->ih.h; i++, j += 3) {
		((unsigned int *)bmp32->img)[i] =
			(((unsigned int)bmp24->img[j]<<0)
			|((unsigned int)bmp24->img[j+1]<<8)
			|((unsigned int)bmp24->img[j+2]<<16))
			&(0x00FFFFFF);

			/* any padding bytes should be added to the end of
			   each of the new scanlines. */
			if (i % bmp24->ih.w == 0)
				j += pad;
	}	

	return (bitmap)bmp32;
}


/* returns a new bitmap_s stuct containing a 16 bit image, converted from 
   given bitmap_s struct containing 24 bit image. name is the new filename
   for the image. */
bitmap 
bmp_convert24to16(bitmap bmp24, char *name)
{
	bitmap_s *bmp16;
	int i, j, pad;

	if (!bmp24) {
		return NULL;
	}

	if (!(bmp16 = init(name))) {
		fatal("memory exhausted");
	}	

	bmp16->fh = bmp24->fh;
	bmp16->ih = bmp24->ih;
	bmp16->ih.bpp = 16;
		
	/* each scanline of the image must end on a double word boundary. */
        pad = bmp24->ih.w % sizeof(int);
	bmp16->ih.img_size = ((bmp24->ih.w + pad) * bmp24->ih.h << 1);

	/* calculate total file size for new bitmap */
	bmp16->fh.file_size = bmp16->fh.dib_offset + bmp16->ih.img_size;

	if (!(bmp16->img = malloc(bmp16->ih.img_size))) {
		fatal("memory exhausted");
	}	
	
        for (i = 0, j = 0; i < bmp24->ih.w * bmp24->ih.h; i++, j += 3) {
                ((unsigned short *)bmp16->img)[i] = (bmp24->img[j]&0xF8)>>3
                        |(bmp24->img[j+1]&0xFC)<<3
                        |(bmp24->img[j+2]&0xF8)<<8;

			/* any padding bytes should be added to the end of
			   each of the new scanlines. */
                        if (i % bmp24->ih.w == 0) {
                                j += pad;
			}	
        }

        return (bitmap)bmp16;
}


/* cleanup memory allocated previously to the given bitmap_s struct. */
void *
bmp_destroy(bitmap bmp)
{	
	struct ref_s *ref;
	
	if (bmp) {
		if ((ref = ref_exists(bmp))) {
			ref_del(ref);
		}
	}	
	
	return NULL;
}


/* cleanup memory allocated to all previously allocated bitmap_s structs. */ 
void
bmp_gc(void)
{
	while (ref_count) {
		ref_del(ref_head);
	}	
}


/* write a bitmap_s struct out to file. the filename is taken from the
   bitmap_s structs name field. */
void
bmp_write(bitmap bmp) 
{
	int x = 1;
	FILE *f;

	if (!bmp) {
		return;
	}

	if (!(f = fopen(bmp->name,"wb"))) {
		warn("Failed to open file",bmp->name);
		return;
	}	
	
	/* is this a big endian machine? */
	if (*(char *)&x != 1) {
		/* swap byte order back to little endian */
		BSWAP_32(bmp->fh.file_size);
		BSWAP_32(bmp->fh.reserved);
		BSWAP_32(bmp->fh.dib_offset);
		BSWAP_32(bmp->ih.ih_size);
		BSWAP_32(bmp->ih.w);
		BSWAP_32(bmp->ih.h);
		BSWAP_16(bmp->ih.planes);
		BSWAP_16(bmp->ih.bpp);
		BSWAP_32(bmp->ih.compress);
		BSWAP_32(bmp->ih.img_size);
		BSWAP_32(bmp->ih.hres);
		BSWAP_32(bmp->ih.vres);
		BSWAP_32(bmp->ih.num_colors);
		BSWAP_32(bmp->ih.num_important);
	}

	/* file header */
	fwrite(bmp->fh.signature  ,1,2,f);
	fwrite(&bmp->fh.file_size ,4,1,f); 
	fwrite(&bmp->fh.reserved  ,4,1,f);
	fwrite(&bmp->fh.dib_offset,4,1,f);

	/* info header */
	fwrite(&bmp->ih.ih_size      ,4,1,f);
	fwrite(&bmp->ih.w            ,4,1,f);
	fwrite(&bmp->ih.h            ,4,1,f);
	fwrite(&bmp->ih.planes       ,2,1,f);
	fwrite(&bmp->ih.bpp          ,2,1,f);
	fwrite(&bmp->ih.compress     ,4,1,f);
	fwrite(&bmp->ih.img_size     ,4,1,f);
	fwrite(&bmp->ih.hres         ,4,1,f);
	fwrite(&bmp->ih.vres         ,4,1,f);
	fwrite(&bmp->ih.num_colors   ,4,1,f);
	fwrite(&bmp->ih.num_important,4,1,f);

	/* image */
	fseek(f,bmp->fh.dib_offset,SEEK_SET);
	fwrite(bmp->img,bmp->ih.img_size,1,f);

	fflush(f);
	fclose(f);
}


/* display information about the structure of a bitmap on stdout. */
void
bmp_info(bitmap bmp)
{
	if (!bmp) {
		return;
	}
	
	fprintf(stdout,"%-25s : %s\n","Image name",bmp->name);
	fprintf(stdout,"%-25s : %s\n","Signature",bmp->fh.signature);
	fprintf(stdout,"%-25s : %d\n","File size",bmp->fh.file_size);
	fprintf(stdout,"%-25s : %d\n","Info header size",bmp->ih.ih_size); 
	fprintf(stdout,"%-25s : %d\n","Width",bmp->ih.w);
	fprintf(stdout,"%-25s : %d\n","Height",bmp->ih.h);
	fprintf(stdout,"%-25s : %d\n","Number of planes",bmp->ih.planes);
	fprintf(stdout,"%-25s : %d\n","Bits Per Pixel",bmp->ih.bpp);
	fprintf(stdout,"%-25s : %d\n","Image size (bytes)",bmp->ih.img_size);
	fprintf(stdout,"%-25s : %d\n","Horizontal resolution",bmp->ih.hres);
	fprintf(stdout,"%-25s : %d\n","Vertical resolution",bmp->ih.vres);
	fprintf(stdout,"%-25s : %d\n","Number of colors",bmp->ih.num_colors);
	fprintf(stdout,"%-25s : %d\n","Number important colors",
		bmp->ih.num_important);
}		

#ifdef __cplusplus
}
#endif
