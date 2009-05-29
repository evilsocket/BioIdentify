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
#ifndef __bio_floatmap_h__
#	define __bio_floatmap_h__

#include "biotypes.h"

bio_floatmap_t * bio_floatmap_create();
void             bio_floatmap_destroy( bio_floatmap_t *fmap );
bio_error_t      bio_floatmap_setsize( bio_floatmap_t *fmap, int width, int height );
bio_error_t      bio_floatmap_copy( bio_floatmap_t *dst, bio_floatmap_t *src );
void             bio_floatmap_fill( bio_floatmap_t *fmap, double value );
void             bio_floatmap_clear( bio_floatmap_t *fmap );

#define          bio_floatmap_get(m,x,y)   m->map[ y * m->pitch + x ]
#define          bio_floatmap_set(m,x,y,v) m->map[ y * m->pitch + x ] = v

#endif


