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
#include "biofinger.h"

static bio_error_t bio_finger_lowpass_filter( double* theta, double* out, int filtersize, int width, int height ){
    double *filter = NULL,
    	   *phix   = NULL,
    	   *phiy   = NULL,
    	   *phi2x  = NULL,
           *phi2y  = NULL,
           nx, ny;
    int fsize = filtersize * 2 + 1,
        bsize = width * height * sizeof(double),
        offset, i, j, x, y;

    filter = (double *)malloc( fsize * fsize *sizeof(double) );
    phix   = (double *)malloc( bsize );
    phiy   = (double *)malloc( bsize );
    phi2x  = (double *)malloc( bsize );
    phi2y  = (double *)malloc( bsize );

	memset( filter, 0, fsize * fsize *sizeof(double) );
	memset( phix,   0, bsize );
	memset( phiy,   0, bsize );
	memset( phi2x,  0, bsize );
	memset( phi2y,  0, bsize );

	for( y = 0; y < height; y++ ){
		for( x = 0; x < width; x++ ){
			offset       = x + y * width;
			phix[offset] = cos(theta[offset]);
			phiy[offset] = sin(theta[offset]);
		}
	}
	
	nx = 0.0;
	for( j = 0; j < fsize; j++ ){
		for( i = 0; i < fsize; i++ ){
			offset         = j * fsize + i;
			filter[offset] = 1.0;
			nx            += filter[offset];
		}
	}
	if( nx > 1.0 ){
		for( j = 0; j < fsize; j++ ){
			for( i = 0; i < fsize; i++ ){
				filter[ j * fsize + i ] /= nx;
			}
		}		
	}

	for( y = 0; y < height - fsize; y++ ){
		for( x = 0; x < width - fsize; x++ ){
			nx = 0.0;
			ny = 0.0;
			for( j = 0; j < fsize; j++ ){
				for( i = 0; i < fsize; i++ ){
					offset = (x + i) + (j + y) * width;
					nx    += filter[j * fsize + i] * phix[offset];
					ny    += filter[j * fsize + i] * phiy[offset];
				}
			}	
			offset        = x + y * width;
			phi2x[offset] = nx;
			phi2y[offset] = ny;
		}
	}
	
	free(phix);
	free(phiy);
	
	for( y = 0; y < height - fsize; y++ ){
		for( x = 0; x < width - fsize; x++ ){
			offset      = x + y * width;
			out[offset] = atan2( phi2y[offset], phi2x[offset] ) * 0.5;
		}
	}
	
	free(phi2x);
    free(phi2y);
    free(filter);

    return BIOERR_SUCCESS;
}

bio_error_t bio_finger_direction( bio_image_t *image, bio_floatmap_t *direction, int blocksize, int filtersize ){
    int i, j, u, v, x, y, xi, yi,
        s = blocksize * 2 + 1,
        offset;
    double dx[s*s],
    	   dy[s*s],
    	   nx, ny,
    	   *theta = NULL;
   	byte_t bi;
   
	if( bio_floatmap_setsize( direction, image->width, image->height ) != BIOERR_SUCCESS ){
		bio_set_lasterror( __FILE__, __LINE__, "Could not set size %d for direction map .", image->width*image->height );
		return BIOERR_MEMORY;	
	}
	bio_floatmap_clear(direction);
 
    if( filtersize > 0 ){
        theta = (double *)calloc( 1, image->width * image->height * sizeof(double));
    }

	for( y = blocksize + 1; y < image->height - blocksize - 1; y++ ){
		for( x = blocksize + 1; x < image->width - blocksize - 1; x++){
			for( j = 0; j < s; j++ ){
				yi = y + j - blocksize;
				for( i = 0; i < s; i++ ){
					offset     = i * s + j;
					xi         = x + i - blocksize;
					bi         = image->data[xi + yi * image->width];
					dx[offset] = (double)(bi - image->data[(xi - 1) + yi * image->width]);
					dy[offset] = (double)(bi - image->data[xi + (yi - 1) * image->width]);
				}
			}
			
			nx = 0.0;
			ny = 0.0;
			for( v = 0; v < s; v++ ){
				for( u = 0; u < s; u++ ){
					offset = u * s + v;
					nx    += 2 * dx[offset] * dy[offset];
					ny    += dx[offset] * dx[offset] - dy[offset] * dy[offset];
				}
			}
			
			if( filtersize > 0 ){
				theta[x + y * image->width] = atan2(nx,ny);
			}
			else{
				direction->map[x + y * image->width] = atan2(nx,ny) * 0.5;
			}
		}
	}
	
	if( filtersize > 0 ){
		bio_finger_lowpass_filter( theta, direction->map, filtersize, image->width, image->height );
    }

    free(theta);
    
    return BIOERR_SUCCESS;
}

