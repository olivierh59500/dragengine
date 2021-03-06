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

#ifndef _INPUTDEVICE_H_
#define _INPUTDEVICE_H_

#include "../deObject.h"
#include "../common/collection/decObjectOrderedSet.h"
#include "../common/string/decString.h"
#include "../resources/image/deImageReference.h"
#include "../resources/model/deModelReference.h"
#include "../resources/skin/deSkinReference.h"

class deInputDeviceButton;
class deInputDeviceAxis;
class deInputDeviceFeedback;


/**
 * \brief Input device installed on the host system.
 * 
 * Input devices provide digital or analog input. Devices can have buttons and axes.
 * 
 * The device identifier uniquely identifies the device across different platforms for
 * the same module. It can be safely used by the scripting module to save input bindings.
 * The identifier composes only of letters, digits and underscore to allow script using
 * them as prefix. The input module should add a unique prefix to the identifier to mark
 * the identifier originating fromt his module in case different input modules are involved.
 * 
 * Devices can have axes, buttons and feedbacks. Axes receive an analog value in the range
 * from -1 to 1 for absolue axes. For relative axes like the mouse only the relative change
 * is returned. Buttons return a pressed or unpressed state. Feedbacks on the other hand
 * allow manipulating a feedback state of the device if present.
 */
class deInputDevice : public deObject{
public:
	/** \brief Device types. */
	enum eDeviceTypes{
		/** \brief Mouse. */
		edtMouse,
		
		/** \brief Keyboard. */
		edtKeyboard,
		
		/** \brief Game pad. */
		edtGamepad,
		
		/** \brief Joystick. */
		edtJoystick,
		
		/** \brief Touchpad. */
		edtTouchpad,
		
		/** \brief Touchscreen. */
		edtTouchscreen,
		
		/** \brief Racing wheel. */
		edtRacingWheel,
		
		/** \brief Generic device. */
		edtGeneric
	};
	
	
	
private:
	/** \brief Device identifier unique across reboots. */
	decString pID;
	
	/** \brief Display name. */
	decString pName;
	
	/** \brief Device type. */
	eDeviceTypes pType;
	
	/** \brief Model to represent the device in 3D user interfaces or NULL if not set. */
	deModelReference pDisplayModel;
	
	/** \brief Skin for display model or NULL if not set. */
	deSkinReference pDisplaySkin;
	
	/**
	 * \brief Image to represent the device in 2D user interfaces or NULL if not set.
	 * 
	 * Large image of 128 pixels squared or larger.
	 */
	deImageReference pDisplayImage;
	
	/** \brief List of small icons of different size for use in binding displays. */
	decObjectOrderedSet pDisplayIcons;
	
	/** \brief Text to display centered across display image or icon. */
	decString pDisplayText;
	
	/** \brief Buttons. */
	deInputDeviceButton *pButtons;
	
	/** \brief Number of buttons. */
	int pButtonCount;
	
	/** \brief Axes. */
	deInputDeviceAxis *pAxes;
	
	/** \brief Number of axes. */
	int pAxisCount;
	
	/** \brief Feedbacks. */
	deInputDeviceFeedback *pFeedbacks;
	
	/** \brief Number of feedbacks. */
	int pFeedbackCount;
	
	
	
public:
	/** \name Constructors and Destructors */
	/*@{*/
	/** \brief Create input device. */
	deInputDevice();
	
protected:
	/** \brief Clean up input device. */
	virtual ~deInputDevice();
	/*@}*/
	
	
	
public:
	/** \name Management */
	/*@{*/
	/**
	 * \brief Device identifier.
	 * 
	 * Unique identifier for the device identifying it across removing and attaching
	 * the device to the host system. The prefix is a normalized string that contains
	 * only letters, numbers and underscores. It is suitable to be combined with
	 * button or axis identifiers to store them in config files as key bindings.
	 */
	inline const decString &GetID() const{ return pID; }
	
	/** \brief Set identifier. */
	void SetID( const char *id );
	
