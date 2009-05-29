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
#include "bioidrecord.h"

bool bio_feature_sort_predicate( bio_feature_t *f1, bio_feature_t *f2 ){
   return f1->x < f2->x;
}


inline void bio_features_sort( bio_feature_set_t *features ){
	sort( features->begin(), features->end(), bio_feature_sort_predicate );
}

bio_error_t bio_bir_compute( bio_image_t *image, bio_bir_t *bir, char *label ){
	bio_mask_t     mask;
	bio_floatmap_t direction,
				   frequency;
				     	
	memset( &direction, 0x00, sizeof(bio_floatmap_t) );
	memset( &frequency, 0x00, sizeof(bio_floatmap_t) );
	
	#ifdef BIO_DEBUG
		bio_image_t original = *image;
		bio_img_export( image, "debug/00_original.bmp" );
	#endif
	
	bio_img_soften_mean( image, 3 );
	#ifdef BIO_DEBUG
		bio_img_export( image, "debug/01_softenmean.bmp" );
	#endif
	
	if( bio_finger_direction( image, &direction, 7, 8 ) != BIOERR_SUCCESS ){
		return BIOERR_MEMORY;	
	}
	
	if( bio_finger_frequency( image, &direction, &frequency ) != BIOERR_SUCCESS ){
		return BIOERR_MEMORY;	
	}
	
	if( bio_finger_mask( image, &direction, &frequency, &mask ) != BIOERR_SUCCESS ){
		return BIOERR_MEMORY;	
	}
	
	if( bio_img_gabor_enhance( image, &direction, &frequency, &mask, 4.0 ) != BIOERR_SUCCESS ){
		free(mask.mask);
		return BIOERR_MEMORY;	
	}
	#ifdef BIO_DEBUG
		bio_img_export( image, "debug/02_gabor.bmp" );
	#endif
		
	if( bio_img_binarize( image ) != BIOERR_SUCCESS ){
		free(mask.mask);
		return BIOERR_MEMORY;	
	}
	#ifdef BIO_DEBUG
		bio_img_export( image, "debug/03_binarized.bmp" );
	#endif
	
	if( bio_finger_thin( image, &mask ) != BIOERR_SUCCESS ){
		free(mask.mask);
		return BIOERR_MEMORY;	
	}
	#ifdef BIO_DEBUG
		bio_img_export( image, "debug/04_thinned.bmp" );
	#endif
	
	memcpy( bir->magic, BIO_BIR_MAGIC, sizeof(bir->magic) );
	gettimeofday(&bir->time,NULL);
	bir->patterns = 1;
	if( label != NULL ){
		strncpy( bir->label, label, 0xFF );
	}
	bir->features = new bio_feature_set_t;
	
	if( bio_finger_extract_features( image, &direction, &mask, bir->features ) != BIOERR_SUCCESS ){
		delete bir->features;
		free(mask.mask);
		return BIOERR_MEMORY;	
	}
	
	free(mask.mask);
	
	#ifdef BIO_DEBUG
		bio_feature_set_iterator_t fi;
		byte_t red[] = { 0xFF, 0, 0 };
		for( fi = bir->features->begin(); fi != bir->features->end(); fi++ ){
			 original.draw_rectangle( (*fi)->x - 1, (*fi)->y - 1, (*fi)->x + 1, (*fi)->y + 1, red ); 	
		}
		bio_img_export( &original, "debug/05_featured.bmp" );
	#endif
	
	return BIOERR_SUCCESS;
}

bio_error_t bio_bir_export( bio_bir_t *bir, const char *filename ){
	FILE *fp = NULL;
	bio_feature_set_iterator_t fi;
	
	if( !(fp = fopen( filename, "w+b" )) ){
		bio_set_lasterror( __FILE__, __LINE__, "Could not open '%s' for writing .", filename );
		return BIOERR_FAILURE;	
	}
	
	fwrite( bir->magic,     1, sizeof(bir->magic),     fp );
	fwrite( &bir->time,     1, sizeof(struct timeval), fp );
	fwrite( &bir->patterns, 1, sizeof(uint_t),         fp );
	fwrite( bir->label,     1, 0xFF,                   fp );
	
	for( fi = bir->features->begin(); fi != bir->features->end(); fi++ ){
		if( *fi != NULL ){
			fwrite( *fi, 1, sizeof(bio_feature_t), fp );	
		}
	}
	
	fclose(fp);
	
	return BIOERR_SUCCESS;
}	