bio_error_t bio_finger_frequency( bio_image_t *image, bio_floatmap_t *direction, bio_floatmap_t *frequency ){
    double *out,
    	   dir    = 0.0,
    	   cosdir = 0.0,
    	   sindir = 0.0,
    	   peak_freq,
    	   Xsig[BIO_BLOCK_L],
    	   pmin, pmax;
    int x, y, u, v, d, k, size,
    	peak_pos[BIO_BLOCK_L],
    	peak_cnt;
    
    if( bio_floatmap_setsize( frequency, image->width, image->height ) != BIOERR_SUCCESS ){
    	bio_set_lasterror( __FILE__, __LINE__, "Could not set size %d for frequency map .", image->width*image->height );
		return BIOERR_MEMORY;	
	}
	bio_floatmap_clear(frequency);

    size = image->width * image->height * sizeof(double);
    out  = (double *)calloc( 1, size );

	for( y = BIO_BLOCK_L2; y < image->height - BIO_BLOCK_L2; y++ ){
		for( x = BIO_BLOCK_L2; x < image->width - BIO_BLOCK_L2; x++ ){
			dir    = direction->map[ (x + BIO_BLOCK_W2) + (y + BIO_BLOCK_W2) * image->width ];
			cosdir = -sin(dir);
			sindir = cos(dir);

			for( k = 0; k < BIO_BLOCK_L; k++ ){
				Xsig[k] = 0.0;
				for( d = 0; d < BIO_BLOCK_W; d++ ){
					u = (int)(x + (d - BIO_BLOCK_W2) * cosdir + (k - BIO_BLOCK_L2) * sindir );
					v = (int)(y + (d - BIO_BLOCK_W2) * sindir - (k - BIO_BLOCK_L2) * cosdir );

					if( u < 0 ){
						u = 0;
					}
					else if( u > image->width - 1 ){ 
						u = image->width - 1;
					}
					if( v < 0 ){ 
						v = 0; 
					}
					else if( v > image->height - 1 ){ 
						v = image->height - 1;
					}
					Xsig[k] += image->data[ u + v * image->width ];
				}
				Xsig[k] /= BIO_BLOCK_W;
			}
			peak_cnt = 0;
			pmax     = pmin = Xsig[0];
			for( k = 1; k < BIO_BLOCK_L; k++ ){
				pmin = (pmin > Xsig[k]) ? Xsig[k] : pmin;
				pmax = (pmax < Xsig[k]) ? Xsig[k] : pmax;
			}
			if( (pmax - pmin) > 64.0 ){
				for( k = 1; k < BIO_BLOCK_L - 1; k++ ){
					if( (Xsig[k - 1] < Xsig[k]) && (Xsig[k] >= Xsig[k + 1]) ){
						peak_pos[peak_cnt++] = k;
					}
				}
			}
			peak_freq = 0.0;
			if( peak_cnt >= 2 ){
				for( k = 0; k < peak_cnt - 1; k++ ){
					peak_freq += peak_pos[k + 1] - peak_pos[k];
				}
				peak_freq /= peak_cnt - 1;
			}

			if( peak_freq > 30.0 ){
				out[x + y * image->width] = 0.0;
			}
			else if( peak_freq < 2.0 ){
				out[x + y * image->width] = 0.0;
			}
			else{
				out[x + y * image->width] = 1.0 / peak_freq;
			}
		}
	}
	
	for( y = BIO_BLOCK_L2; y < image->height - BIO_BLOCK_L2; y++ ){
		for( x = BIO_BLOCK_L2; x < image->width - BIO_BLOCK_L2; x++ ){
			if( out[x + y * image->width] < BIO_EPSILON ){
				if( out[x + (y-1) * image->width] > BIO_EPSILON ){
					out[x + (y * image->width)] = out[x + (y-1) * image->width];
				}
				else{
					if( out[x - 1 + (y * image->width)] > BIO_EPSILON ){
						out[ x + (y * image->width)] = out[x - 1 + (y * image->width)];
					}
				}
			}
		}
	}
	
	for( y = BIO_BLOCK_L2; y < image->height - BIO_BLOCK_L2; y++ ){
		for( x = BIO_BLOCK_L2; x < image->width - BIO_BLOCK_L2; x++ ){
			k         = x + y * image->width;
			peak_freq = 0.0;
			for( v = -BIO_LPSIZE; v <= BIO_LPSIZE; v++ ){
				for( u = -BIO_LPSIZE; u <= BIO_LPSIZE; u++ ){
					peak_freq += out[(x + u) + (y + v) * image->width];
				}
			}
			frequency->map[k] = peak_freq * BIO_LPFACTOR;
		}
	}
	free(out);
    
    return BIOERR_SUCCESS;	
}

