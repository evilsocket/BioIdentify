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
#include "biohistogram.h"

bio_error_t bio_histogram_compute( bio_image_t *image, bio_histogram_t *histogram ){
    int x, y, offset;

	memset( histogram, 0x00, sizeof(bio_histogram_t) );
	
	histogram->mean     = -1;
	histogram->variance = -1;
	
	for( y = 0; y < image->height; y++ ){
		offset = image->width * y;
		for( x = 0; x < image->width; x++ ){
			histogram->table[ (*image)[offset++] ]++;
		}
	}
	
	histogram->size = image->width * image->height;

    return BIOERR_SUCCESS;	
}

byte_t bio_histogram_compute_mean( bio_histogram_t *histogram ){
    int i;

    if( histogram->mean == -1 ){
    	histogram->mean = 0;
        for( i = 1; i < 255; i++ ){
            histogram->mean += i * histogram->table[i];
		}
		
        histogram->mean /= histogram->size;
    }
    
    return (byte_t)histogram->mean;
}

uint_t bio_histogram_compute_variance( bio_histogram_t *histogram ){
    int i;
    byte_t mean;

    if( histogram->variance == -1 ){
    	histogram->variance = 0;
        mean                = bio_histogram_compute_mean(histogram);
        for( i = 0; i < 255; i++ ){
            histogram->variance += histogram->table[i] * (i - mean) * (i - mean);
		}
		
        histogram->variance /= histogram->size;
    }
    return (uint_t)histogram->variance;	
}
