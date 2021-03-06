/* 
 * Drag[en]gine IGDE
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

#ifdef IGDE_TOOLKIT_FOX

#include "igdeNativeFoxMenuOption.h"
#include "../../../igdeContainer.h"
#include "../../../igdeCommonDialogs.h"
#include "../../../event/igdeAction.h"
#include "../../../menu/igdeMenuOption.h"

#include <dragengine/common/exceptions.h>
#include <dragengine/logger/deLogger.h>



// Events
///////////

FXDEFMAP( igdeNativeFoxMenuOption ) igdeNativeFoxMenuOptionMap[] = {
	FXMAPFUNC( SEL_COMMAND, igdeNativeFoxMenuOption::ID_SELF, igdeNativeFoxMenuOption::onMenuAction ),
	FXMAPFUNC( SEL_UPDATE, igdeNativeFoxMenuOption::ID_SELF, igdeNativeFoxMenuOption::updateMenuAction )
};



// Class igdeNativeFoxMenuOption
/////////////////////////////////

FXIMPLEMENT( igdeNativeFoxMenuOption, FXMenuRadio,
	igdeNativeFoxMenuOptionMap, ARRAYNUMBER( igdeNativeFoxMenuOptionMap ) )

// Constructor, destructor
////////////////////////////

igdeNativeFoxMenuOption::igdeNativeFoxMenuOption(){ }

igdeNativeFoxMenuOption::igdeNativeFoxMenuOption( igdeMenuOption &owner, FXComposite *parent ) :
FXMenuRadio( parent, BuildConstrText( owner ), this, ID_SELF ),
pOwner( &owner )
{
	if( ! owner.GetEnabled() ){
		disable();
	}
	
	setCheck( owner.GetSelected() );
}

igdeNativeFoxMenuOption::~igdeNativeFoxMenuOption(){
}

igdeNativeFoxMenuOption *igdeNativeFoxMenuOption::CreateNativeWidget( igdeMenuOption &owner ){
	if( ! owner.GetParent() ){
		DETHROW( deeInvalidParam );
	}
	
	FXComposite * const parent = ( FXComposite* )owner.GetParent()->GetNativeContainer();
	if( ! parent ){
		DETHROW( deeInvalidParam );
	}
	
	return new igdeNativeFoxMenuOption( owner, parent );
}

void igdeNativeFoxMenuOption::PostCreateNativeWidget(){
	FXComposite &parent = *( ( FXComposite* )pOwner->GetParent()->GetNativeContainer() );
	if( parent.id() ){
		create();
	}
}

void igdeNativeFoxMenuOption::DestroyNativeWidget(){
	delete this;
}



// Management
///////////////

void igdeNativeFoxMenuOption::UpdateSelected(){
	setCheck( pOwner->GetSelected() );
}


FXString igdeNativeFoxMenuOption::BuildConstrText( igdeMenuOption &owner ){
	return igdeUIFoxHelper::MnemonizeString( owner.GetText(), owner.GetMnemonic() )
		+ "\t" + igdeUIFoxHelper::AccelString( owner.GetHotKey() )
		+ "\t" + owner.GetDescription().GetString();
}



// Events
///////////

long igdeNativeFoxMenuOption::onMenuAction( FXObject *sender, FXSelector selector, void *data ){
	if( ! pOwner->GetEnabled() ){
		return 0;
	}
	
	pOwner->SetSelected( true );
	// TODO set others in group unselected
	
	try{
		pOwner->OnAction();
		
	}catch( const deException &e ){
		pOwner->GetLogger()->LogException( "IGDE", e );
		igdeCommonDialogs::Exception( pOwner, e );
		return 0;
	}
	
	return 1;
}

long igdeNativeFoxMenuOption::updateMenuAction( FXObject *sender, FXSelector selector, void *data ){
	igdeAction * const action = pOwner->GetAction();
	if( ! action ){
		return 0;
	}
	
	try{
		action->Update();
		
	}catch( const deException &e ){
		pOwner->GetLogger()->LogException( "IGDE", e );
	}
	
	return 0;
}

#endif
