/* 
 * Drag[en]gine IGDE World Editor
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

// include only once
#ifndef _MEUHTVRULEMATHVALUEA_H_
#define _MEUHTVRULEMATHVALUEA_H_

// includes
#include <deigde/undo/igdeUndo.h>

// predefinitions
class meHTVegetationLayer;
class meHTVRuleMath;



/**
 * \brief Undo Height Terrain Vegetation Rule Math Set Value A.
 *
 * Undo action to set the A value bound of a height terrain vegetation rule mapping.
 */
class meUHTVRuleMathSetValueA : public igdeUndo{
private:
	meHTVegetationLayer *pVLayer;
	meHTVRuleMath *pRule;
	
	float pOldValueA;
	float pNewValueA;
	
public:
	/** \name Constructors and Destructors */
	/*@{*/
	/** \brief Create object. */
	meUHTVRuleMathSetValueA( meHTVegetationLayer *vlayer, meHTVRuleMath *rule, float nvalue );
	/** \brief Clean up object. */
	virtual ~meUHTVRuleMathSetValueA();
	/*@}*/
	
	/** \name Management */
	/*@{*/
	/** \brief Undo. */
	virtual void Undo();
	/** \brief Redo. */
	virtual void Redo();
	/*@}*/
};

// end of include only once
#endif
