/* 
 * Drag[en]gine IGDE Game Definition Editor
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
#include <string.h>
#include <stdlib.h>

#include "gdeUOCComponentToggleAttachTarget.h"
#include "../../../gamedef/objectClass/gdeObjectClass.h"
#include "../../../gamedef/objectClass/component/gdeOCComponent.h"

#include <dragengine/common/exceptions.h>



// Class gdeUOCComponentToggleAttachTarget
///////////////////////////////////////////

// Constructor, destructor
////////////////////////////

gdeUOCComponentToggleAttachTarget::gdeUOCComponentToggleAttachTarget(
gdeObjectClass *objectClass, gdeOCComponent *component ) :
pObjectClass( NULL ),
pComponent( NULL )
{
	if( ! objectClass || ! component ){
		DETHROW( deeInvalidParam );
	}
	
	SetShortInfo( "Component toggle attach target" );
	
	pComponent = component;
	component->AddReference();
	
	pObjectClass = objectClass;
	objectClass->AddReference();
}

gdeUOCComponentToggleAttachTarget::~gdeUOCComponentToggleAttachTarget(){
	if( pComponent ){
		pComponent->FreeReference();
	}
	if( pObjectClass ){
		pObjectClass->FreeReference();
	}
}



// Management
///////////////

void gdeUOCComponentToggleAttachTarget::Undo(){
	pComponent->SetAttachTarget( ! pComponent->GetAttachTarget() );
	pObjectClass->NotifyComponentChanged( pComponent );
}

void gdeUOCComponentToggleAttachTarget::Redo(){
	Undo();
}
