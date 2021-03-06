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

#ifndef _CEUCCSHOTSETROTTO_H_
#define _CEUCCSHOTSETROTTO_H_

#include <deigde/undo/igdeUndo.h>

#include <dragengine/common/math/decMath.h>

class ceCameraShot;



/**
 * \brief Undo action camera shot set end rotation.
 */
class ceUCCShotSetRotTo : public igdeUndo{
private:
	ceCameraShot *pCameraShot;
	
	decVector pOldRot;
	decVector pNewRot;
	
public:
	/** \name Constructors and Destructors */
	/*@{*/
	/** \brief Creates a new undo action. */
	ceUCCShotSetRotTo( ceCameraShot *cameraShot, const decVector &newRot );
	/** \brief Cleans up the undo action. */
	virtual ~ceUCCShotSetRotTo();
	/*@}*/
	
public:
	/** \name Management */
	/*@{*/
	/** \brief Undo action. */
	virtual void Undo();
	/** \brief Redo action. */
	virtual void Redo();
	/*@}*/
};

#endif
