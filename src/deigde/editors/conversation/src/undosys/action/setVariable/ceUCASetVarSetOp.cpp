/* 
 * Drag[en]gine IGDE Conversation Editor
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

#include "ceUCASetVarSetOp.h"
#include "../../../conversation/ceConversation.h"
#include "../../../conversation/topic/ceConversationTopic.h"

#include <dragengine/common/exceptions.h>



// Class ceUCASetVarSetOp
///////////////////////////

// Constructor, destructor
////////////////////////////

ceUCASetVarSetOp::ceUCASetVarSetOp( ceConversationTopic *topic,
ceCASetVariable *action, ceCASetVariable::eOperators newOperator ){
	if( ! topic || ! action ) DETHROW( deeInvalidParam );
	
	pTopic = NULL;
	pAction = NULL;
	pOldOperator = action->GetOperator();
	pNewOperator = newOperator;
	
	SetShortInfo( "Action SetVariable Set Operator" );
	
	pTopic = topic;
	topic->AddReference();
	
	pAction = action;
	action->AddReference();
}

ceUCASetVarSetOp::~ceUCASetVarSetOp(){
	if( pAction ){
		pAction->FreeReference();
	}
	if( pTopic ){
		pTopic->FreeReference();
	}
}



// Management
///////////////

void ceUCASetVarSetOp::Undo(){
	pAction->SetOperator( pOldOperator );
	pTopic->NotifyActionChanged( pAction );
}

void ceUCASetVarSetOp::Redo(){
	pAction->SetOperator( pNewOperator );
	pTopic->NotifyActionChanged( pAction );
}
