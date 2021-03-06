/* 
 * Drag[en]gine OpenGL Graphic Module
 *
 * Copyright (C) 2020, Roland Plüss (roland@rptd.ch)
 * 
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation; either 
 * version 2 of the License, or (at your option) any later 
 * version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "deoglPointSieveBucket.h"

#include <dragengine/common/exceptions.h>



// Class deoglPointSieveBucket
////////////////////////////////

// Constructor, destructor
////////////////////////////

deoglPointSieveBucket::deoglPointSieveBucket(){
	pIndices = NULL;
	pIndexCount = 0;
	pIndexSize = 0;
}

deoglPointSieveBucket::~deoglPointSieveBucket(){
	if( pIndices ) delete [] pIndices;
}



// Management
///////////////

int deoglPointSieveBucket::GetIndexAt( int position ) const{
	if( position < 0 || position >= pIndexCount ) DETHROW( deeInvalidParam );
	
	return pIndices[ position ];
}

void deoglPointSieveBucket::AddIndex( int index ){
	if( pIndexCount == pIndexSize ){
		int newSize = pIndexSize * 3 / 2 + 1;
		int *newArray = new int[ newSize ];
		if( ! newArray ) DETHROW( deeOutOfMemory );
		if( pIndices ){
			memcpy( newArray, pIndices, sizeof( int ) * pIndexSize );
			delete [] pIndices;
		}
		pIndices = newArray;
		pIndexSize = newSize;
	}
	
	pIndices[ pIndexCount++ ] = index;
}

void deoglPointSieveBucket::RemoveAllIndices(){
	pIndexCount = 0;
}