bio_error_t bio_bir_import( bio_bir_t *bir, const char *filename ){
	FILE *fp = NULL;
	bio_feature_t feature;
	
	if( !(fp = fopen( filename, "rb" )) ){
		bio_set_lasterror( __FILE__, __LINE__, "Could not open '%s' for reading .", filename );
		return BIOERR_FAILURE;	
	}
	
	fread( bir->magic, 1, sizeof(bir->magic), fp );
	if( memcmp( bir->magic, BIO_BIR_MAGIC, sizeof(bir->magic) ) != 0 ){
		return BIOERR_FAILURE;	
	}
	
	fread( &bir->time,     1, sizeof(struct timeval), fp );
	fread( &bir->patterns, 1, sizeof(uint_t),         fp );
	fread( bir->label,     1, 0xFF,                   fp );
	
	bir->features = new bio_feature_set_t;
	while( feof(fp) == 0 ){
		fread( &feature, 1, sizeof(bio_feature_t), fp );
		bir->features->push_back( new bio_feature_t(&feature) );
	}
		
	fclose(fp);
	
	return BIOERR_SUCCESS;
}

static void bio_find_max_distance( bio_feature_set_t *set, int *neighbours, int ref, double *distance, int *idx ){
	int i, 
	  	maxi      = 0, 
	  	neighbour = neighbours[0];
	double deltax = (*set)[neighbour]->x - (*set)[ref]->x,
		   deltay = (*set)[neighbour]->y - (*set)[ref]->y,
		   maxd   = sqrt(deltax * deltax + deltay * deltay), 
		   tmpd;

	for( i = 1; i < BIO_NEIGHBOURS; i++ ){
		neighbour = neighbours[i];
		deltax    = (*set)[neighbour]->x - (*set)[ref]->x;
		deltay    = (*set)[neighbour]->y - (*set)[ref]->y;
		tmpd      = sqrt(deltax * deltax + deltay * deltay);

		if( tmpd > maxd ){
			maxd = tmpd;
			maxi = i;
		}
	}

	*distance = maxd;
	*idx      = maxi;
}

static void bio_find_neighbours( bio_feature_set_t *set, int **neighbours ){
	int i, j, k, idx, end, wrap = 0;
	double deltax, deltay, distance, maxdis;

	for( i = 0; i < set->size(); i++ ){
		j = i + 1;
		for( k = 0; k < BIO_NEIGHBOURS; k++ ){
			if( j >= set->size() ){
				j    = 0;
				wrap = 1;
			}
			neighbours[i][k] = j++;
		}

		bio_find_max_distance( set, neighbours[i], i, &maxdis, &idx );

		end = (wrap ? i     : set->size());
		j   = (wrap ? i - 1 : j);
		
		while( (j >= 0 && j < set->size()) && (((*set)[j]->x - (*set)[i]->x) < maxdis) && (j < end) ){
			deltax   = (*set)[j]->x - (*set)[i]->x;
			deltay   = (*set)[j]->y - (*set)[i]->y;
			distance = sqrt(deltax * deltax + deltay * deltay);

			if(distance < maxdis){
				neighbours[i][idx] = j;
				bio_find_max_distance( set, neighbours[i], i, &maxdis, &idx );
			}
			
			j += (wrap ? -1 : 1);
		}

		if(!wrap){
			j = (i ? i - 1 : i);
			while( (j >= 0) && (((*set)[i]->x - (*set)[j]->x) < maxdis) ){
				deltax   = (*set)[i]->x - (*set)[j]->x;
				deltay   = (*set)[i]->y - (*set)[j]->y;
				distance = sqrt(deltax * deltax + deltay * deltay);

				if(distance < maxdis){
					neighbours[i][idx] = j;
					bio_find_max_distance( set, neighbours[i], i, &maxdis, &idx );
				}
				j--;
			}
		}
	}	
}

inline double bio_pi_diff( double a, double b ){
	double dif = a - b;
  
	if( dif > -BIO_M_PI && dif <= BIO_M_PI ){
		return dif;
	}
	else if( dif <= -BIO_M_PI ){
		return (BIO_D_M_PI + dif);
	}
	else{
		return (BIO_D_M_PI - dif);
	}
}

