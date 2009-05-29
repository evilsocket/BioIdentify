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
#include "bioerror.h"

static char __bio_lasterror[65535] = {0};

void bio_set_lasterror( const char *file, uint_t line, const char *fmt, ... ){
	char location[0xFF] = {0x00},
	     message[0xFF]  = {0x00};
	va_list ap;
	
	sprintf( location, "[%s:%d]", file, line );
	
	va_start( ap, fmt);
		vsprintf( message, fmt, ap );
	va_end(ap);

	sprintf( __bio_lasterror, "%s : %s", location, message );
}

char *bio_get_lasterror(){
	return __bio_lasterror;
}