bio_error_t bio_finger_mask( bio_image_t *image, bio_floatmap_t *direction, bio_floatmap_t *frequency, bio_mask_t *mask ){
    int offset, x, y;

	mask->mask   = (byte_t *)calloc( 1, image->width * image->height );
	mask->width  = image->width;
	mask->height = image->height;

    for( y = 0; y < image->height; y++ ){
        for( x = 0; x < image->width; x++ ){
            offset = x + y * image->width;
            if( frequency->map[offset] >= BIO_FREQ_MIN && frequency->map[offset] <= BIO_FREQ_MAX ){
            	mask->mask[offset] = 0xFF;
            }
        }
    }
    
    for( y = 0; y < 4; y++ ){
        bio_mask_dilate(mask);
    }
    for( y = 0; y < 12; y++ ){
        bio_mask_erode(mask);
    }
    
    return BIOERR_SUCCESS;
}

bio_error_t bio_finger_thin( bio_image_t *image, bio_mask_t *mask ){
    int x, y, c, 
    	xp, xm, 
    	yp, ym, 
    	offset,
    	changed = 1;
	
    while(changed){
        changed = 0;
        for( y = 1; y < image->height - 1; y++ ){
        	yp = y + 1;
        	ym = y - 1;
			for( x = 1; x < image->width - 1; x++ ){
				for( c = 0; c < 3; c++ ){
					if( BIO_PIX(x,y,c) ){
						xp = x + 1;
						xm = x - 1;
						if( BIO_PIX(xm,ym,c) == 0x00 && BIO_PIX(x,ym,c) == 0x00 && BIO_PIX(xp,ym,c) == 0x00 &&
							BIO_PIX(xm,yp,c) != 0x00 && BIO_PIX(x,yp,c) != 0x00 && BIO_PIX(xp,yp,c) != 0x00 ){
							BIO_PIX( x, y, 0 ) = 0x00;
							BIO_PIX( x, y, 1 ) = 0x00;
							BIO_PIX( x, y, 2 ) = 0x00;
							changed = 1;
						}
						if( BIO_PIX(xm,ym,c) != 0x00 && BIO_PIX(x,ym,c) != 0x00 && BIO_PIX(xp,ym,c) != 0x00 &&
							BIO_PIX(xm,yp,c) == 0x00 && BIO_PIX(x,yp,c) == 0x00 && BIO_PIX(xp,yp,c) == 0x00 ){
							BIO_PIX( x, y, 0 ) = 0x00;
							BIO_PIX( x, y, 1 ) = 0x00;
							BIO_PIX( x, y, 2 ) = 0x00;
							changed = 1;
						}
						if( BIO_PIX(xm,ym,c) == 0x00 && BIO_PIX(xm,y,c) == 0x00 && BIO_PIX(xm,yp,c) == 0x00 &&
							BIO_PIX(xp,ym,c) != 0x00 && BIO_PIX(xp,y,c) != 0x00 && BIO_PIX(xp,yp,c) != 0x00 ){
							BIO_PIX( x, y, 0 ) = 0x00;
							BIO_PIX( x, y, 1 ) = 0x00;
							BIO_PIX( x, y, 2 ) = 0x00;
							changed = 1;
						}
						if( BIO_PIX(xm,ym,c) != 0x00 && BIO_PIX(xm,y,c) != 0x00 && BIO_PIX(xm,yp,c) != 0x00 &&
							BIO_PIX(xp,ym,c) == 0x00 && BIO_PIX(xp,y,c) == 0x00 && BIO_PIX(xp,yp,c) == 0x00 ){
							BIO_PIX( x, y, 0 ) = 0x00;
							BIO_PIX( x, y, 1 ) = 0x00;
							BIO_PIX( x, y, 2 ) = 0x00;
							changed = 1;
						}
             
						if( BIO_PIX(x,ym,c) == 0x00 && BIO_PIX(xp,ym,c) == 0x00 && BIO_PIX(xp,y,c) == 0x00 &&
							BIO_PIX(xm,y,c) != 0x00 && BIO_PIX(x,yp,c) != 0x00 ){
							BIO_PIX( x, y, 0 ) = 0x00;
							BIO_PIX( x, y, 1 ) = 0x00;
							BIO_PIX( x, y, 2 ) = 0x00;
							changed = 1;
						}

						if( BIO_PIX(xm,ym,c) == 0x00 && BIO_PIX(x,ym,c) == 0x00 && BIO_PIX(xm,y,c) == 0x00 &&
							BIO_PIX(xp,y,c) != 0x00 && BIO_PIX(x,yp,c) != 0x00 ){
							BIO_PIX( x, y, 0 ) = 0x00;
							BIO_PIX( x, y, 1 ) = 0x00;
							BIO_PIX( x, y, 2 ) = 0x00;
							changed = 1;
						}
						if( BIO_PIX(xm,yp,c) == 0x00 && BIO_PIX(xm,y,c) == 0x00 && BIO_PIX(x,yp,c) == 0x00 &&
							BIO_PIX(xp,y,c) != 0x00 && BIO_PIX(x,ym,c) != 0x00 ){
							BIO_PIX( x, y, 0 ) = 0x00;
							BIO_PIX( x, y, 1 ) = 0x00;
							BIO_PIX( x, y, 2 ) = 0x00;
							changed = 1;
						}

						if( BIO_PIX(xp,yp,c) == 0x00 && BIO_PIX(xp,y,c) == 0x00 && BIO_PIX(x,yp,c) == 0x00 &&
							BIO_PIX(xm,y,c) != 0x00 && BIO_PIX(x,ym,c) != 0x00 ){
							BIO_PIX( x, y, 0 ) = 0x00;
							BIO_PIX( x, y, 1 ) = 0x00;
							BIO_PIX( x, y, 2 ) = 0x00;
							changed = 1;
						}
					}
				}
			}
		}
    }
    
    for( y = 1; y < image->height - 1; y++ ){
		for( x = 1; x < image->width - 1; x++ ){
			offset = x + y * image->width;		
			if( mask->mask[offset] == 0x00 ){
				BIO_PIX( x, y, 0 ) = 0x00;
				BIO_PIX( x, y, 1 ) = 0x00;
				BIO_PIX( x, y, 2 ) = 0x00;
			}
		}
	}
   
    return bio_img_clean(image);	
}