static void bio_compute_featureset_area( bio_feature_set_t *set, int **neighbours, bio_feature_area_t *areaset ){
	double deltax, deltay;
	int i, j, neighbour;

	for( i = 0; i < set->size(); i++ ){
		for( j = 0; j < BIO_NEIGHBOURS; j++ ){
			neighbour = neighbours[i][j];

			deltax                 = (*set)[neighbour]->x - (*set)[i]->x;
			deltay                 = (*set)[neighbour]->y - (*set)[i]->y;
			
			areaset[i].distance[j] = sqrt(deltax * deltax + deltay * deltay);
			areaset[i].dphi[j]     = bio_pi_diff( (*set)[neighbour]->angle, (*set)[i]->angle );
			areaset[i].dtheta[j]   = bio_pi_diff( atan2(deltay,deltax), (*set)[i]->angle );
			areaset[i].angle       = (*set)[i]->angle;
			areaset[i].type        = (*set)[i]->type;
		}
	}
}

static int bio_compare_featureset_area( bio_feature_area_t* input, int isize, bio_feature_area_t *pattern, int psize ){
	int marked  = 0, 
		matched = 0,
		i, j, k, l;

	for(i = 0; i < isize; i++){
		for (j = 0; j < psize; j++){
			if( fabs(input[i].angle - pattern[j].angle) <= BIO_DELTA_ANGLE ){	
				marked = 0;
				for( k = 0; k < BIO_NEIGHBOURS; k++ ){
					for( l = 0; l < BIO_NEIGHBOURS; l++ ){
						if( fabs(input[i].distance[k] - pattern[j].distance[l]) <= BIO_DELTA_DISTANCE &&
							fabs(input[i].dphi[k] - pattern[j].dphi[l]) <= BIO_DELTA_PHI &&
							fabs(input[i].dtheta[k] - pattern[j].dtheta[l]) <= BIO_DELTA_THETA &&
							input[i].type == pattern[j].type ){					
							marked++;
						}
					}
				}

				if( marked > BIO_THRESHOLD_MARKED ){
					matched++;
				}
			}
		}
	}

	return matched;	
}

int bio_bir_correlate( bio_bir_t *input, bio_bir_t *pattern, int threshold ){
	double matches   = 0,
	       bestmatch = 0,
	       max       = input->features->size(),
	       radian  = BIO_M_PI / 180.0 ;
	int k, i, j, foundpoint = 0;

	for( k = -10; k <= 10; k++ ){
		for( i = 0; i < max; i++ ){		
			foundpoint = 0;  
			for( j = 0; j < pattern->features->size(); j++ ){
				if( foundpoint == 0 ){
					int resx=0;
					int resy=0;
					double x1=0 ;
					double y1=0;
					double x2=0;
					double y2=0;
					double r=0;
					double d=0;
					
					r  = pattern->features->at(j)->radius;
					d  = pattern->features->at(j)->angle;
					x2 = input->features->at(i)->x;
					y2 = input->features->at(i)->y;
					
					x1   = r * cos( d + (k*radian) );
					resx = abs( (int)x2 + (int)(-1*x1) );
					y1   = r * sin( d + (k*radian) );
					resy = abs( (int)y2 + (int)(-1*y1) );
				   
					if( (30 > resx) && (30 > resy) ){
						if( input->features->at(i)->type == pattern->features->at(j)->type ){
							matches++;
							foundpoint = 1;
						}
					}
				}
			}
		}

 
		if( matches > bestmatch ){
			bestmatch = matches;
		}
			
		matches = 0;   	  
	}

	return (int)((bestmatch/max)*100);
}

double bio_bir_compare( bio_bir_t *input, bio_bir_t *pattern ){
	int **ineighbours,
	    **pneighbours,
	    i;
	double matches, max;
	    
	bio_feature_area_t *iarea = new bio_feature_area_t[input->features->size()],
				       *parea = new bio_feature_area_t[pattern->features->size()];
  
	bio_features_sort(input->features);
	bio_features_sort(pattern->features);
	
	ineighbours = new int *[input->features->size()];
	for( i = 0; i < input->features->size(); i++ ){
		ineighbours[i] = new int[BIO_NEIGHBOURS];
	}	
	
	pneighbours = new int *[pattern->features->size()];
	for( i = 0; i < pattern->features->size(); i++ ){
		pneighbours[i] = new int[BIO_NEIGHBOURS];
	}	
	
	bio_find_neighbours( input->features,   ineighbours );
	bio_find_neighbours( pattern->features, pneighbours );
	
	bio_compute_featureset_area( input->features,   ineighbours, iarea );
	bio_compute_featureset_area( pattern->features, pneighbours, parea );
	
	matches = (double)bio_compare_featureset_area( iarea, input->features->size(), parea, pattern->features->size() );
	max     = (double)pattern->features->size();//((input->features->size() > pattern->features->size()) ? input->features->size() : pattern->features->size());
	
	for( i = 0; i < input->features->size(); i++ ){
		delete[] ineighbours[i];
	}	
	delete[] ineighbours;
	for( i = 0; i < pattern->features->size(); i++ ){
		delete[] pneighbours[i];
	}	
	delete[] pneighbours;
	delete[] iarea;
	delete[] parea;
	
	return matches / max;
}

