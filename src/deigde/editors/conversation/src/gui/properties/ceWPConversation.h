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

#ifndef _CEWPCONVERSATION_H_
#define _CEWPCONVERSATION_H_

#include <deigde/gui/igdeButtonReference.h>
#include <deigde/gui/igdeCheckBoxReference.h>
#include <deigde/gui/igdeComboBoxReference.h>
#include <deigde/gui/igdeComboBoxFilterReference.h>
#include <deigde/gui/igdeTextFieldReference.h>
#include <deigde/gui/composed/igdeEditVectorReference.h>
#include <deigde/gui/layout/igdeContainerScroll.h>

class ceWindowProperties;
class ceWPConversationListener;
class ceConversation;
class cePose;
class ceGesture;
class ceFacePose;
class ceCameraShot;
class ceTarget;
class ceLookAt;
class ceControllerValue;



/**
 * \brief Conversation property window.
 */
class ceWPConversation : public igdeContainerScroll{
private:
	ceWindowProperties &pWindowProperties;
	ceWPConversationListener *pListener;
	ceConversation *pConversation;
	
	igdeComboBoxFilterReference pCBPose;
	igdeButtonReference pBtnPose;
	igdeComboBoxFilterReference pCBPoseMove;
	
	igdeComboBoxFilterReference pCBGesture;
	igdeButtonReference pBtnGesture;
	igdeTextFieldReference pEditGestureAnimator;
	igdeCheckBoxReference pChkGestureHold;
	
	igdeComboBoxFilterReference pCBFacePose;
	igdeButtonReference pBtnFacePose;
	igdeComboBoxReference pCBFPController;
	igdeTextFieldReference pEditFPControllerValue;
	igdeButtonReference pBtnFPController;
	
	igdeComboBoxFilterReference pCBTarget;
	igdeButtonReference pBtnTarget;
	igdeComboBoxFilterReference pCBTargetActorID;
	igdeTextFieldReference pEditTargetEntityID;
	igdeTextFieldReference pEditTargetBone;
	igdeEditVectorReference pEditTargetPosition;
	igdeEditVectorReference pEditTargetOrientation;
	
	igdeComboBoxFilterReference pCBCameraShot;
	igdeButtonReference pBtnCameraShot;
	igdeTextFieldReference pEditCShotActorCount;
	igdeComboBoxFilterReference pCBCameraShotCameraTarget;
	igdeEditVectorReference pEditCShotOffCamFrom;
	igdeEditVectorReference pEditCShotOffCamTo;
	igdeComboBoxFilterReference pCBCameraShotLookAtTarget;
	igdeEditVectorReference pEditCShotOffLookAtFrom;
	igdeEditVectorReference pEditCShotOffLookAtTo;
	igdeEditVectorReference pEditCShotCamOrbitFrom;
	igdeEditVectorReference pEditCShotCamOrbitTo;
	igdeTextFieldReference pEditCShotCamDistFrom;
	igdeTextFieldReference pEditCShotCamDistTo;
	igdeEditVectorReference pEditCShotPosFrom;
	igdeEditVectorReference pEditCShotPosTo;
	igdeEditVectorReference pEditCShotRotFrom;
	igdeEditVectorReference pEditCShotRotTo;
	igdeTextFieldReference pEditCShotTiltFrom;
	igdeTextFieldReference pEditCShotTiltTo;
	igdeTextFieldReference pEditCShotFovFrom;
	igdeTextFieldReference pEditCShotFovTo;
	igdeCheckBoxReference pChkCShotLockUpAxis;
	igdeCheckBoxReference pChkCShotAlignTargets;
	igdeCheckBoxReference pChkCShotRelativeToLookAt;
	igdeCheckBoxReference pChkCShotLockCameraTarget;
	igdeCheckBoxReference pChkCShotLockLookAtTarget;
	
	igdeComboBoxFilterReference pCBLookAt;
	igdeButtonReference pBtnLookAt;
	igdeComboBoxFilterReference pCBLookAtTarget;
	
	
	
public:
	/** \name Constructors and Destructors */
	/*@{*/
	/** \brief Create panel. */
	ceWPConversation( ceWindowProperties &windowProperties );
	
protected:
	/** \brief Clean up panel. */
	virtual ~ceWPConversation();
	/*@}*/
	
	
	
public:
	/** \name Management */
	/*@{*/
	/** \brief Properties windows. */
	inline ceWindowProperties &GetWindowProperties() const{ return pWindowProperties; }
	
	/** \brief Conversation. */
	inline ceConversation *GetConversation() const{ return pConversation; }
	
	/** \brief Set conversation. */
	void SetConversation( ceConversation *conversation );
	
	/** \brief Update conversation. */
	void UpdateConversation();
	
	
	
	/** \brief Active pose. */
	cePose *GetPose() const;
	
	/** \brief Update list of poses. */
	void UpdatePoseList();
	
	/** \brief Select active pose. */
	void SelectActivePose();
	
	/** \brief Update active pose. */
	void UpdatePose();
	
	
	
	/** \brief Active gesture. */
	ceGesture *GetGesture() const;
	
	/** \brief Update list of gestures. */
	void UpdateGestureList();
	
	/** \brief Select active gesture. */
	void SelectActiveGesture();
	
	/** \brief Update active gesture. */
	void UpdateGesture();
	
	
	
	/** \brief Active face pose. */
	ceFacePose *GetFacePose() const;
	
	/** \brief Update list of face poses. */
	void UpdateFacePoseList();
	
	/** \brief Select active face pose. */
	void SelectActiveFacePose();
	
	/** \brief Update active face pose. */
	void UpdateFacePose();
	
	/** \brief Active controller value. */
	ceControllerValue *GetFacePoseController() const;
	
	/** \brief Update list of face pose controllers. */
	void UpdateFPControllerList();
	
	/** \brief Select face pose controller. */
	void SelectFacePoseController( ceControllerValue *controller );
	
	/** \brief Update the active face pose controller. */
	void UpdateFPController();
	
	
	
	/** \brief Active target. */
	ceTarget *GetTarget() const;
	
	/** \brief Update list of targets. */
	void UpdateTargetList();
	
	/** \brief Select active target. */
	void SelectActiveTarget();
	
	/** \brief Update active target. */
	void UpdateTarget();
	
	
	
	/** \brief Active camera shot. */
	ceCameraShot *GetCameraShot() const;
	
	/** \brief Update list of camera shots. */
	void UpdateCameraShotList();
	
	/** \brief Select active camera shot. */
	void SelectActiveCameraShot();
	
	/** \brief Update active camera shot. */
	void UpdateCameraShot();
	
	
	
	/** \brief Active lookat. */
	ceLookAt *GetLookAt() const;
	
	/** \brief Update the list of lookats. */
	void UpdateLookAtList();
	
	/** \brief Select active lookat. */
	void SelectActiveLookAt();
	
	/** \brief Update active lookat. */
	void UpdateLookAt();
	
	/** \brief Update actor id lists. */
	void UpdateActorIDLists();
	/*@}*/
};

#endif
