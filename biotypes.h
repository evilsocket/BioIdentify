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
#ifndef __bio_types_h__
#	define __bio_types_h__

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <CImg.h>
#include <vector>
#include <algorithm>

using namespace std;
using namespace cimg_library;

/* base types definition */
typedef unsigned char byte_t;
typedef unsigned int  uint_t;

typedef CImg<byte_t>              bio_image_t;
typedef vector<bio_image_t *>     bio_image_set_t;
typedef bio_image_set_t::iterator bio_image_set_iterator_t;

typedef short bio_error_t;	

/* debug flag, save stages images */
#define  BIO_DEBUG	   

/* error codes */
#define BIOERR_FAILURE -1
#define BIOERR_SUCCESS 0x00
#define BIOERR_MEMORY  0x01
#define BIOERR_PARAM   0x02
#define BIOERR_FORMAT  0x03
#define BIOERR_IO      0x04

/* math constants definition */
#ifndef M_PI
#	define M_PI 3.1415926535897932384626433832795
#endif

#define BIO_M_PI    M_PI
/* M_PI / 2.0 */
#define BIO_H_M_PI  1.5707963267948965579989817342720925807952880859375
/* M_PI * 2.0 */
#define BIO_D_M_PI  6.28318530717958623199592693708837032318115234375

/* frequency thresholds */
#define BIO_FREQ_MIN 0.04
#define BIO_FREQ_MAX 0.33333333333333333333333333333333333333333333333

#define BIO_WG2     8

#define BIO_EPSILON 0.0001

#define BIO_BLOCK_W  16
#define BIO_BLOCK_W2 8
#define BIO_BLOCK_L  32
#define BIO_BLOCK_L2 16

#define BIO_LPSIZE   3
/* (1.0 / ((BIO_LPSIZE * 2 + 1) * (BIO_LPSIZE * 2 + 1))) */
#define BIO_LPFACTOR 0.020408163 

#define BIO_EDGE_THRESHOLD 8

typedef struct bio_floatmap{
	double *map;
	int     width;
	int     height;
	int     pitch;
	
	bio_floatmap(){
		map    = NULL;
		width  = 0;
		height = 0;
		pitch  = 0;	
	}
}
bio_floatmap_t;

typedef struct bio_mask{
	byte_t *mask;
	int     width;
	int     height;
	
	bio_mask(){
		mask   = NULL;
		width  = 0;
		height = 0;	
	}
}
bio_mask_t;

typedef struct{
    uint_t table[256];
    int    size;
    int    mean;
    int    variance;
} 
bio_histogram_t;

typedef enum{
    bio_ending    = 0,
    bio_branching = 1,
    /*bio_core      = 2,*/
    bio_delta     = 3,
} 
bio_feature_type_t;

typedef struct bio_feature{
    bio_feature_type_t type;
    int                x;
    int                y;
    double             angle;
    double             radius;
    
    bio_feature(){}
    
    bio_feature( bio_feature_type_t _type, int _x, int _y, double _a ){
    	type   = _type;
    	x      = _x;
    	y      = _y;
    	angle  = _a;
    	radius = sqrt(x*x + y*y);
    }
    
    bio_feature( struct bio_feature * f ){
    	type   = f->type;
    	x      = f->x;
    	y      = f->y;
    	angle  = f->angle;
    	radius = f->radius;
    }
} 
bio_feature_t;

typedef vector<bio_feature_t *>     bio_feature_set_t;
typedef bio_feature_set_t::iterator bio_feature_set_iterator_t;

#define BIO_BIR_MAGIC "BIR\x0"

typedef struct{
	byte_t             magic[4];
	struct timeval     time;
	uint_t             patterns;
	char               label[0xFF];
	bio_feature_set_t *features;
}
bio_bir_t;

typedef struct{
	byte_t    magic[4];
	char      identification[0xFF];
	bio_bir_t birs[3];
}
bio_enrollment_t;

#define BIO_NEIGHBOURS 6

#define BIO_DELTA_ANGLE    (4 * BIO_M_PI) / 180
#define BIO_DELTA_DISTANCE 50
#define BIO_DELTA_PHI      (1 * BIO_M_PI) / 180
#define BIO_DELTA_THETA    (2 * BIO_M_PI) / 180

#define BIO_THRESHOLD_MARKED 2

typedef struct{
	double             distance[BIO_NEIGHBOURS];
	double             dphi[BIO_NEIGHBOURS];
	double             dtheta[BIO_NEIGHBOURS];
	double             angle;
	bio_feature_type_t type;
}
bio_feature_area_t;

/** TODO TODO TODO **/
typedef struct{
	double x;
	double y;
	double alpha;	
}
bio_cmp_tolerances_t;
/** TODO TODO TODO **/
typedef struct{
	double alpha;
	double distance;
	double phi;	
	double theta;
	
	int    marked;
}
bio_match_tolerances_t;
/** TODO TODO TODO **/
typedef struct bio_thresholds{
	bio_cmp_tolerances_t   cmp;
	bio_match_tolerances_t match;
	
	/* assign default values on base ctor */
	bio_thresholds(){
		cmp.x     = 8.0;
		cmp.y     = 8.0;
		cmp.alpha = 5.0;
	
		match.alpha    = BIO_DELTA_ANGLE;
		match.distance = BIO_DELTA_DISTANCE;
		match.phi      = BIO_DELTA_PHI;
		match.theta    = BIO_DELTA_THETA;
		match.marked   = BIO_THRESHOLD_MARKED;
	}
}
bio_thresholds_t;

static bio_thresholds_t __bio_thresholds;

#endif
