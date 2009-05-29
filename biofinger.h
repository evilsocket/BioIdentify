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
#ifndef __bio_finger_h__
#	define __bio_finger_h__

#include "biotypes.h"
#include "biofloatmap.h"
#include "bioimage.h"
#include "bioerror.h"

bio_error_t bio_finger_direction( bio_image_t *image, bio_floatmap_t *direction, int blocksize, int filtersize );
bio_error_t bio_finger_frequency( bio_image_t *image, bio_floatmap_t *direction, bio_floatmap_t *frequency );
bio_error_t bio_finger_mask( bio_image_t *image, bio_floatmap_t *direction, bio_floatmap_t *frequency, bio_mask_t *mask );
bio_error_t bio_finger_thin( bio_image_t *image, bio_mask_t *mask );
int         bio_feature_cmp( bio_feature_t *ma, bio_feature_t *mb );
bio_error_t bio_finger_extract_features( bio_image_t *image, bio_floatmap_t *direction, bio_mask_t *mask, bio_feature_set_t *features );

#endif


