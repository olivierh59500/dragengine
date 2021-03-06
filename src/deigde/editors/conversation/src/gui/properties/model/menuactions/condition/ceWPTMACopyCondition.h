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

#ifndef _CEWPTMACOPYCONDITION_H_
#define _CEWPTMACOPYCONDITION_H_

#include "../ceWPTMenuAction.h"

class ceConversationCondition;
class ceWindowMain;



/**
 * \brief Menu action copy condition.
 */
class ceWPTMACopyCondition : public ceWPTMenuAction{
private:
	ceConversationCondition *pCondition;
	
	
	
protected:
	/** \brief FOX constructor. */
	ceWPTMACopyCondition();
	
public:
	/** \brief Constructors and Destructors */
	/*@{*/
	/** \brief Crete menu action. */
	ceWPTMACopyCondition( ceWindowMain &windowMain,
		ceConversationCondition *condition );
	
	/** \brief Crete menu action. */
	ceWPTMACopyCondition( ceWindowMain &windowMain,
		ceConversationCondition *condition, const char *text, igdeIcon *icon );
	/*@}*/
	
	
	
	/** \brief Management */
	/*@{*/
	/** \brief Action. */
	inline ceConversationCondition *GetCondition() const{ return pCondition; }
	
	/** \brief Do menu action. */
	virtual void OnAction();
	/*@}*/
};

#endif