bio_error_t bio_enroll_compute( bio_image_set_t *images, bio_enrollment_t *data, char *identification ){
	bio_image_set_iterator_t ii;
	int i;
	
	if( images->size() < 3 ){
		bio_set_lasterror( __FILE__, __LINE__, "Enrollment process requires at least 3 fingerprints ." );
		return BIOERR_PARAM;		
	}
	if( identification == NULL ){
		bio_set_lasterror( __FILE__, __LINE__, "Identification is mandatory ." );
		return BIOERR_PARAM;		
	}

	for( i = 0, ii = images->begin(); ii != images->end() && i < 3; ii++, i++ ){
		if( bio_bir_compute( *ii, &data->birs[i], identification ) != BIOERR_SUCCESS ){
			return BIOERR_MEMORY;
		}
	}
	
	return BIOERR_SUCCESS;
}

bio_error_t bio_enroll_export( bio_enrollment_t *data, const char *filename ){
	FILE *fp = NULL;
	bio_feature_set_iterator_t fi;
	int i;
	
	if( !(fp = fopen( filename, "w+b" )) ){
		bio_set_lasterror( __FILE__, __LINE__, "Could not open '%s' for writing .", filename );
		return BIOERR_FAILURE;	
	}
	
	fwrite( data->magic,          1, sizeof(data->magic),          fp );
	fwrite( data->identification, 1, sizeof(data->identification), fp );
	
	for( i = 0; i < 3; i++ ){
		fwrite( data->birs[i].magic,     1, sizeof(data->birs[i].magic), fp );
		fwrite( &data->birs[i].time,     1, sizeof(struct timeval),      fp );
		fwrite( &data->birs[i].patterns, 1, sizeof(uint_t),              fp );
		fwrite( data->birs[i].label,     1, 0xFF,                        fp );
		
		for( fi = data->birs[i].features->begin(); fi != data->birs[i].features->end(); fi++ ){
			if( *fi != NULL ){
				fwrite( *fi, 1, sizeof(bio_feature_t), fp );	
			}
		}
	}
	
	fclose(fp);
	
	return BIOERR_SUCCESS;
}

bio_error_t bio_enroll_import( bio_enrollment_t *data, const char *filename ){
	FILE *fp = NULL;
	bio_feature_set_iterator_t fi;
	bio_feature_t feature;
	int i;
	
	if( !(fp = fopen( filename, "rb" )) ){
		bio_set_lasterror( __FILE__, __LINE__, "Could not open '%s' for reading .", filename );
		return BIOERR_FAILURE;	
	}
	
	fread( data->magic,          1, sizeof(data->magic),          fp );
	if( memcmp( data->magic, BIO_BIR_MAGIC, sizeof(data->magic) ) != 0 ){
		bio_set_lasterror( __FILE__, __LINE__, "Invalid biometric file '%s' .", filename );
		return BIOERR_FAILURE;	
	}
	fread( data->identification, 1, sizeof(data->identification), fp );
	
	for( i = 0; i < 3; i++ ){
		fread( data->birs[i].magic, 1, sizeof(data->birs[i].magic), fp );
		if( memcmp( data->birs[i].magic, BIO_BIR_MAGIC, sizeof(data->birs[i].magic) ) != 0 ){
			bio_set_lasterror( __FILE__, __LINE__, "Invalid biometric record in file '%s' .", filename );
			return BIOERR_FAILURE;	
		}
		
		fread( &data->birs[i].time,     1, sizeof(struct timeval), fp );
		fread( &data->birs[i].patterns, 1, sizeof(uint_t),         fp );
		fread( data->birs[i].label,     1, 0xFF,                   fp );
		
		data->birs[i].features = new bio_feature_set_t;
		while( feof(fp) == 0 ){
			fread( &feature, 1, sizeof(bio_feature_t), fp );
			data->birs[i].features->push_back( new bio_feature_t(&feature) );
		}
	}
	
	fclose(fp);
	
	return BIOERR_SUCCESS;	
}

double bio_enroll_compare( bio_bir_t *input, bio_enrollment_t *data ){
	double matches[3] = {0.0};
	int i;
	
	for( i = 0; i < 3; i++ ){
		matches[i] = bio_bir_compare( input, &data->birs[i] );	
	}
	
	return (matches[0]+matches[1]+matches[2]) / 3.0;
}
