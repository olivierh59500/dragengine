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

#ifndef _IGDENATIVEFOXCONTANIERFORM_H_
#define _IGDENATIVEFOXCONTANIERFORM_H_

#include "foxtoolkit.h"

class igdeContainerForm;


/**
 * \brief FOX toolkit Native Form Container.
 */
class igdeNativeFoxContainerForm : public FXMatrix{
	FXDECLARE( igdeNativeFoxContainerForm )
protected:
	   igdeNativeFoxContainerForm();
	
private:
	igdeContainerForm *pOwner;
	
	
	
public:
	/** \name Constructors and Destructors */
	/*@{*/
	/** \brief Create container. */
	igdeNativeFoxContainerForm( igdeContainerForm &owner, FXComposite *parent, int layoutFlags );
	
	/** \brief Clean up container. */
	virtual ~igdeNativeFoxContainerForm();
	
	/** \brief Create native widget. */
	static igdeNativeFoxContainerForm* CreateNativeWidget( igdeContainerForm &owner );
	
	/** \brief Post create native widget. */
	virtual void PostCreateNativeWidget();
	
	/** \brief Destroy native widget. */
	virtual void DestroyNativeWidget();
	/*@}*/
	
	
	
	/** \name Management */
	/*@{*/
	virtual void ChildRemoved();
	/*@}*/
	
	
	
	/** \name Events */
	/*@{*/
	long onResize( FXObject *sender, FXSelector selector, void *data );
	long onChildLayoutFlags( FXObject *sender, FXSelector selector, void *data );
	/*@}*/
};

typedef igdeNativeFoxContainerForm igdeNativeContainerForm;

#endif
