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
#include <sys/time.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#include "bioidrecord.h"
#include "biotypes.h"

#define BIO_DATABASE_PATH "database"

void bio_usage( char *app ){
	printf( "Usage: %s -f <fingerprint_image> -b <bir_file_name> -A [ACTION] (-l <label>) (-G)\n"
			"Compute a fingerprint biometric identification record or print match value with desided\n"
			"fingerprint against given BIR .\n\n"
			"All parameters are mandatory .\n"
			"\t-f <fingerprint_image> : File path of fingerprint image to elaborate .\n"
			"\t-b <bir_file_name>     : File path of biometric identification record .\n"
			"\t-A [ACTION]            : ACTION may be `enroll` , `match` or `identify` (case sensitive) .\n"
			"\t-l <label>             : Optional label to assign to BIR in enrollment stage .\n"
			"\t-G                     : Show a simple gui .\n", app );
	exit(0);
}

typedef struct bio_match{
	char   filename[0xFF];
	double score;	
	
	bio_match( char *file, double s ){
		strcpy( filename, file );
		score = s;	
	}
}
bio_match_t;

bool bio_match_sort_predicate( bio_match_t *m1, bio_match_t *m2 ){
   return m1->score > m2->score;
}

int main( int argc, char *argv[] ){
	char *f_file = NULL,
		 *b_file = NULL,
		 *action = NULL,
		 *label  = NULL,
		 c;
	int  gui = 0;
	
	printf( "BioIdentify v1.0 Copyright(c) 2008-2009 by evilsocket\n\n" );

	while( (c = getopt(argc,argv,"f:b:A:l:G")) != -1 ){
		switch(c){
			case 'f' : f_file = strdup(optarg); break;
			case 'b' : b_file = strdup(optarg); break;
			case 'A' : action = strdup(optarg); break;
			case 'l' : label  = strdup(optarg); break;
			case 'G' : gui    = 1;              break;
			
			default  : bio_usage(strdup(argv[0]));
		}
	}

	bio_image_t           fingerprint,original;
	bio_bir_t             bir, bir_match;
	vector<bio_match_t *> scores; 
	
	if( action == NULL ){
		bio_usage(strdup(argv[0]));	
	}
	
	// BIR creation
	if( strcmp( action, "enroll" ) == 0 ){
		if( f_file == NULL || b_file == NULL ){
			bio_usage(strdup(argv[0]));	
		}
		bio_img_import( &fingerprint, f_file );
		original = fingerprint;
		printf( "@ Computing fingerprint biometric identification record of %s ... ", f_file );
		fflush(stdout);
		if( bio_bir_compute( &fingerprint, &bir, label ) != BIOERR_SUCCESS ){
			printf( "FAILED !\n" );
			printf( "@ %s\n", bio_get_lasterror() );
			exit(-1);	
		}
		printf( "done .\n" );
		printf( "@ Exporting BIR to %s ... ", b_file );
		fflush(stdout);
		if( bio_bir_export( &bir, b_file ) != BIOERR_SUCCESS ){
			printf( "FAILED !\n" );
			printf( "@ %s\n", bio_get_lasterror() );
			exit(-1);	
		}
		printf( "done .\n" );
		if(gui){
			bio_feature_set_iterator_t fi;
			byte_t red[]   = { 0xFF, 0, 0 },
				   green[] = { 0, 0xFF, 0 },
				   blue[]  = { 0, 0, 0xFF },
				   *color; 
			double fx,fy;
			//fingerprint = original;
			for( fi = bir.features->begin(); fi != bir.features->end(); fi++ ){
				if( (*fi)->x < fingerprint.width - 5 && (*fi)->x > 4 ){
					if( (*fi)->y < fingerprint.height - 5 && (*fi)->y > 4 ){
						switch( (*fi)->type ){
							case bio_ending    : color = red;   break;
							case bio_delta     : color = blue;  break;
							case bio_branching : color = green; break;	
						}
						
						fingerprint.draw_line( (*fi)->x - 2, (*fi)->y - 2, (*fi)->x - 2, (*fi)->y + 2, color );
						fingerprint.draw_line( (*fi)->x - 2, (*fi)->y + 2, (*fi)->x + 2, (*fi)->y + 2, color );
						fingerprint.draw_line( (*fi)->x + 2, (*fi)->y + 2, (*fi)->x + 2, (*fi)->y - 2, color );
						fingerprint.draw_line( (*fi)->x + 2, (*fi)->y - 2, (*fi)->x - 2, (*fi)->y - 2, color );
						
						fx = sin((*fi)->angle);
						fy = -cos((*fi)->angle);
						*fingerprint.ptr((*fi)->x+(int)(fx)    ,(*fi)->y+(int)(fy)    ,0) = 0xFF;
						*fingerprint.ptr((*fi)->x+(int)(fx*2.0),(*fi)->y+(int)(fy*2.0),0) = 0xFF;
						*fingerprint.ptr((*fi)->x+(int)(fx*3.0),(*fi)->y+(int)(fy*3.0),0) = 0xFF;
						*fingerprint.ptr((*fi)->x+(int)(fx*4.0),(*fi)->y+(int)(fy*4.0),0) = 0xFF;
						*fingerprint.ptr((*fi)->x+(int)(fx*5.0),(*fi)->y+(int)(fy*5.0),0) = 0xFF;
					}
				}
			}
			CImgList<byte_t>  panels( original, fingerprint );
			CImgDisplay       maindisplay( panels, "Feature Extraction");
			do{
				maindisplay.display(panels).resize();
			}
			while( !maindisplay.is_closed );
		}
	}
	// 1 on 1 match
	else if( strcmp( action, "match" ) == 0 ){
		if( f_file == NULL || b_file == NULL ){
			bio_usage(strdup(argv[0]));	
		}
		bio_img_import( &fingerprint, f_file );
		printf( "@ Computing fingerprint biometric identification record of %s ... ", f_file );
		fflush(stdout);
		if( bio_bir_compute( &fingerprint, &bir_match ) != BIOERR_SUCCESS ){
			printf( "FAILED !\n" );
			printf( "@ %s\n", bio_get_lasterror() );
			exit(-1);	
		}
		printf( "done .\n" );
		
		printf( "@ Loading template BIR %s ... ", b_file );
		fflush(stdout);
		if( bio_bir_import( &bir, b_file ) != BIOERR_SUCCESS ){
			printf( "FAILED !\n" );
			printf( "@ %s\n", bio_get_lasterror() );
			exit(-1);	
		}
		printf( "done .\n" );
		printf( "@ SCORE (%s -> %s) : %f .\n", bir.label, f_file, bio_bir_compare(&bir_match,&bir) );
	}
	// 1 on N match 
	else if( strcmp( action, "identify" ) == 0 ){
		if( f_file == NULL ){
			bio_usage(strdup(argv[0]));	
		}
		bio_img_import( &fingerprint, f_file );
		printf( "@ Computing fingerprint biometric identification record of %s ... ", f_file );
		fflush(stdout);
		if( bio_bir_compute( &fingerprint, &bir_match ) != BIOERR_SUCCESS ){
			printf( "FAILED !\n" );
			printf( "@ %s\n", bio_get_lasterror() );
			exit(-1);	
		}
		printf( "done .\n" );
		
		DIR *dp;
		struct dirent *dent;	
		char filename[0xFF] = {0};
		
		dp = opendir( BIO_DATABASE_PATH );
		if( dp == NULL ){
			return -1;
		}
		printf( "@ Searching best matches from database ... " );
		fflush(stdout);
		while( (dent = readdir(dp)) != NULL ){
			if( strstr( dent->d_name, ".BIR" ) != 0 ){
				sprintf( filename, "%s/%s", BIO_DATABASE_PATH, dent->d_name );
				if( bio_bir_import( &bir, filename ) != BIOERR_SUCCESS ){
					printf( "FAILED !\n" );
					printf( "@ %s\n", bio_get_lasterror() );
					exit(-1);	
				}
				scores.push_back( new bio_match_t( dent->d_name, bio_bir_compare(&bir_match,&bir) ) );
			}
		}
		closedir( dp );
		sort( scores.begin(), scores.end(), bio_match_sort_predicate );
		printf( "done .\n\n" );
		
		for( int i = 0; i < scores.size() && scores[i]->score; i++ ){
			printf( "[%.2d] %s\t\t : %f\n", i + 1, scores[i]->filename, scores[i]->score );	
		}
		
		printf("\n");
	}
	else{
		bio_usage(strdup(argv[0]));
	}
	
	return 0;	
}

