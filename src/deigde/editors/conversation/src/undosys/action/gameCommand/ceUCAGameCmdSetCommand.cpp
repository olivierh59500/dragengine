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

#include "ceUCAGameCmdSetCommand.h"
#include "../../../conversation/ceConversation.h"
#include "../../../conversation/action/ceCAGameCommand.h"
#include "../../../conversation/topic/ceConversationTopic.h"

#include <dragengine/common/exceptions.h>



// Class ceUCAGameCmdSetCommand
//////////////////////////////

// Constructor, destructor
////////////////////////////

ceUCAGameCmdSetCommand::ceUCAGameCmdSetCommand( ceConversationTopic *topic, ceCAGameCommand *action, const char *newCommand ){
	if( ! topic || ! newCommand ){
		DETHROW( deeInvalidParam );
	}
	
	pTopic = NULL;
	pAction = NULL;
	pOldCommand = action->GetCommand();
	pNewCommand = newCommand;
	
	SetShortInfo( "Action game command set command" );
	
	pTopic = topic;
	topic->AddReference();
	
	pAction = action;
	action->AddReference();
}

ceUCAGameCmdSetCommand::~ceUCAGameCmdSetCommand(){
	if( pAction ){
		pAction->FreeReference();
	}
	if( pTopic ){
		pTopic->FreeReference();
	}
}



// Management
///////////////

void ceUCAGameCmdSetCommand::Undo(){
	pAction->SetCommand( pOldCommand.GetString() );
	pTopic->NotifyActionChanged( pAction );
}

void ceUCAGameCmdSetCommand::Redo(){
	pAction->SetCommand( pNewCommand.GetString() );
	pTopic->NotifyActionChanged( pAction );
}
