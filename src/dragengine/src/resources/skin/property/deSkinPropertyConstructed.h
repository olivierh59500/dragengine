/* 
 * Drag[en]gine Game Engine
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

#ifndef _DESKINPROPERTYCONSTRUCTED_H_
#define _DESKINPROPERTYCONSTRUCTED_H_

#include "../../../common/math/decMath.h"
#include "deSkinProperty.h"

class deSkinPropertyNodeGroup;


/**
 * \brief Skin texture constructed property.
 */
class deSkinPropertyConstructed : public deSkinProperty{
private:
	deSkinPropertyNodeGroup *pContent;
	decColor pColor;
	bool pTileX;
	bool pTileY;
	
	
	
public:
	/** \name Constructors and Destructors */
	/*@{*/
	/**
	 * \brief Create constructed property.
	 * \param type Type of the skin property obtained from adding a property
	 * name to the texture property map object held by the engine.
	 */
	deSkinPropertyConstructed( const char *type );
	
	/** \brief Clean up constructed property. */
	virtual ~deSkinPropertyConstructed();
	/*@}*/
	
	
	
	/** \name Management */
	/*@{*/
	/** \brief Content. */
	inline deSkinPropertyNodeGroup &GetContent() const{ return *pContent; }
	
	/** \brief Base color. */
	inline const decColor &GetColor() const{ return pColor; }
	
	/** \brief Set base color. */
	void SetColor( const decColor &color );
	
	/** \brief Nodes are tiled along x axis. */
	inline bool GetTileX() const{ return pTileX; }
	
	/** \brief Set if nodes are tiled along x axis. */
	void SetTileX( bool tileX );
	
	/** \brief Nodes are tiled along y axis. */
	inline bool GetTileY() const{ return pTileY; }
	
	/** \brief Set if nodes are tiled along y axis. */
	void SetTileY( bool tileY );
	/*@}*/
	
	
	
	/** \name Visiting */
	/*@{*/
	/** \brief Visit property. */
	virtual void Visit( deSkinPropertyVisitor &visitor );
	/*@}*/
};

#endif
