/***************************************************************************
 *   Copyright (C) 2008-2009 by evilsocket                                 *
 *                                                                         *
 *   evilsocket@gmail.com                                                  *
 *   http://www.evilsocket.net/ 										   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef __bio_image_h__
#	define __bio_image_h__

#include "biotypes.h"
#include "biofloatmap.h"
#include "biohistogram.h"
#include "bioerror.h"

bio_error_t bio_img_import( bio_image_t *image, const char *filename );
bio_error_t bio_img_set_import( bio_image_set_t *imageset, const char *directory );
bio_error_t bio_img_export( bio_image_t *image, const char *filename );
bio_error_t bio_img_soften_mean( bio_image_t *image, int size );
bio_error_t bio_img_normalize( bio_image_t *image, byte_t mean, uint_t variance );
bio_error_t bio_img_binarize( bio_image_t *image );
bio_error_t bio_img_clean( bio_image_t *image );
bio_error_t bio_img_gabor_enhance( bio_image_t *image, bio_floatmap_t *direction, bio_floatmap_t *frequency, bio_mask_t *mask, double radius ); 
bio_error_t bio_mask_dilate( bio_mask_t *mask );
bio_error_t bio_mask_erode( bio_mask_t *mask );

#define bio_img_at(i,x,y,c) i->ptr(x,y,c)

#define BIO_PIX(x,y,c) *image->ptr(x,y,c)

#define BIO_P1(img,x,y,c) *bio_img_at( img, x, y, c )
#define BIO_P2(img,x,y,c) *bio_img_at( img, x, y - 1, c )
#define BIO_P3(img,x,y,c) *bio_img_at( img, x + 1, y - 1, c )
#define BIO_P4(img,x,y,c) *bio_img_at( img, x + 1, y, c )
#define BIO_P5(img,x,y,c) *bio_img_at( img, x + 1, y + 1, c )
#define BIO_P6(img,x,y,c) *bio_img_at( img, x, y + 1, c )
#define BIO_P7(img,x,y,c) *bio_img_at( img, x - 1, y + 1, c )
#define BIO_P8(img,x,y,c) *bio_img_at( img, x - 1, y, c )
#define BIO_P9(img,x,y,c) *bio_img_at( img, x - 1, y - 1, c )

#endif

