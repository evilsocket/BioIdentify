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
#include "bioimage.h"

bio_error_t bio_img_import( bio_image_t *image, const char *filename ){
	if( image == NULL ){
		bio_set_lasterror( __FILE__, __LINE__, "Invalid input image pointer ." );
    	return BIOERR_PARAM;	
    }
    
    image->load(filename);
    
    return BIOERR_SUCCESS;	
}

bio_error_t bio_img_set_import( bio_image_set_t *imageset, const char *directory ){
	DIR *dp;
    struct dirent *dent;	
    char filename[0xFF] = {0};
    
    dp = opendir( directory );
    if( dp == NULL ){
    	bio_set_lasterror( __FILE__, __LINE__, "Could not open directory '%s' handle .", directory );
        return BIOERR_FAILURE;
    }

    while( (dent = readdir(dp)) != NULL ){
    	if( strcmp( dent->d_name, "." ) != 0 && strcmp( dent->d_name, ".." ) != 0 ){
    		bio_image_t *image = new bio_image_t;
    		sprintf( filename, "%s/%s", directory, dent->d_name );
    		bio_img_import( image, filename );
       		imageset->push_back( image );
    	}
    }

    closedir( dp );
    
    return BIOERR_SUCCESS;
}

bio_error_t bio_img_export( bio_image_t *image, const char *filename ){
	if( image == NULL ){
		bio_set_lasterror( __FILE__, __LINE__, "Invalid input image pointer ." );
    	return BIOERR_PARAM;	
    }
    
    image->save(filename);
    
    return BIOERR_SUCCESS;		
}

bio_error_t bio_img_soften_mean( bio_image_t *image, int size ){
	int x,y,z,s,p,q,a,c;
    bio_image_t tmp = *image;

    s = size / 2;
    a = size * size;
    
    for( y = s; y < image->height - s; y++ ){
		for( x = s; x < image->width - s; x++ ){
			for( z = 0; z < 3; z++ ){
				c = 0;
				for( q = -s; q <= s; q++ ){
					for( p = -s; p <= s; p++ ){
						c += *tmp.ptr( x+p, y+q, z );
					}
				}
				*image->ptr( x, y, z ) = c / a;
			}
		}
    }
    
    return BIOERR_SUCCESS;	
}

bio_error_t bio_img_normalize( bio_image_t *image, byte_t mean, uint_t variance ){
	int x,y,c;
    double fmean, 
    	   fmean0,
    	   fsigma, 
    	   fsigma0, 
    	   fgray,
           fcoeff = 0.0;
	bio_histogram_t histogram;

	bio_histogram_compute( image, &histogram );
	
	fmean0  = (double)mean;
	fmean   = (double)bio_histogram_compute_mean(&histogram);
	fsigma0 = sqrt((double)variance);
	fsigma  = sqrt((double)bio_histogram_compute_variance(&histogram));
	
	if( fsigma > 0.0 ){
		fcoeff = fsigma0 / fsigma;
	}
	
	for( y = 0; y < image->height; y++ ){
		for( x = 0; x < image->width; x++ ){
			for( c = 0; c < 3; c++ ){
				fgray = (double)BIO_PIX(x,y,c);
				fgray = fmean0 + fcoeff * (fgray - mean);
				
				if(fgray < 0.0){    
					fgray = 0.0;
				}
				else if( fgray > 255.0 ){  
					fgray = 255.0;
				}
				
				BIO_PIX(x,y,c) = (byte_t)fgray;
			}
		}
	}

    
    return BIOERR_SUCCESS;	
}

bio_error_t bio_img_binarize( bio_image_t *image ){
    int x, y;
    
    if( image == NULL ){
    	bio_set_lasterror( __FILE__, __LINE__, "Invalid input image pointer ." );
    	return BIOERR_PARAM;	
    }
    
    for( x = 0; x < image->width; x++ ){
		for( y = 0; y < image->height; y++ ){
			if( BIO_PIX(x,y,0) == 0x00 && BIO_PIX(x,y,1) == 0x00 && BIO_PIX(x,y,2) == 0x00 ){
				BIO_PIX(x,y,0) = 0xFF;	
				BIO_PIX(x,y,1) = 0xFF;
				BIO_PIX(x,y,2) = 0xFF;
			}
			else{
				BIO_PIX(x,y,0) = 0x00;	
				BIO_PIX(x,y,1) = 0x00;
				BIO_PIX(x,y,2) = 0x00;
			}
		}	
	}	
           
    return BIOERR_SUCCESS;	
}

bio_error_t bio_mask_dilate( bio_mask_t *mask ){
	int x, y, offset;
	
	if( mask == NULL || mask->mask == NULL ){
		bio_set_lasterror( __FILE__, __LINE__, "Invalid input mask pointer ." );
    	return BIOERR_PARAM;	
    }
    
    for( x = 1; x < mask->width - 1; x++ ){
		for( y = 1; y < mask->height - 1; y++ ){
			offset = x + y * mask->width;
			if( mask->mask[offset] == 0xFF ){
				mask->mask[ (x - 1) + y * mask->width ] |= 0x80;
				mask->mask[ (x + 1) + y * mask->width ] |= 0x80;
				mask->mask[ x + (y - 1) * mask->width ] |= 0x80;
				mask->mask[ x + (y + 1) * mask->width ] |= 0x80;
			}
			else if( mask->mask[offset] ){
				mask->mask[offset] = 0xFF;
			}
		}
	}
	
	return BIOERR_SUCCESS;
}

