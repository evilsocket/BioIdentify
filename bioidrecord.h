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
#ifndef __bio_idrecord_h__
#	define __bio_idrecord_h__

#include "biotypes.h"
#include "biofloatmap.h"
#include "bioimage.h"
#include "biofinger.h"
#include "bioerror.h"

bio_error_t bio_bir_compute( bio_image_t *image, bio_bir_t *bir, char *label = NULL );
bio_error_t bio_bir_export( bio_bir_t *bir, const char *filename );
bio_error_t bio_bir_import( bio_bir_t *bir, const char *filename );
double      bio_bir_compare( bio_bir_t *input, bio_bir_t *pattern );
int bio_bir_correlate( bio_bir_t *input, bio_bir_t *pattern, int threshold );

bio_error_t bio_enroll_compute( bio_image_set_t *images, bio_enrollment_t *data, char *identification );
bio_error_t bio_enroll_export( bio_enrollment_t *data, const char *filename );
bio_error_t bio_enroll_import( bio_enrollment_t *data, const char *filename );
double      bio_enroll_compare( bio_bir_t *input, bio_enrollment_t *data );

#endif