inline int bio_feature_cmp( bio_feature_t *ma, bio_feature_t *mb ){
	if( ma == NULL || mb == NULL ){
		return 1;	
	}
	
	double tx  = 8.0, 
		   ty  = 8.0, 
		   ta  = 5.0, 
		   dx  = fabs( ma->x - mb->x ),
		   dy  = fabs( ma->y - mb->y ),
		   da  = fabs( ma->angle - mb->angle );
	
	return ( dx < tx && dy < ty && da < ta ) ? 0 : 1;
}

inline void bio_features_clean( bio_feature_set_t *features ){
	
	int i, j;	   
	for( i = 0; i < features->size(); i++ ){
		for( j = 0; j < features->size(); j++ ){
			if( i != j ){
				if( bio_feature_cmp( (*features)[i], (*features)[j] ) == 0 ){
					features->erase( features->begin() + j );
				}
			}
		}	
	}
}

inline int bio_mask_get_empty_neighbours( int ix, int iy, bio_mask_t *mask, int size ){
	int x, y, n = 0;
	
	for( x = ix - size; x < ix + size; x++ ){
		for( y = iy - size; y < iy + size; y++ ){
			if( mask->mask[x + y * mask->width] == 0x00 ){
				n++;	
			}
		}	
	}
	return n;
}

bio_error_t bio_finger_extract_features( bio_image_t *image, bio_floatmap_t *direction, bio_mask_t *mask, bio_feature_set_t *features ){
	int    x, y, neighbours;
    double angle = 0.0;
    
    features->clear();
    for( y = 1; y < image->height - 1; y++ ){
		for( x = 1; x < image->width - 1; x++ ){
			if( mask->mask[x + y * image->width] != 0x00 && BIO_PIX(x,y,0) == 0xFF ){
				/* Check if is on an edge of mask */
				if( bio_mask_get_empty_neighbours(x,y,mask,BIO_EDGE_THRESHOLD) == 0 ){
					neighbours = 0;
					if( BIO_PIX(x,y-1,0) != 0x00 ){ 
						neighbours++;
					}
					if( BIO_PIX(x+1,y-1,0) != 0x00 ){ 
						neighbours++;
					}
					if( BIO_PIX(x+1,y,0) != 0x00 ){ 
						neighbours++;
					}
					if( BIO_PIX(x+1,y+1,0) != 0x00 ){ 
						neighbours++;
					}
					if( BIO_PIX(x,y+1,0) != 0x00 ){ 
						neighbours++;
					}
					if( BIO_PIX(x-1,y+1,0) != 0x00 ){ 
						neighbours++;
					}
					if( BIO_PIX(x-1,y,0) != 0x00 ){ 
						neighbours++;
					}
					if( BIO_PIX(x-1,y-1,0) != 0x00 ){ 
						neighbours++;
					}
					
					angle = bio_floatmap_get( direction, x, y );
					switch(neighbours){
						/* spur */
						case 0 : break;
						/* line */
						case 2 : break;

						case 1 :
							features->push_back( new bio_feature_t( bio_ending, x, y, angle ) );
						break;

						case 3 : 
							features->push_back( new bio_feature_t( bio_delta, x, y, angle ) );
						break;
							
						default:
							features->push_back( new bio_feature_t( bio_branching, x, y, angle ) );
					}
				}
			}
		}
	}
	
	bio_features_clean(features);
    
    return BIOERR_SUCCESS;	
}

