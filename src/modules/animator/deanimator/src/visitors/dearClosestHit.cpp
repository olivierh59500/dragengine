/* 
 * Drag[en]gine Animator Module
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

#include "dearClosestHit.h"

#include <dragengine/deEngine.h>
#include <dragengine/resources/collider/deCollider.h>
#include <dragengine/resources/collider/deCollisionInfo.h>
#include <dragengine/common/exceptions.h>



// Class dearClosestHit
/////////////////////////

// Constructor, destructor
////////////////////////////

dearClosestHit::dearClosestHit(){
	pListener = NULL;
	pHitDistance = 0.0f;
	pHasHit = false;
}

dearClosestHit::~dearClosestHit(){
}



// Management
///////////////

void dearClosestHit::Reset(){
	pHitDistance = 0.0f;
	pHasHit = false;
}

void dearClosestHit::SetListener( deBaseScriptingCollider *listener ){
	pListener = listener;
}



// Notifications
//////////////////

void dearClosestHit::CollisionResponse( deCollider*, deCollisionInfo *info ){
	float distance = info->GetDistance();
	
	/*
	if( info->GetHTSector() ){
		printf( "hit height terrain: cd=%f bd=%f hh=%i\n", distance, pHitDistance, pHasHit ? 1 : 0 );
	}else if( info->GetCollider() ){
		printf( "hit collider: c=%p cd=%f bd=%f hh=%i\n", info->GetCollider(), distance, pHitDistance, pHasHit ? 1 : 0 );
	}else{
		printf( "hit something else: cd=%f bd=%f hh=%i\n", distance, pHitDistance, pHasHit ? 1 : 0 );
	}
	*/
	
	if( pHasHit && distance >= pHitDistance ) return;
	
	pHitDistance = distance;
	pHitNormal = info->GetNormal();
	
	pHasHit = true;
}

bool dearClosestHit::CanHitCollider( deCollider *owner, deCollider *collider ){
	return pListener == NULL || pListener->CanHitCollider( owner, collider );
}

void dearClosestHit::ColliderChanged( deCollider* ){
}
