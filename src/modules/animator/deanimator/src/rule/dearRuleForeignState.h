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

#ifndef _DEARRULEFOREIGNSTATE_H_
#define _DEARRULEFOREIGNSTATE_H_

#include "dearRule.h"

#include <dragengine/resources/animator/rule/deAnimatorRuleForeignState.h>



/**
 * \brief Foreign state rule.
 */
class dearRuleForeignState : public dearRule{
private:
	const deAnimatorRuleForeignState &pForeignState;
	int pForeignBone;
	
	dearControllerTarget pTargetPosition;
	dearControllerTarget pTargetOrientation;
	dearControllerTarget pTargetSize;
	
	const bool pEnablePosition;
	const bool pEnableOrientation;
	const bool pEnableSize;
	const deAnimatorRuleForeignState::eCoordinateFrames pSourceCoordFrame;
	const deAnimatorRuleForeignState::eCoordinateFrames pDestCoordFrame;
	const float pScalePosition;
	const float pScaleOrientation;
	const float pScaleSize;
	
public:
	/** \name Constructors and Destructors */
	/*@{*/
	/** \brief Create rule. */
	dearRuleForeignState( dearAnimatorInstance &instance, int firstLink, const deAnimatorRuleForeignState &rule );
	
	/** \brief Clean up rule. */
	virtual ~dearRuleForeignState();
	/*@}*/
	
	
	
	/** \name Management */
	/*@{*/
	/** \brief Foreign bone index. */
	inline int GetForeignBone() const{ return pForeignBone; }
	
	
	
	/** \brief Apply to animator. */
	virtual void Apply( dearBoneStateList &stalist );
	
	/** \brief Rule changed. */
	virtual void RuleChanged();
	/*@}*/
	
private:
	void pUpdateForeignBone();
};

#endif
