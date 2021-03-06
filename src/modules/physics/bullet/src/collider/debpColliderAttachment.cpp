/* 
 * Drag[en]gine Bullet Physics Module
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

#include <stdio.h>
#include <stdlib.h>

#include "debpColliderAttachment.h"
#include "debpCollider.h"

#include <dragengine/resources/collider/deCollider.h>
#include <dragengine/resources/collider/deColliderAttachment.h>
#include <dragengine/resources/component/deComponent.h>
#include <dragengine/resources/deResourceManager.h>
#include <dragengine/resources/billboard/deBillboard.h>
#include <dragengine/resources/camera/deCamera.h>
#include <dragengine/resources/forcefield/deForceField.h>
#include <dragengine/resources/light/deLight.h>
#include <dragengine/resources/debug/deDebugDrawer.h>
#include <dragengine/resources/probe/deEnvMapProbe.h>
#include <dragengine/resources/sensor/deLumimeter.h>
#include <dragengine/resources/sensor/deTouchSensor.h>
#include <dragengine/resources/sensor/deSoundLevelMeter.h>
#include <dragengine/resources/sound/deMicrophone.h>
#include <dragengine/resources/sound/deSpeaker.h>
#include <dragengine/resources/navigation/blocker/deNavigationBlocker.h>
#include <dragengine/resources/navigation/space/deNavigationSpace.h>
#include <dragengine/resources/navigation/navigator/deNavigator.h>
#include <dragengine/resources/particle/deParticleEmitterInstance.h>
#include <dragengine/resources/propfield/dePropField.h>
#include <dragengine/common/exceptions.h>
#include <dragengine/systems/modules/scripting/deBaseScriptingCollider.h>



// Class debpColliderAttachment
/////////////////////////////////

// Constructor, destructor
////////////////////////////

debpColliderAttachment::debpColliderAttachment( deColliderAttachment *attachment ) :
pAttachment( attachment ),
pTrackBone( -1 ),
pBoneMappings( NULL ),
pBoneMappingCount( 0 ),
pDirtyMappings( true ),
pDirtyLocalMatrix( true ),
pHasLocalMatrix( false ){
}

debpColliderAttachment::~debpColliderAttachment(){
	pCleanUp();
}



// Management
///////////////

void debpColliderAttachment::SetBoneMappingCount( int count ){
	if( count < 0 ){
		DETHROW( deeInvalidParam );
	}
	
	if( count == pBoneMappingCount ){
		return;
	}
	
	int i, *newArray = NULL;
	if( count > 0 ){
		newArray = new int[ count ];
		for( i=0; i<count; i++ ){
			newArray[ i ] = -1;
		}
	}
	
	if( pBoneMappings ){
		delete [] pBoneMappings;
	}
	pBoneMappings = newArray;
	pBoneMappingCount = count;
}

void debpColliderAttachment::SetTrackBone( int boneIndex ){
	if( boneIndex < -1 ){
		DETHROW( deeInvalidParam );
	}
	pTrackBone = boneIndex;
}

int debpColliderAttachment::GetBoneMappingAt( int index ) const{
	if( index < 0 || index >= pBoneMappingCount ){
		DETHROW( deeInvalidParam );
	}
	return pBoneMappings[ index ];
}

void debpColliderAttachment::SetBoneMappingAt( int index, int boneIndex ){
	if( index < 0 || index >= pBoneMappingCount ){
		DETHROW( deeInvalidParam );
	}
	if( boneIndex < -1 ){
		DETHROW( deeInvalidParam );
	}
	pBoneMappings[ index ] = boneIndex;
}

void debpColliderAttachment::SetDirtyMappings( bool dirtyMappings ){
	pDirtyMappings = dirtyMappings;
}



void debpColliderAttachment::AttachmentChanged(){
	pDirtyMappings = true;
	pDirtyLocalMatrix = true;
}



void debpColliderAttachment::Reposition( const decDMatrix &matrix, bool changeNotify ){
	pPrepareLocalMatrix();
	
	if( pHasLocalMatrix ){
		const decDMatrix attmat( pLocalMatrix.QuickMultiply( matrix ) );
		pRepositionResource( attmat.GetPosition(), attmat.ToQuaternion(), changeNotify );
		
	}else{
		pRepositionResource( matrix.GetPosition(), matrix.ToQuaternion(), changeNotify );
	}
}

void debpColliderAttachment::Reposition( const decDVector &position, const decQuaternion &orientation, bool changeNotify ){
	pPrepareLocalMatrix();
	
	if( pHasLocalMatrix ){
		const decDMatrix attmat( pLocalMatrix.QuickMultiply( decDMatrix::CreateWorld( position, orientation ) ) );
		pRepositionResource( attmat.GetPosition(), attmat.ToQuaternion(), changeNotify );
		
	}else{
		pRepositionResource( position, orientation, changeNotify );
	}
}

void debpColliderAttachment::Transform( const decDMatrix &matrix, bool changeNotify ){
	deResource * const resource = pAttachment->GetResource();
	
	switch( resource->GetResourceManager()->GetResourceType() ){
	case deResourceManager::ertCollider:{
		deCollider &collider = *( ( deCollider* )resource );
		const decDVector position( matrix * collider.GetPosition() );
		const decQuaternion orientation( collider.GetOrientation() * matrix.ToQuaternion() );
		debpCollider &bpCollider = *( ( debpCollider* )collider.GetPeerPhysics() );
		
		if( changeNotify ){
			if( collider.GetPosition().IsEqualTo( position )
			&& collider.GetOrientation().IsEqualTo( orientation ) ){
				break;
			}
		}
		
		collider.SetGeometry( position, orientation );
		
		if( changeNotify ){
			collider.GetPeerScripting()->ColliderChanged( &collider );
			bpCollider.ClearRequiresUpdate();
			
		}else{
			// notification is prevented. register collider for finish detection.
			// at this time the notification can be resolved
			bpCollider.RegisterColDetFinish();
		}
		}break;
		
	case deResourceManager::ertBillboard:{
		deBillboard &billboard = *( ( deBillboard* )resource );
		billboard.SetPosition( matrix * billboard.GetPosition() );
		}break;
		
	case deResourceManager::ertCamera:{
		deCamera &camera = *( ( deCamera* )resource );
		camera.SetPosition( matrix * camera.GetPosition() );
		camera.SetOrientation( camera.GetOrientation() * matrix.ToQuaternion() );
		}break;
		
	case deResourceManager::ertComponent:{
		deComponent &component = *( ( deComponent* )resource );
		component.SetPosition( matrix * component.GetPosition() );
		component.SetOrientation( component.GetOrientation() * matrix.ToQuaternion() );
		}break;
		
	case deResourceManager::ertDebugDrawer:{
		deDebugDrawer &debugDrawer = *( ( deDebugDrawer* )resource );
		debugDrawer.SetPosition( matrix * debugDrawer.GetPosition() );
		debugDrawer.SetOrientation( debugDrawer.GetOrientation() * matrix.ToQuaternion() );
		}break;
		
	case deResourceManager::ertEnvMapProbe:{
		deEnvMapProbe &envMapProbe = *( ( deEnvMapProbe* )resource );
		envMapProbe.SetPosition( matrix * envMapProbe.GetPosition() );
		envMapProbe.SetOrientation( envMapProbe.GetOrientation() * matrix.ToQuaternion() );
		}break;
		
	case deResourceManager::ertForceField:{
		deForceField &forceField = *( ( deForceField* )resource );
		forceField.SetPosition( matrix * forceField.GetPosition() );
		forceField.SetOrientation( forceField.GetOrientation() * matrix.ToQuaternion() );
		}break;
		
	case deResourceManager::ertLight:{
		deLight &light = *( ( deLight* )resource );
		light.SetPosition( matrix * light.GetPosition() );
		light.SetOrientation( light.GetOrientation() * matrix.ToQuaternion() );
		}break;
		
	case deResourceManager::ertLumimeter:{
		deLumimeter &lumimeter = *( ( deLumimeter* )resource );
		lumimeter.SetPosition( matrix * lumimeter.GetPosition() );
		//lumimeter.SetOrientation( lumimeter.GetOrientation() * matrix.ToQuaternion() ); // TODO missing
		}break;
		
	case deResourceManager::ertMicrophone:{
		deMicrophone &microphone = *( ( deMicrophone* )resource );
		microphone.SetPosition( matrix * microphone.GetPosition() );
		microphone.SetOrientation( microphone.GetOrientation() * matrix.ToQuaternion() );
		}break;
		
	case deResourceManager::ertNavigationSpace:{
		deNavigationSpace &navigationSpace = *( ( deNavigationSpace* )resource );
		navigationSpace.SetPosition( matrix * navigationSpace.GetPosition() );
		navigationSpace.SetOrientation( navigationSpace.GetOrientation() * matrix.ToQuaternion() );
		}break;
		
	case deResourceManager::ertNavigationBlocker:{
		deNavigationBlocker &navigationBlocker = *( ( deNavigationBlocker* )resource );
		navigationBlocker.SetPosition( matrix * navigationBlocker.GetPosition() );
		navigationBlocker.SetOrientation( navigationBlocker.GetOrientation() * matrix.ToQuaternion() );
		}break;
		
	case deResourceManager::ertParticleEmitterInstance:{
		deParticleEmitterInstance &instance = *( ( deParticleEmitterInstance* )resource );
		instance.SetPosition( matrix * instance.GetPosition() );
		instance.SetOrientation( instance.GetOrientation() * matrix.ToQuaternion() );
		}break;
		
	case deResourceManager::ertPropField:{
		dePropField &propField = *( ( dePropField* )resource );
		propField.SetPosition( matrix * propField.GetPosition() );
		//propField.SetOrientation( propfield.GetOrientation() * matrix.ToQuaternion() ); // TODO missing
		}break;
		
	case deResourceManager::ertSpeaker:{
		deSpeaker &speaker = *( ( deSpeaker* )resource );
		speaker.SetPosition( matrix * speaker.GetPosition() );
		speaker.SetOrientation( speaker.GetOrientation() * matrix.ToQuaternion() );
		}break;
		
	case deResourceManager::ertTouchSensor:{
		deTouchSensor &touchSensor = *( ( deTouchSensor* )resource );
		touchSensor.SetPosition( matrix * touchSensor.GetPosition() );
		touchSensor.SetOrientation( touchSensor.GetOrientation() * matrix.ToQuaternion() );
		}break;
		
	case deResourceManager::ertSoundLevelMeter:{
		deSoundLevelMeter &soundLevelMeter = *( ( deSoundLevelMeter* )resource );
		soundLevelMeter.SetPosition( matrix * soundLevelMeter.GetPosition() );
		soundLevelMeter.SetOrientation( soundLevelMeter.GetOrientation() * matrix.ToQuaternion() );
		}break;
		
	default:
		break; // not supported
	}
}



// private functions
//////////////////////

void debpColliderAttachment::pCleanUp(){
	if( pBoneMappings ){
		delete [] pBoneMappings;
	}
}



void debpColliderAttachment::pPrepareLocalMatrix(){
	if( ! pDirtyLocalMatrix ){
		return;
	}
	
	const decQuaternion &localOrientation = pAttachment->GetOrientation();
	const decVector &localPosition = pAttachment->GetPosition();
	
	if( localPosition.IsEqualTo( decVector() ) && localOrientation.IsEqualTo( decQuaternion() ) ){
		pLocalMatrix.SetIdentity();
		pHasLocalMatrix = false;
		
	}else{
		pLocalMatrix.SetWorld( localPosition, localOrientation );
		pHasLocalMatrix = true;
	}
	
	pDirtyLocalMatrix = false;
}

void debpColliderAttachment::pRepositionResource( const decDVector &position,
const decQuaternion &orientation, bool changeNotify ){
	deResource * const resource = pAttachment->GetResource();
	
	switch( resource->GetResourceManager()->GetResourceType() ){
	case deResourceManager::ertCollider:{
		deCollider &collider = *( ( deCollider* )resource );
		debpCollider &bpCollider = *( ( debpCollider* )collider.GetPeerPhysics() );
		
		if( changeNotify ){
			if( collider.GetPosition().IsEqualTo( position )
			&& collider.GetOrientation().IsEqualTo( orientation ) ){
				break;
			}
		}
		
		collider.SetGeometry( position, orientation );
		
		if( changeNotify ){
			collider.GetPeerScripting()->ColliderChanged( &collider );
			bpCollider.ClearRequiresUpdate();
			
		}else{
			// notification is prevented. register collider for finish detection.
			// at this time the notification can be resolved
			bpCollider.RegisterColDetFinish();
		}
		}break;
		
	case deResourceManager::ertBillboard:{
		deBillboard &billboard = *( ( deBillboard* )resource );
		billboard.SetPosition( position );
		}break;
		
	case deResourceManager::ertCamera:{
		deCamera &camera = *( ( deCamera* )resource );
		camera.SetPosition( position );
		camera.SetOrientation( orientation );
		}break;
		
	case deResourceManager::ertComponent:{
		deComponent &component = *( ( deComponent* )resource );
		component.SetPosition( position );
		component.SetOrientation( orientation );
		}break;
		
	case deResourceManager::ertDebugDrawer:{
		deDebugDrawer &debugDrawer = *( ( deDebugDrawer* )resource );
		debugDrawer.SetPosition( position );
		debugDrawer.SetOrientation( orientation );
		}break;
		
	case deResourceManager::ertEnvMapProbe:{
		deEnvMapProbe &envMapProbe = *( ( deEnvMapProbe* )resource );
		envMapProbe.SetPosition( position );
		envMapProbe.SetOrientation( orientation );
		}break;
		
	case deResourceManager::ertForceField:{
		deForceField &forceField = *( ( deForceField* )resource );
		forceField.SetPosition( position );
		forceField.SetOrientation( orientation );
		}break;
		
	case deResourceManager::ertLight:{
		deLight &light = *( ( deLight* )resource );
		light.SetPosition( position );
		light.SetOrientation( orientation );
		}break;
		
	case deResourceManager::ertLumimeter:{
		deLumimeter &lumimeter = *( ( deLumimeter* )resource );
		lumimeter.SetPosition( position );
		//lumimeter.SetOrientation( orientation ); // TODO missing
		}break;
		
	case deResourceManager::ertMicrophone:{
		deMicrophone &microphone = *( ( deMicrophone* )resource );
		microphone.SetPosition( position );
		microphone.SetOrientation( orientation );
		}break;
		
	case deResourceManager::ertNavigationSpace:{
		deNavigationSpace &navigationSpace = *( ( deNavigationSpace* )resource );
		navigationSpace.SetPosition( position );
		navigationSpace.SetOrientation( orientation );
		}break;
		
	case deResourceManager::ertNavigationBlocker:{
		deNavigationBlocker &navigationBlocker = *( ( deNavigationBlocker* )resource );
		navigationBlocker.SetPosition( position );
		navigationBlocker.SetOrientation( orientation );
		}break;
		
	case deResourceManager::ertParticleEmitterInstance:{
		deParticleEmitterInstance &particleEmitterInstance = *( ( deParticleEmitterInstance* )resource );
		particleEmitterInstance.SetPosition( position );
		particleEmitterInstance.SetOrientation( orientation );
		}break;
		
	case deResourceManager::ertPropField:{
		dePropField &propField = *( ( dePropField* )resource );
		propField.SetPosition( position );
		//propField.SetOrientation( orientation ); // TODO missing
		}break;
		
	case deResourceManager::ertSpeaker:{
		deSpeaker &speaker = *( ( deSpeaker* )resource );
		speaker.SetPosition( position );
		speaker.SetOrientation( orientation );
		}break;
		
	case deResourceManager::ertTouchSensor:{
		deTouchSensor &touchSensor = *( ( deTouchSensor* )resource );
		touchSensor.SetPosition( position );
		touchSensor.SetOrientation( orientation );
		}break;
		
	case deResourceManager::ertSoundLevelMeter:{
		deSoundLevelMeter &soundLevelMeter = *( ( deSoundLevelMeter* )resource );
		soundLevelMeter.SetPosition( position );
		soundLevelMeter.SetOrientation( orientation );
		}break;
		
	default:
		break; // not supported
	}
}
