/* 
 * Drag[en]gine Console Launcher
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

#ifndef _DECLEMPARAMETERLIST_H_
#define _DECLEMPARAMETERLIST_H_

#include <dragengine/common/collection/decObjectList.h>

class declEMParameter;



/**
 * @brief Engine Parameter Parameter List.
 */
class declEMParameterList{
private:
	decObjectList pParameters;
	
public:
	/** @name Constructors and Destructors */
	/*@{*/
	/** Creates a new parameter list. */
	declEMParameterList();
	/** Cleans up the parameter list. */
	~declEMParameterList();
	/*@}*/
	
	/** @name Management */
	/*@{*/
	/** Retrieves the number of parameters. */
	int GetCount() const;
	/** Retrieves the parameter at the given position. */
	declEMParameter *GetAt( int index ) const;
	/** Retrieves the parameter with the given name or NULL if not found. */
	declEMParameter *GetNamed( const char *name ) const;
	/** Determines if a parameter exists. */
	bool Has( declEMParameter *parameter ) const;
	/** Determines if a parameter with the given name exists. */
	bool HasNamed( const char *name ) const;
	/** Retrieves the index of a parameter or -1 if not found. */
	int IndexOf( declEMParameter *parameter ) const;
	/** Retrieves the index of a parameter with the given name or -1 if not found. */
	int IndexOfNamed( const char *name ) const;
	/** Adds a parameter. */
	void Add( declEMParameter *parameter );
	/** Removes a parameter. */
	void Remove( declEMParameter *parameter );
	/** Removes all parameters. */
	void RemoveAll();
	/*@}*/
};

#endif // _DEGLGAME_H_
