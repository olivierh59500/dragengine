/* 
 * Drag[en]gine OpenGL Graphic Module
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
#include <string.h>

#include "deoglSkinState.h"
#include "deoglSkinStateRenderable.h"
#include "deoglSkinStateCalculated.h"
#include "../deoglRSkin.h"
#include "../deoglSkinRenderable.h"
#include "../dynamic/deoglRDynamicSkin.h"
#include "../dynamic/renderables/render/deoglRDSRenderable.h"
#include "../../billboard/deoglRBillboard.h"
#include "../../component/deoglRComponent.h"
#include "../../component/deoglRComponentTexture.h"
#include "../../decal/deoglRDecal.h"
#include "../../light/deoglRLight.h"
#include "../../rendering/plan/deoglRenderPlan.h"
#include "../../rendering/plan/deoglRenderPlanMasked.h"
#include "../../renderthread/deoglRenderThread.h"
#include "../../texture/pixelbuffer/deoglPixelBuffer.h"
#include "../../texture/texture2d/deoglTexture.h"
#include "../../video/deoglRVideoPlayer.h"

#include <dragengine/deEngine.h>
#include <dragengine/common/exceptions.h>



// Class deoglSkinState
/////////////////////////

// Constructor, destructor
////////////////////////////

deoglSkinState::deoglSkinState( deoglRenderThread &renderThread ) :
pRenderThread( renderThread )
{
	pSharedInit();
}

deoglSkinState::deoglSkinState( deoglRenderThread &renderThread, deoglRComponent &component, int texture ) :
pRenderThread( renderThread )
{
	pSharedInit();
	pOwnerComponent = &component;
	pOwnerComponentTexture = texture;
}

deoglSkinState::deoglSkinState( deoglRenderThread &renderThread, deoglRBillboard &billboard ) :
pRenderThread( renderThread )
{
	pSharedInit();
	pOwnerBillboard = &billboard;
}

deoglSkinState::deoglSkinState( deoglRenderThread &renderThread, deoglRDecal &decal ) :
pRenderThread( renderThread )
{
	pSharedInit();
	pOwnerDecal = &decal;
}

deoglSkinState::deoglSkinState( deoglRenderThread &renderThread, deoglRLight &light ) :
pRenderThread( renderThread )
{
	pSharedInit();
	pOwnerLight = &light;
}

deoglSkinState::~deoglSkinState(){
	SetVideoPlayerCount( 0 );
	
	RemoveAllRenderables();
	if( pRenderables ){
		delete [] pRenderables;
	}
	
	SetCalculatedPropertyCount( 0 );
}



// Management
///////////////

deoglRSkin *deoglSkinState::GetOwnerSkin() const{
	if( pOwnerComponent ){
		if( pOwnerComponentTexture != -1 ){
			return pOwnerComponent->GetTextureAt( pOwnerComponentTexture ).GetSkin();
			
		}else{
			return pOwnerComponent->GetSkin();
		}
		
	}else if( pOwnerBillboard ){
		return pOwnerBillboard->GetSkin();
		
	}else if( pOwnerDecal ){
		return pOwnerDecal->GetSkin();
		
	}else if( pOwnerLight ){
		return pOwnerLight->GetLightSkin();
		
	}else{
		return NULL;
	}
}

deoglRDynamicSkin *deoglSkinState::GetOwnerDynamicSkin() const{
	if( pOwnerComponent ){
		if( pOwnerComponentTexture != -1 ){
			deoglRDynamicSkin * const dynamicSkin =
				pOwnerComponent->GetTextureAt( pOwnerComponentTexture ).GetDynamicSkin();
			return dynamicSkin ? dynamicSkin : pOwnerComponent->GetDynamicSkin();
			
		}else{
			return pOwnerComponent->GetDynamicSkin();
		}
		
	}else if( pOwnerBillboard ){
		return pOwnerBillboard->GetDynamicSkin();
		
	}else if( pOwnerDecal ){
		return pOwnerDecal->GetUseDynamicSkin();
		
	}else if( pOwnerLight ){
		return pOwnerLight->GetDynamicSkin();
		
	}else{
		return NULL;
	}
}



void deoglSkinState::SetTime( float time ){
	pTime = time;
}

void deoglSkinState::AdvanceTime( float timeStep ){
	pTime += timeStep;
}

void deoglSkinState::SetUpdateNumber( int updateNumber ){
	pUpdateNumber = updateNumber;
}



void deoglSkinState::DropDelayedDeletionObjects(){
	pVideoPlayers.RemoveAll();
	
	int i;
	for( i=0; i<pRenderableCount; i++ ){
		pRenderables[ i ]->DropDelayedDeletionObjects();
	}
	
	SetCalculatedPropertyCount( 0 );
}



int deoglSkinState::GetVideoPlayerCount() const{
	return pVideoPlayers.GetCount();
}

void deoglSkinState::SetVideoPlayerCount( int count ){
	if( count == pVideoPlayers.GetCount() ){
		return;
	}
	
	pVideoPlayers.RemoveAll();
	
	int i;
	for( i=0; i<count; i++ ){
		pVideoPlayers.Add( NULL );
	}
}

deoglRVideoPlayer *deoglSkinState::GetVideoPlayerAt( int index ) const{
	return ( deoglRVideoPlayer* )pVideoPlayers.GetAt( index );
}

void deoglSkinState::SetVideoPlayerAt( int index, deoglRVideoPlayer *videoPlayer ){
	pVideoPlayers.SetAt( index, videoPlayer );
}



void deoglSkinState::PrepareRenderables( deoglRSkin *skin, deoglRDynamicSkin *dynamicSkin ){
	const int videoPlayerCount = pVideoPlayers.GetCount();
	int i;
	for( i=0; i< videoPlayerCount; i++ ){
		deoglRVideoPlayer * const videoPlayer = ( deoglRVideoPlayer* )pVideoPlayers.GetAt( i );
		if( videoPlayer ){
			videoPlayer->UpdateTexture();
		}
	}
	
	for( i=0; i<pRenderableCount; i++ ){
		deoglSkinStateRenderable &skinStateRenderable = *pRenderables[ i ];
		skinStateRenderable.Clear();
		
		const int hostIndex = skinStateRenderable.GetHostRenderable();
		
		if( hostIndex != -1 && dynamicSkin ){
			dynamicSkin->GetRenderableAt( hostIndex )->PrepareForRender();
		}
	}
	
	for( i=0; i<pCalculatedPropertyCount; i++ ){
		pCalculatedProperties[ i ].Update( *this );
	}
}

void deoglSkinState::AddRenderPlans( deoglRenderPlan &plan ){
	// HACK: preventing a dead loop. has to be made better
	if( plan.GetLevel() > 0 ){
		return;
	}
	
	// TODO: currently only the mirror related plans are properly added.
	// renderable related plans are wrapped by a dirty-skin test and can be
	// skipped the second time.
	// required to verify if this is an issue or not ( and acting accordingly ).
	int i;
	for( i=0; i<pRenderableCount; i++ ){
		pRenderables[ i ]->AddRenderPlans( plan );
	}
}



// Renderables
////////////////

deoglSkinStateRenderable *deoglSkinState::GetRenderableAt( int index ) const{
	if( index < 0 || index >= pRenderableCount ){
		DETHROW( deeInvalidParam );
	}
	
	return pRenderables[ index ];
}

deoglSkinStateRenderable *deoglSkinState::AddRenderable(){
	deoglSkinStateRenderable * const renderable = new deoglSkinStateRenderable( *this, pRenderableCount );
	
	if( pRenderableCount == pRenderableSize ){
		int newSize = pRenderableSize * 3 / 2 + 1;
		deoglSkinStateRenderable **newArray = new deoglSkinStateRenderable*[ newSize ];
		if( ! newArray ) DETHROW( deeOutOfMemory );
		if( pRenderables ){
			memcpy( newArray, pRenderables, sizeof( deoglSkinStateRenderable* ) * pRenderableSize );
			delete [] pRenderables;
		}
		pRenderables = newArray;
		pRenderableSize = newSize;
	}
	
	pRenderables[ pRenderableCount ] = renderable;
	pRenderableCount++;
	
	return renderable;
}

void deoglSkinState::AddRenderables( deoglRSkin &skin, deoglRDynamicSkin &dynamicSkin ){
	const int count = skin.GetRenderableCount();
	int i;
	
	for( i=0; i<count; i++ ){
		deoglSkinStateRenderable * const renderable = AddRenderable();
		renderable->SetHostRenderable( dynamicSkin.IndexOfRenderableNamed( skin.GetRenderableAt( i ).GetName() ) );
	}
}

void deoglSkinState::RemoveAllRenderables(){
	while( pRenderableCount > 0 ){
		pRenderableCount--;
		delete pRenderables[ pRenderableCount ];
	}
}



// Calculated properties
//////////////////////////

deoglSkinStateCalculated &deoglSkinState::GetCalculatedPropertyAt( int index ) const{
	if( index < 0 || index >= pCalculatedPropertyCount ){
		DETHROW( deeInvalidParam );
	}
	return pCalculatedProperties[ index ];
}

void deoglSkinState::SetCalculatedPropertyCount( int count ){
	if( count < 0 ){
		DETHROW( deeInvalidParam );
	}
	
	if( pCalculatedProperties ){
		delete [] pCalculatedProperties;
		pCalculatedProperties = NULL;
		pCalculatedPropertyCount = 0;
	}
	
	if( count > 0 ){
		pCalculatedProperties = new deoglSkinStateCalculated[ count ];
		pCalculatedPropertyCount = count;
	}
}

void deoglSkinState::InitCalculatedProperties(){
	deoglRSkin * const skin = GetOwnerSkin();
	if( ! skin || skin->GetCalculatedPropertyCount() == 0 ){
		SetCalculatedPropertyCount( 0 );
		return;
	}
	
	SetCalculatedPropertyCount( skin->GetCalculatedPropertyCount() );
	
	int i;
	for( i=0; i<pCalculatedPropertyCount; i++ ){
		pCalculatedProperties[ i ].SetProperty( skin->GetCalculatedPropertyAt( i ) );
		pCalculatedProperties[ i ].SetComponent( pOwnerComponent );
	}
}

void deoglSkinState::CalculatedPropertiesMapBones( const deComponent &component ){
	int i;
	for( i=0; i<pCalculatedPropertyCount; i++ ){
		pCalculatedProperties[ i ].MapBones( component );
	}
}

void deoglSkinState::UpdateCalculatedPropertiesBones( const deComponent &component ){
	int i;
	for( i=0; i<pCalculatedPropertyCount; i++ ){
		pCalculatedProperties[ i ].UpdateBones( component );
	}
}



void deoglSkinState::SetVariationSeed( const decPoint &seed ){
	pVariationSeed = seed;
}



// Private Functions
//////////////////////

void deoglSkinState::pSharedInit(){
	pOwnerComponent = NULL;
	pOwnerComponentTexture = 0;
	pOwnerBillboard = NULL;
	
	pTime = 0.0f;
	
	pRenderables = NULL;
	pRenderableCount = 0;
	pRenderableSize = 0;
	
	pCalculatedProperties = NULL;
	pCalculatedPropertyCount = 0;
	
	pVariationSeed.x = ( int )( ( ( float )rand() / ( float )RAND_MAX ) * 100.0f );
	pVariationSeed.y = ( int )( ( ( float )rand() / ( float )RAND_MAX ) * 100.0f );
	
	pUpdateNumber = 0;
}