	/**
	 * \brief Display name.
	 * 
	 * Dispaly name is human readable like for example 'USB Gamepad #1'. The
	 * name is unique but not guaranteed to stay the same across restarting
	 * the input module or game engine.
	 */
	inline const decString &GetName() const{ return pName; }
	
	/** \brief Set Display name. */
	void SetName( const char *name );
	
	/** \brief Device type. */
	inline eDeviceTypes GetType() const{ return pType; }
	
	/** \brief Device type. */
	void SetType( eDeviceTypes type );
	
	/** \brief Model to represent the device in 3D user interfaces or NULL if not set. */
	inline deModel *GetDisplayModel() const{ return pDisplayModel; }
	
	/** \brief Set model to represent the device in 3D user interfaces or NULL if not set. */
	void SetDisplayModel( deModel *model );
	
	/** \brief Skin for display model or NULL if not set. */
	inline deSkin *GetDisplaySkin() const{ return pDisplaySkin; }
	
	/** \brief Set skin for display model or NULL if not set. */
	void SetDisplaySkin( deSkin *skin );
	
	/**
	 * \brief Image to represent the device in 2D user interfaces or NULL if not set.
	 * 
	 * Large image of 128 pixels squared or larger.
	 */
	inline deImage *GetDisplayImage() const{ return pDisplayImage; }
	
	/**
	 * \brief Set image to represent the device in 2D user interfaces or NULL if not set.
	 * 
	 * Large image of 128 pixels squared or larger.
	 */
	void SetDisplayImage( deImage *image );
	
	/** \brief Count of icons representing the device in bindings. */
	int GetDisplayIconCount() const;
	
	/**
	 * \brief Icon at index representing the device in bindings.
	 * 
	 * Icon is of square size and typically has a size of 16, 24, 32 or 64.
	 */
	deImage *GetDisplayIconAt( int index ) const;
	
	/**
	 * \brief Add icon representing the device in bindings.
	 * 
	 * Icon is of square size and typically has a size of 16, 24, 32 or 64.
	 */
	void AddDisplayIcon( deImage *image );
	
	/** \brief Text to display centered across display image or icon. */
	inline const decString &GetDisplayText() const{ return pDisplayText; }
	
	/** \brief Set text to display centered across display image or icon. */
	void SetDisplayText( const char *text );
	/*@}*/
	
	
	
	/** \name Buttons */
	/*@{*/
	/** \brief Number of buttons. */
	inline int GetButtonCount() const{ return pButtonCount; }
	
	/**
	 * \brief Set number of buttons.
	 * 
	 * Resets all buttons to default values.
	 */
	void SetButtonCount( int count );
	
	/** \brief Button at index. */
	deInputDeviceButton &GetButtonAt( int index ) const;
	
	/** \brief Index of button with identifier or -1 if not found. */
	int IndexOfButtonWithID( const char *id ) const;
	/*@}*/
	
	
	
	/** \name Axes */
	/*@{*/
	/** \brief Number of axes. */
	inline int GetAxisCount() const{ return pAxisCount; }
	
	/**
	 * \brief Set number of axes.
	 * 
	 * Resets all axes to default values.
	 */
	void SetAxisCount( int count );
	
	/** \brief Axis at index. */
	deInputDeviceAxis &GetAxisAt( int index ) const;
	
	/** \brief Index of axis with identifier or -1 if not found. */
	int IndexOfAxisWithID( const char *id ) const;
	/*@}*/
	
	
	
	/** \name Feedbacks */
	/*@{*/
	/** \brief Number of feedbacks. */
	inline int GetFeedbackCount() const{ return pFeedbackCount; }
	
	/**
	 * \brief Set number of feedbacks.
	 * 
	 * Resets all feedbacks to default values.
	 */
	void SetFeedbackCount( int count );
	
	/** \brief Feedback at index. */
	deInputDeviceFeedback &GetFeedbackAt( int index ) const;
	
	/** \brief Index of feedback with identifier or -1 if not found. */
	int IndexOfFeedbackWithID( const char *id ) const;
	/*@}*/
	
	
	
private:
	void pCleanUp();
};

#endif
