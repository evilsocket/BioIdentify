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
#include "biofloatmap.h"

bio_floatmap_t * bio_floatmap_create(){
	return (bio_floatmap_t *)calloc( 1, sizeof(bio_floatmap_t) );
}

void bio_floatmap_destroy( bio_floatmap_t *fmap ){
	if(fmap){
		if(fmap->map){
			free(fmap->map);	
		}
		free(fmap);	
	}	
}

bio_error_t bio_floatmap_setsize( bio_floatmap_t *fmap, int width, int height ){
	int newsize = width * height *sizeof(double);
	
	if( newsize == 0 ){
        if( fmap->map != NULL ){
            free(fmap->map);
            fmap->map    = NULL;
            fmap->width  = 0;
            fmap->height = 0;
            fmap->pitch  = 0;
        }
        return BIOERR_SUCCESS;
    }
    else if( ( fmap->height * fmap->width * sizeof(double) ) != newsize ){
        if( fmap->map != NULL ){
        	free(fmap->map);
        	fmap->map = NULL;
		}
		fmap->width  = width;
		fmap->height = height;
		fmap->pitch  = width;
        fmap->map    = (double *)calloc( 1, newsize );
    }
    
    return (fmap->map == NULL ? BIOERR_MEMORY : BIOERR_SUCCESS);
}	

bio_error_t bio_floatmap_copy( bio_floatmap_t *dst, bio_floatmap_t *src ){
    if( bio_floatmap_setsize( dst, src->width, src->height ) == BIOERR_SUCCESS ){
    	memcpy( dst->map, src->map, src->height * src->width * sizeof(double) );
    	return BIOERR_SUCCESS;
    }
    else{
    	return BIOERR_MEMORY;
    }
}

void bio_floatmap_fill( bio_floatmap_t *fmap, double value ){
	if( fmap->map != NULL ){
		memset( fmap->map, value, fmap->width * fmap->height * sizeof(double) );	
	}
}

void bio_floatmap_clear( bio_floatmap_t *fmap ){
	bio_floatmap_fill( fmap, 0.0 );
}