bio_error_t bio_mask_erode( bio_mask_t *mask ){
	int x, y, offset;
	
	if( mask == NULL || mask->mask == NULL ){
		bio_set_lasterror( __FILE__, __LINE__, "Invalid input mask pointer ." );
    	return BIOERR_PARAM;	
    }
    
    for( x = 1; x < mask->width - 1; x++ ){
		for( y = 1; y < mask->height - 1; y++ ){
			offset = x + y * mask->width;	
			if( mask->mask[offset] == 0x00 ){
				mask->mask[ (x - 1) + y * mask->width ] &= 0x80;
				mask->mask[ (x + 1) + y * mask->width ] &= 0x80;
				mask->mask[ x + (y - 1) * mask->width ] &= 0x80;
				mask->mask[ x + (y + 1) * mask->width ] &= 0x80;
			}
			else if( mask->mask[offset] != 0xFF ){
				mask->mask[offset] = 0x00;
			}		
		}
	}
	
	return BIOERR_SUCCESS;
}

bio_error_t bio_img_clean( bio_image_t *image ){
    int x, y, n, t, i, c;
	
    i = 0;
    do{
		n = 0;    
		for( y = 1; y < image->height - 1; y++ ){
			for( x = 1; x < image->width - 1; x++ ){
				for( c = 0; c < 3; c++ ){
					if( *bio_img_at( image, x, y, c ) == 0xFF ){
						t = 0;
						if( BIO_P3(image,x,y,c) == 0 && BIO_P2(image,x,y,c) != 0 && BIO_P4(image,x,y,c) == 0 ){
							t++;	
						}
						if( BIO_P5(image,x,y,c) == 0 && BIO_P4(image,x,y,c) != 0 && BIO_P6(image,x,y,c) == 0 ){
							t++;	
						}
						if( BIO_P7(image,x,y,c) == 0 && BIO_P6(image,x,y,c) != 0 && BIO_P8(image,x,y,c) == 0 ){
							t++;	
						}
						if( BIO_P9(image,x,y,c) == 0 && BIO_P8(image,x,y,c) != 0 && BIO_P2(image,x,y,c) == 0 ){
							t++;	
						}		
						if( BIO_P3(image,x,y,c) != 0 && BIO_P4(image,x,y,c) == 0 ){
							t++;	
						}	
						if( BIO_P5(image,x,y,c) != 0 && BIO_P6(image,x,y,c) == 0 ){
							t++;	
						}	
						if( BIO_P7(image,x,y,c) != 0 && BIO_P8(image,x,y,c) == 0 ){
							t++;	
						}	
						if( BIO_P9(image,x,y,c) != 0 && BIO_P2(image,x,y,c) == 0 ){
							t++;	
						}		
						if( t == 1 ){
							*bio_img_at( image, x, y, c ) = 0x80;
							n++;
						}
					}
				}
			}
		}
		for( y = 1; y < image->height - 1; y++ ){
			for( x = 1; x < image->width - 1; x++ ){
				for( c = 0; c < 3; c++ ){
					if( BIO_PIX(x,y,c) == 0x80 ){
						BIO_PIX(x,y,c) = 0x00;
					}
				}
			}
    	}
    } 
    while( n > 0 && ++i < 5 );
        
    return BIOERR_SUCCESS;		
}

inline double bio_gabor_factor( int x, int y, double phi, double frequency, double radius ){
    double dr = 1.0 / radius,
           x2, y2,
           sphi,
           cphi;
           
    phi += BIO_H_M_PI;
    sphi = sin(phi);
    cphi = cos(phi); 
    x2   = -x * sphi + y * cphi;
    y2   =  x * cphi + y * sphi;
    
    return exp(-0.5 * (x2*x2*dr + y2*y2*dr)) * cos(BIO_D_M_PI*x2*frequency);
}

bio_error_t bio_img_gabor_enhance( bio_image_t *image, bio_floatmap_t *direction, bio_floatmap_t *frequency, bio_mask_t *mask, double radius ){
	int dheight = image->height - BIO_WG2,
		dwidth  = image->width - BIO_WG2,
		x,y, x2,y2, dy,
		offset;
	bio_image_t enhanced = *image;
	double sum, freq, orient;

    radius = radius * radius;

	enhanced.fill(0x00);
	for( y = BIO_WG2; y < dheight; y++ ){
		for( x = BIO_WG2; x < dwidth; x++ ){
			offset = x + y * image->width;
			if( mask->mask[offset] ){
				sum    = 0.0;
				orient = direction->map[offset];
				freq   = frequency->map[offset];
				for( y2 = -BIO_WG2; y2 <= BIO_WG2; y2++ ){
					dy  = y - y2;
					for( x2 = -BIO_WG2; x2 <= BIO_WG2; x2++ ){
						sum += bio_gabor_factor( x2, y2, orient, freq, radius ) * image->data[(x - x2) + dy * image->width];
					}
				}
				
				if( sum > 255.0 ){ 
					sum = 255.0;
				}
				if( sum < 0.0 ){   
					sum = 0.0;
				}
				
				*enhanced.ptr(x,y,0) = (byte_t)sum;
				*enhanced.ptr(x,y,1) = (byte_t)sum;
				*enhanced.ptr(x,y,2) = (byte_t)sum;
				//~ enhanced.data[offset + 0] = (byte_t)sum;
				//~ enhanced.data[offset + 1] = (byte_t)sum;
				//~ enhanced.data[offset + 2] = (byte_t)sum;
			}
		}
	}
    *image = enhanced;
	
	return BIOERR_SUCCESS;		
}	
