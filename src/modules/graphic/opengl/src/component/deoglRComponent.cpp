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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "deoglComponentNotifier.h"
#include "deoglComponentTestForTouch.h"
#include "deoglRComponent.h"
#include "deoglRComponentTexture.h"
#include "deoglRComponentLOD.h"
#include "../capabilities/deoglCapabilities.h"
#include "../collidelist/deoglCollideList.h"
#include "../collidelist/deoglCollideListManager.h"
#include "../configuration/deoglConfiguration.h"
#include "../decal/deoglDecal.h"
#include "../decal/deoglRDecal.h"
#include "../delayedoperation/deoglDelayedDeletion.h"
#include "../delayedoperation/deoglDelayedOperations.h"
#include "../envmap/deoglEnvironmentMap.h"
#include "../extensions/deoglExtResult.h"
#include "../extensions/deoglExtensions.h"
#include "../light/deoglRLight.h"
#include "../model/deoglModelLOD.h"
#include "../model/deoglRModel.h"
#include "../model/face/deoglModelFace.h"
#include "../occlusiontest/deoglOcclusionTest.h"
#include "../occlusiontest/mesh/deoglDynamicOcclusionMesh.h"
#include "../occlusiontest/mesh/deoglROcclusionMesh.h"
#include "../rendering/defren/deoglDeferredRendering.h"
#include "../rendering/deoglRenderWorld.h"
#include "../rendering/plan/deoglRenderPlan.h"
#include "../rendering/plan/deoglRenderPlanMasked.h"
#include "../renderthread/deoglRTBufferObject.h"
#include "../renderthread/deoglRTChoices.h"
#include "../renderthread/deoglRTLogger.h"
#include "../renderthread/deoglRenderThread.h"
#include "../shaders/paramblock/deoglSPBParameter.h"
#include "../shaders/paramblock/deoglSPBlockUBO.h"
#include "../shaders/paramblock/shared/deoglSharedSPBElement.h"
#include "../shaders/paramblock/shared/deoglSharedSPBListUBO.h"
#include "../shaders/paramblock/shared/deoglSharedSPBRTIGroup.h"
#include "../skin/channel/deoglSkinChannel.h"
#include "../skin/deoglRSkin.h"
#include "../skin/deoglSkinRenderable.h"
#include "../skin/deoglSkinTexture.h"
#include "../skin/dynamic/deoglRDynamicSkin.h"
#include "../skin/dynamic/renderables/render/deoglRDSRenderable.h"
#include "../skin/shader/deoglSkinShader.h"
#include "../skin/state/deoglSkinState.h"
#include "../skin/state/deoglSkinStateRenderable.h"
#include "../sky/deoglRSkyInstance.h"
#include "../target/deoglRenderTarget.h"
#include "../utils/deoglCubeHelper.h"
#include "../vbo/deoglSharedVBO.h"
#include "../vbo/deoglSharedVBOBlock.h"
#include "../vbo/deoglSharedVBOList.h"
#include "../vbo/deoglVBOAttribute.h"
#include "../vbo/deoglVBOLayout.h"
#include "../visitors/deoglFindBestEnvMap.h"
#include "../world/deoglRCamera.h"
#include "../world/deoglRWorld.h"
#include "../world/deoglWorldOctree.h"
#include "../utils/collision/deoglDCollisionSphere.h"
#include "../utils/collision/deoglDCollisionBox.h"
#include "../utils/collision/deoglCollisionDetection.h"

#include <dragengine/common/exceptions.h>
#include <dragengine/resources/component/deComponent.h>
#include <dragengine/resources/component/deComponentBone.h>
#include <dragengine/resources/decal/deDecal.h>
#include <dragengine/resources/model/deModel.h>
#include <dragengine/resources/model/deModelBone.h>
#include <dragengine/resources/rig/deRig.h>
#include <dragengine/resources/rig/deRigBone.h>

#ifdef OS_W32
#undef near
#undef far
#endif



// Class deoglRComponent
/////////////////////////

// Constructor, destructor
////////////////////////////

deoglRComponent::deoglRComponent( deoglRenderThread &renderThread ) :
pRenderThread( renderThread ),

pParentWorld( NULL ),
pOctreeNode( NULL ),

pVisible( true ),
pMovementHint( deComponent::emhStationary ),

pOccMeshSharedSPBElement( NULL ),
pDirtyOccMeshSharedSPBElement( true ),
pOccMeshSharedSPBDoubleSided( NULL ),
pOccMeshSharedSPBSingleSided( NULL ),

pParamBlockSpecial( NULL ),
pSpecialFlags( 0 ),

pSkinRendered( renderThread, *this ),

pWorldMarkedRemove( false ),
pLLWorldPrev( NULL ),
pLLWorldNext( NULL )
{
	pLODErrorScaling = 1.0f;
	
	pModel = NULL;
	pSkin = NULL;
	pDynamicSkin = NULL;
	pOcclusionMesh = NULL;
	pDynamicOcclusionMesh = NULL;
	
	pParamBlockOccMesh = NULL;
	pDirtyParamBlockOccMesh = true;
	
	pSkinState = NULL;
	pRenderVisible = true;
	
	pFirstRender = true;
	pRenderStatic = true;
	pStaticSince = 0.0f;
	
	pBoneMatrices = NULL;
	pBoneMatrixCount = 0;
	
	pRenderMode = ermStatic;
	
	pDirtyRenderables = true;
	pDirtyTextureUseSkin = true;
	pDirtyModelRigMappings = true;
	
	pSolid = true;
	pOutlineSolid = true;
	
	pMarked = false;
	
	pDirtyCulling = true;
	
	pRenderEnvMap = NULL;
	pRenderEnvMapFade = NULL;
	pRenderEnvMapFadePerTime = 1.0f;
	pRenderEnvMapFadeFactor = 1.0f;
	pDirtyRenderEnvMap = true;
	
	pNotifierIndex = 0;
	
	pEnvMap = NULL;
	
	int i;
	for( i=0; i<6; i++ ){
		pCubeFaceVisible[ i ] = true;
	}
	
	try{
		pSkinState = new deoglSkinState( renderThread, *this );
		
	}catch( const deException & ){
		pCleanUp();
		throw;
	}
	LEAK_CHECK_CREATE( renderThread, Component );
}

deoglRComponent::~deoglRComponent(){
	LEAK_CHECK_FREE( pRenderThread, Component );
	pCleanUp();
}



// Management
///////////////

void deoglRComponent::SetParentWorld( deoglRWorld *parentWorld ){
	if( parentWorld == pParentWorld ){
		return;
	}
	
	InvalidateRenderEnvMap();
	pSkinRendered.DropDelayedDeletionObjects();
	
	/*if( pEnvMap ){
		pEnvMap->SetWorld( parentWorld );
	}*/
	
	pParentWorld = parentWorld;
	
	pRemoveFromAllLights();
	
	if( pOctreeNode ){
		pOctreeNode->RemoveComponent( this );
	}
	
	pDirtyRenderEnvMap = true;
	pFirstRender = true;
}



void deoglRComponent::SetOctreeNode( deoglWorldOctree *octreeNode ){
	pOctreeNode = octreeNode;
}

void deoglRComponent::UpdateOctreeNode(){
	if( ! pParentWorld ){
		return;
	}
	
	NotifyLightsDirtyLightVolume();
	
	pRemoveFromAllLights();
	
	// insert into parent world octree
	if( pVisible && pModel ){
		pParentWorld->GetOctree().InsertComponentIntoTree( this, 8 );
		
		// visit the world for touching lights
		deoglComponentTestForTouch testForTouching( this );
		pParentWorld->VisitRegion( pMinExtend, pMaxExtend, testForTouching );
		
	}else{
		if( pOctreeNode ){
			pOctreeNode->RemoveComponent( this );
		}
	}
}



void deoglRComponent::SetVisible( bool visible ){
	pVisible = visible;
}

void deoglRComponent::SetMovementHint( deComponent::eMovementHints hint ){
	pMovementHint = hint;
}

void deoglRComponent::SetLayerMask( const decLayerMask &layerMask ){
	if( layerMask == pLayerMask ){
		return;
	}
	
	pLayerMask = layerMask;
	
	// light shadow matching potentially changed
	const deoglLightList list( pLightList );
	const int count = list.GetCount();
	int i;
	
	for( i=0; i<count; i++ ){
		list.GetAt( i )->TestComponent( this );
	}
}



void deoglRComponent::SetModel( deoglRModel *model ){
	if( model == pModel ){
		return;
	}
	
	if( pModel ){
		pModel->FreeReference();
	}
	pModel = model;
	if( model ){
		model->AddReference();
	}
	
	InvalidateVAO();
	
	pDirtyModelRigMappings = true;
	MarkTextureUseSkinDirty();
	
	if( true ){ // if not using world based shared SPB
		MarkAllTexturesParamBlocksDirty();
	}
	
	pResizeModelSkinMappings();
	pUpdateModelSkinMappings();
	pResizeModelRigMappings();
	pResizeBoneMatrices();
	pUpdateRenderMode();
	NotifyBoundariesChanged();
}

void deoglRComponent::SetSkin( deoglRSkin *skin ){
	if( skin == pSkin ){
		return;
	}
	
	if( pSkin ){
		pSkin->FreeReference();
	}
	pSkin = skin;
	if( skin ){
		skin->AddReference();
	}
	
	pUpdateModelSkinMappings();
	
	pSkinRendered.SetDirty();
}

void deoglRComponent::RigChanged(){
	InvalidateVAO();
	
	pDirtyModelRigMappings = true;
	
	pUpdateRenderMode();
	pResizeBoneMatrices();
	pResizeModelRigMappings();
	NotifyBoundariesChanged();
	MarkAllTexturesParamBlocksDirty();
	MarkAllDecalTexturesParamBlocksDirty();
	MarkOccMeshParamBlockDirty();
}

void deoglRComponent::SetDynamicSkin( deoglRDynamicSkin *dynamicSkin ){
	// NOTE this is called from the main thread during synchronization
	if( dynamicSkin == pDynamicSkin ){
		return;
	}
	
	if( pDynamicSkin ){
		pDynamicSkin->FreeReference();
	}
	
	pDynamicSkin = dynamicSkin;
	
	if( dynamicSkin ){
		dynamicSkin->AddReference();
	}
	
	// texture can use the dynamic skin we had so far for their skin state.
	// force an update to make sure everything matches up again
	const int textureCount = pTextures.GetCount();
	int i;
	for( i=0; i<textureCount; i++ ){
		deoglRComponentTexture &texture = *( ( deoglRComponentTexture* )pTextures.GetAt( i ) );
		texture.SetSkinState( NULL ); // required since UpdateSkinState can not figure out dynamic skin changed
		texture.UpdateSkinState();
	}
	
	// child decals can use the dynamic skin we had so far for their skin state.
	// force an update to make sure everything matches up again
	const int decalCount = pDecals.GetCount();
	for( i=0; i<decalCount; i++ ){
		deoglRDecal &decal = *( ( deoglRDecal* )pDecals.GetAt( i ) );
		decal.SetSkinState( NULL ); // required since UpdateSkinState can not figure out dynamic skin changed
		decal.UpdateSkinState();
	}
	
	pSkinRendered.SetDirty();
}

void deoglRComponent::SetOcclusionMesh( deoglROcclusionMesh *occlusionMesh ){
	if( occlusionMesh == pOcclusionMesh ){
		return;
	}
	
	if( pDynamicOcclusionMesh ){
		delete pDynamicOcclusionMesh;
		pDynamicOcclusionMesh = NULL;
	}
	if( pOccMeshSharedSPBDoubleSided ){
		pOccMeshSharedSPBDoubleSided->FreeReference();
		pOccMeshSharedSPBDoubleSided = NULL;
	}
	if( pOccMeshSharedSPBSingleSided ){
		pOccMeshSharedSPBSingleSided->FreeReference();
		pOccMeshSharedSPBSingleSided = NULL;
	}
	if( pOccMeshSharedSPBElement ){
		pOccMeshSharedSPBElement->FreeReference();
		pOccMeshSharedSPBElement = NULL;
	}
	if( pOcclusionMesh ){
		pOcclusionMesh->FreeReference();
	}
	
	pOcclusionMesh = occlusionMesh;
	
	if( occlusionMesh ){
		occlusionMesh->AddReference();
	}
	
	if( occlusionMesh ){
		if( occlusionMesh->GetWeightsCount() > 0 ){
			pDynamicOcclusionMesh = new deoglDynamicOcclusionMesh( pRenderThread, pOcclusionMesh, this );
			
			// update all touching lights since their light volume potentially changes. this works even with
			// lights storing only components in their reduced extends. if a component has a new occlusion
			// mesh (or a larger one) that did not have it before the light volume can at best reduce but
			// not expand. if on the other hand the component has now no occlusion mesh (or a smaller one)
			// than before the light volumes of the affected lights can potentially expand. this works since
			// lights visit the entire full extends upon rebuilding the light volume. this catches then also
			// component occlusion meshes not included yet in their list
		//	pDirtyOctree = true;
		//	ResetStatic();
		}
	}
}

deoglSharedSPBElement *deoglRComponent::GetOccMeshSharedSPBElement(){
	if( ! pOccMeshSharedSPBElement ){
		if( pRenderThread.GetChoices().GetSharedSPBUseSSBO() ){
			pOccMeshSharedSPBElement = pRenderThread.GetBufferObject()
				.GetSharedSPBList( deoglRTBufferObject::esspblOccMeshInstanceSSBO ).AddElement();
			
		}else{
			if( pRenderThread.GetChoices().GetGlobalSharedSPBLists() ){
				pOccMeshSharedSPBElement = pRenderThread.GetBufferObject()
					.GetSharedSPBList( deoglRTBufferObject::esspblOccMeshInstanceUBO ).AddElement();
				
			}else if( pOcclusionMesh ){
				pOccMeshSharedSPBElement = pOcclusionMesh->GetSharedSPBListUBO().AddElement();
			}
		}
	}
	
	if( pDirtyOccMeshSharedSPBElement ){
		if( pOccMeshSharedSPBElement ){
			deoglShaderParameterBlock &paramBlock = pOccMeshSharedSPBElement->MapBuffer();
			try{
				UpdateOccmeshInstanceParamBlock( paramBlock, pOccMeshSharedSPBElement->GetIndex() );
				
			}catch( const deException & ){
				paramBlock.UnmapBuffer();
				throw;
			}
			
			paramBlock.UnmapBuffer();
		}
		
		pDirtyOccMeshSharedSPBElement = false;
	}
	
	return pOccMeshSharedSPBElement;
}

deoglSharedSPBRTIGroup &deoglRComponent::GetOccMeshSharedSPBRTIGroup( bool doubleSided ){
	if( doubleSided ){
		if( pOccMeshSharedSPBDoubleSided ){
			return *pOccMeshSharedSPBDoubleSided;
		}
		
	}else{
		if( pOccMeshSharedSPBSingleSided ){
			return *pOccMeshSharedSPBSingleSided;
		}
	}
	
	if( ! pOcclusionMesh ){
		DETHROW( deeInvalidParam );
	}
	
	deoglSharedSPBElement * const element = GetOccMeshSharedSPBElement();
	
	if( doubleSided ){
		pOccMeshSharedSPBDoubleSided = pOcclusionMesh->GetRTIGroupDouble().GetWith( element->GetSPB() );
		return *pOccMeshSharedSPBDoubleSided;
		
	}else{
		pOccMeshSharedSPBSingleSided = pOcclusionMesh->GetRTIGroupsSingle().GetWith( element->GetSPB() );
		return *pOccMeshSharedSPBSingleSided;
	}
}

void deoglRComponent::MeshChanged(){
	pInvalidateLODVBOs();
	
	if( pDynamicOcclusionMesh ){
		pDynamicOcclusionMesh->ComponentStateChanged();
	}
}



void deoglRComponent::InitSkinStateCalculatedProperties( const deComponent &component ){
	pSkinState->InitCalculatedProperties();
	pSkinState->CalculatedPropertiesMapBones( component );
	
	const int textureCount = pTextures.GetCount();
	int i;
	for( i=0; i<textureCount; i++ ){
		deoglRComponentTexture &texture = *( ( deoglRComponentTexture* )pTextures.GetAt( i ) );
		if( texture.GetSkinState() ){
			texture.GetSkinState()->InitCalculatedProperties();
			texture.GetSkinState()->CalculatedPropertiesMapBones( component );
		}
	}
}

void deoglRComponent::UpdateSkinStateCalculatedPropertiesBones( const deComponent &component ){
	pSkinState->UpdateCalculatedPropertiesBones( component );
	
	const int textureCount = pTextures.GetCount();
	int i;
	for( i=0; i<textureCount; i++ ){
		deoglRComponentTexture &texture = *( ( deoglRComponentTexture* )pTextures.GetAt( i ) );
		if( texture.GetSkinState() ){
			texture.GetSkinState()->UpdateCalculatedPropertiesBones( component );
		}
	}
}



deoglSPBlockUBO *deoglRComponent::GetParamBlockOccMesh(){
	if( ! pParamBlockOccMesh ){
		try{
			pParamBlockOccMesh = new deoglSPBlockUBO( pRenderThread );
			pParamBlockOccMesh->SetRowMajor( ! pRenderThread.GetCapabilities().GetUBOIndirectMatrixAccess().Broken() );
			pParamBlockOccMesh->SetParameterCount( 1 );
			pParamBlockOccMesh->GetParameterAt( 0 ).SetAll( deoglSPBParameter::evtFloat, 4, 3, 1 ); // mat4x3 pMatrixModel
			
			pParamBlockOccMesh->MapToStd140();
			pParamBlockOccMesh->SetBindingPoint( deoglSkinShader::eubInstanceParameters );
			
		}catch( const deException & ){
			if( pParamBlockOccMesh ){
				pParamBlockOccMesh->FreeReference();
			}
			pParamBlockOccMesh = NULL;
			throw;
		}
	}
	
	if( pDirtyParamBlockOccMesh ){
		if( pParamBlockOccMesh ){
			pParamBlockOccMesh->MapBuffer();
			try{
				UpdateOccmeshInstanceParamBlock( *pParamBlockOccMesh, 0 );
				
			}catch( const deException & ){
				pParamBlockOccMesh->UnmapBuffer();
				throw;
			}
			pParamBlockOccMesh->UnmapBuffer();
		}
		pDirtyParamBlockOccMesh = false;
	}
	
	return pParamBlockOccMesh;
}

void deoglRComponent::MarkOccMeshParamBlockDirty(){
	pDirtyParamBlockOccMesh = true;
	pDirtyOccMeshSharedSPBElement = true;
}

void deoglRComponent::UpdateOccmeshInstanceParamBlock( deoglShaderParameterBlock &paramBlock,
int element ){
	if( ! pParentWorld ){
		DETHROW( deeInvalidParam );
	}
	
	const decDVector &referencePosition = pParentWorld->GetReferencePosition();
	decDMatrix matrix( GetMatrix() );
	matrix.a14 -= referencePosition.x;
	matrix.a24 -= referencePosition.y;
	matrix.a34 -= referencePosition.z;
	
	paramBlock.SetParameterDataMat4x3( 0, element, matrix ); // pMatrixModel
}



deoglSPBlockUBO *deoglRComponent::GetParamBlockSpecial(){
	if( ! pParamBlockSpecial ){
		pParamBlockSpecial = deoglSkinShader::CreateSPBSpecial( pRenderThread );
	}
	return pParamBlockSpecial;
}

void deoglRComponent::UpdateCubeFaceVisibility( const decDVector &cubePosition ){
	deoglCubeHelper::CalcFaceVisibility(
		pMinExtend - cubePosition, pMaxExtend - cubePosition, pCubeFaceVisible );
	
	// DEBUG
	/*
	pRenderThread.GetLogger().LogInfoFormat( "DEBUG: (%g,%g,%g) [%d, %d, %d, %d, %d, %d] {(%g,%g,%g), (%g,%g,%g)}",
		pMatrix.GetPosition().x, pMatrix.GetPosition().y, pMatrix.GetPosition().z,
		    pCubeFaceVisible[0], pCubeFaceVisible[1], pCubeFaceVisible[2],
		    pCubeFaceVisible[3], pCubeFaceVisible[4], pCubeFaceVisible[5],
		    (pMinExtend - cubePosition).x, (pMinExtend - cubePosition).y, (pMinExtend - cubePosition).z,
		    (pMaxExtend - cubePosition).x, (pMaxExtend - cubePosition).y, (pMaxExtend - cubePosition).z );
	*/
}

bool deoglRComponent::GetCubeFaceVisible( int cubeFace ) const{
	if( cubeFace < 0 || cubeFace > 5 ){
		DETHROW( deeInvalidParam );
	}
	return pCubeFaceVisible[ cubeFace ];
}

void deoglRComponent::SetSpecialFlagsFromFaceVisibility(){
	pSpecialFlags = 0;
	if( pCubeFaceVisible[ 0 ] ){
		pSpecialFlags |= 0x1;
	}
	if( pCubeFaceVisible[ 1 ] ){
		pSpecialFlags |= 0x2;
	}
	if( pCubeFaceVisible[ 2 ] ){
		pSpecialFlags |= 0x8;
	}
	if( pCubeFaceVisible[ 3 ] ){
		pSpecialFlags |= 0x4;
	}
	if( pCubeFaceVisible[ 4 ] ){
		pSpecialFlags |= 0x10;
	}
	if( pCubeFaceVisible[ 5 ] ){
		pSpecialFlags |= 0x20;
	}
}

void deoglRComponent::UpdateSpecialSPBCubeRender(){
	deoglSPBlockUBO &spb = *GetParamBlockSpecial();
	
	SetSpecialFlagsFromFaceVisibility();
	
	spb.MapBuffer();
	try{
		spb.SetParameterDataInt( deoglSkinShader::esutCubeFaceVisible, pSpecialFlags );
		
	}catch( const deException & ){
		spb.UnmapBuffer();
		throw;
	}
	spb.UnmapBuffer();
}



void deoglRComponent::UpdateBoneMatrices( deComponent &component ){
	const deRig * const rig = component.GetRig();
	int i;
	
	pUpdateModelRigMappings( component );
	component.PrepareBones();
	
	for( i=0; i<pBoneMatrixCount; i++ ){
		oglMatrix3x4 &boneMatrix = pBoneMatrices[ i ];
		const int bone = pModelRigMappings[ i ];
		
		if( bone == -1 ){
			boneMatrix.a11 = 1.0f;
			boneMatrix.a12 = 0.0f;
			boneMatrix.a13 = 0.0f;
			boneMatrix.a14 = 0.0f;
			boneMatrix.a21 = 0.0f;
			boneMatrix.a22 = 1.0f;
			boneMatrix.a23 = 0.0f;
			boneMatrix.a24 = 0.0f;
			boneMatrix.a31 = 0.0f;
			boneMatrix.a32 = 0.0f;
			boneMatrix.a33 = 1.0f;
			boneMatrix.a34 = 0.0f;
			
		}else{
			const decMatrix matrix( rig->GetBoneAt( bone ).GetInverseMatrix()
				.QuickMultiply( component.GetBoneAt( bone ).GetMatrix() ) );
			
			boneMatrix.a11 = matrix.a11;
			boneMatrix.a12 = matrix.a12;
			boneMatrix.a13 = matrix.a13;
			boneMatrix.a14 = matrix.a14;
			boneMatrix.a21 = matrix.a21;
			boneMatrix.a22 = matrix.a22;
			boneMatrix.a23 = matrix.a23;
			boneMatrix.a24 = matrix.a24;
			boneMatrix.a31 = matrix.a31;
			boneMatrix.a32 = matrix.a32;
			boneMatrix.a33 = matrix.a33;
			boneMatrix.a34 = matrix.a34;
		}
	}
}



int deoglRComponent::GetPointOffset( int lodLevel ) const{
	deoglRComponentLOD &lod = GetLODAt( lodLevel );
	
	if( lod.GetVAO() ){
		return lod.GetPointOffset();
	}
	
	if( ! pModel ){
		DETHROW( deeInvalidParam );
	}
	return pModel->GetLODAt( lodLevel ).GetVBOBlock()->GetOffset();
}

int deoglRComponent::GetIndexOffset( int lodLevel ) const{
	deoglRComponentLOD &lod = GetLODAt( lodLevel );
	
	if( lod.GetVAO() ){
		return lod.GetIndexOffset();
	}
	
	if( ! pModel ){
		DETHROW( deeInvalidParam );
	}
	return pModel->GetLODAt( lodLevel ).GetVBOBlock()->GetIndexOffset();
}

deoglVAO *deoglRComponent::GetVAO( int lodLevel ){
	deoglRComponentLOD &lod = GetLODAt( lodLevel );
	
	if( lod.GetVAO() ){
		return lod.GetVAO();
	}
	
	if( ! pModel ){
		DETHROW( deeInvalidParam );
	}
	return pModel->GetLODAt( lodLevel ).GetVBOBlock()->GetVBO()->GetVAO();
}

void deoglRComponent::InvalidateVAO(){
	const int count = pLODs.GetCount();
	int i;
	
	for( i=0; i<count; i++ ){
		( ( deoglRComponentLOD* )pLODs.GetAt( i ) )->InvalidateVAO();
	}
}



void deoglRComponent::UpdateSkin( float elapsed ){
	// WARNING called from main thread during synchronization time
	
	const int count = pTextures.GetCount();
	int i;
	
	// advance time and mark tucs/parameters dirty if dynamic channels are present
	pSkinState->AdvanceTime( elapsed );
	
	for( i=0; i<count; i++ ){
		//deoglRComponentTexture &texture = *( ( deoglRComponentTexture* )pTextures.GetAt( i ) );
		deoglRComponentTexture &texture = GetTextureAt( i );
		
		if( texture.GetSkinState() ){
			texture.GetSkinState()->AdvanceTime( elapsed );
		}
		
		if( texture.GetUseSkinTexture() ){
			if( texture.GetUseSkinTexture()->GetDynamicChannels() ){
				texture.MarkParamBlocksDirty();
				texture.MarkTUCsDirty();
				
			}else if( texture.GetUseSkinTexture()->GetCalculatedProperties() ){
				texture.MarkParamBlocksDirty();
			}
		}
	}
	
	// update environment map fading
	if( pRenderEnvMapFade ){
		pRenderEnvMapFadeFactor += pRenderEnvMapFadePerTime * elapsed;
		
		if( pRenderEnvMapFadeFactor >= 1.0f ){
			SetRenderEnvMapFade( NULL );
			pRenderEnvMapFadeFactor = 1.0f;
		}
	}
	
	// update skin state in all attached decals
	const int decalCount = pDecals.GetCount();
	for( i=0; i<decalCount; i++ ){
		( ( deoglRDecal* )pDecals.GetAt( i ) )->UpdateSkin( elapsed );
	}
}



//#define DO_TIMING

#ifdef DO_TIMING
#include <dragengine/common/utils/decTimer.h>
static decTimer timer;

float elapsedCompCalcVBO;
float elapsedCompBuildVBO;
float elapsedCompWeights;
float elapsedCompTVert;
float elapsedCompNorTan;
float elapsedCompNormalize;

void debugCompReset(){
	elapsedCompCalcVBO = 0.0f;
	elapsedCompBuildVBO = 0.0f;
	elapsedCompWeights = 0.0f;
	elapsedCompTVert = 0.0f;
	elapsedCompNorTan = 0.0f;
	elapsedCompNormalize = 0.0f;
}
#endif



//#include <dragengine/common/utils/decTimer.h>
//static decTimer timer;

void deoglRComponent::Update( float elapsed ){
	if( pFirstRender ){
		pFirstRender = false;
		pStaticSince = 0.0f;
		SetRenderStatic( pMovementHint == deComponent::emhStationary );
	}
	
	pStaticSince += elapsed;
	if( pStaticSince > 1.0f ){
		SetRenderStatic( true );
	}
}

void deoglRComponent::UpdateVBO(){
	int lodCount = 0;
	int i;
	
	if( pModel ){
		lodCount = pModel->GetLODCount();
		for( i=0; i<lodCount; i++ ){
			pModel->GetLODAt( i ).GetVBOBlock()->Prepare(); // Prepare is deprecated, GetVBOBlock is not!
		}
	}
	
	if( pRenderMode == ermDynamic ){
		for( i=0; i<lodCount; i++ ){
			( ( deoglRComponentLOD* )pLODs.GetAt( i ) )->UpdateVBO();
		}
		
	}else{
		for( i=0; i<lodCount; i++ ){
			( ( deoglRComponentLOD* )pLODs.GetAt( i ) )->FreeVBO();
		}
	}
	
	const int decalCount = pDecals.GetCount();
	for( i=0; i<decalCount; i++ ){
		( ( deoglRDecal* )pDecals.GetAt( i ) )->UpdateVBO();
	}
	
	// update render env map if required
	UpdateRenderEnvMap();
}



void deoglRComponent::SetDirtyRendereables(){
	pDirtyRenderables = true;
}

void deoglRComponent::UpdateRenderables( deoglRenderPlan &plan ){
	int i;
	
	// update render modifiers
	pCheckRenderModifier( plan.GetCamera() );
	
	// update renderables. this is required here as this call sets up properly the
	// skin state if not done already
	pUpdateRenderables();
	
	// make sure all textures are updated and create the render info if required
	pSkinState->PrepareRenderables( pSkin, pDynamicSkin );
	const int textureCount = pTextures.GetCount();
	for( i=0; i<textureCount; i++ ){
		( ( deoglRComponentTexture* )pTextures.GetAt( i ) )->PrepareSkinStateRenderables();
	}
	
	// determine if the component has any transparent faces
	pUpdateSolid();
	
	// update renderables of all attached decals
	const int decalCount = pDecals.GetCount();
	for( i=0; i<decalCount; i++ ){
		( ( deoglRDecal* )pDecals.GetAt( i ) )->UpdateRenderables( plan );
	}
}

void deoglRComponent::AddSkinStateRenderPlans( deoglRenderPlan &plan ){
	pSkinState->AddRenderPlans( plan );
	
	int i;
	for( i=0; i<pTextures.GetCount(); i++ ){
		deoglRComponentTexture &texture = *( ( deoglRComponentTexture* )pTextures.GetAt( i ) );
		if( texture.GetSkinState() ){
			texture.GetSkinState()->AddRenderPlans( plan );
		}
	}
	
	pSkinRendered.AddRenderPlans( plan );
}



void deoglRComponent::SetMatrix( const decDMatrix &matrix ){
	pMatrix = matrix;
	pInverseMatrix = matrix.QuickInvert();
	
	MarkAllTexturesParamBlocksDirty();
	MarkAllDecalTexturesParamBlocksDirty();
	MarkOccMeshParamBlockDirty();
	
	NotifyBoundariesChanged();
	
	/*if( pEnvMap ){
		pEnvMap->SetPosition( pComponent->GetPosition() );
	}*/
}

void deoglRComponent::UpdateExtends( deComponent &component ){
	if( pModel ){
		const int boneCount = component.GetBoneCount();
		decDVector corners[ 8 ];
		int i, j;
		
		if( boneCount > 0 ){
			component.PrepareBones();
			const decDMatrix &matrix = component.GetMatrix();
			bool extendsSet = false;
			
			if( pModel->GetHasWeightlessExtends() ){
				corners[7].Set( pModel->GetWeightlessExtends().minimum );
				corners[1].Set( pModel->GetWeightlessExtends().maximum );
				corners[0].Set( corners[7].x, corners[1].y, corners[1].z );
				corners[2].Set( corners[1].x, corners[7].y, corners[1].z );
				corners[3].Set( corners[7].x, corners[7].y, corners[1].z );
				corners[4].Set( corners[7].x, corners[1].y, corners[7].z );
				corners[5].Set( corners[1].x, corners[1].y, corners[7].z );
				corners[6].Set( corners[1].x, corners[7].y, corners[7].z );
				for( j=0; j<8; j++ ){
					corners[ j ] = matrix * corners[ j ];
					
					if( extendsSet ){
						pMinExtend.SetSmallest( corners[ j ] );
						pMaxExtend.SetLargest( corners[ j ] );
						
					}else{
						pMinExtend = pMaxExtend = corners[ j ];
						extendsSet = true;
					}
				}
			}
			
			for( i=0; i<pModelRigMappings.GetCount(); i++ ){
				if( pModelRigMappings.GetAt( i ) == -1 ){
					continue;
				}
				
				const deoglRModel::sExtends &boneExtends = pModel->GetBoneExtends()[ i ];
				const decDMatrix boneMatrix( decDMatrix( component.GetBoneAt(
					pModelRigMappings.GetAt( i ) ).GetMatrix() ) * matrix );
				
				corners[7].Set( boneExtends.minimum );
				corners[1].Set( boneExtends.maximum );
				corners[0].Set( corners[7].x, corners[1].y, corners[1].z );
				corners[2].Set( corners[1].x, corners[7].y, corners[1].z );
				corners[3].Set( corners[7].x, corners[7].y, corners[1].z );
				corners[4].Set( corners[7].x, corners[1].y, corners[7].z );
				corners[5].Set( corners[1].x, corners[1].y, corners[7].z );
				corners[6].Set( corners[1].x, corners[7].y, corners[7].z );
				
				for( j=0; j<8; j++ ){
					corners[ j ] = boneMatrix * corners[ j ];
					
					if( extendsSet ){
						pMinExtend.SetSmallest( corners[ j ] );
						pMaxExtend.SetLargest( corners[ j ] );
						
					}else{
						pMinExtend = pMaxExtend = corners[ j ];
						extendsSet = true;
					}
				}
			}
			
		}else{
			const decDMatrix &matrix = component.GetMatrix();
			corners[7] = decDVector( pModel->GetExtends().minimum );
			corners[1] = decDVector( pModel->GetExtends().maximum );
			corners[0].Set( corners[7].x, corners[1].y, corners[1].z );
			corners[2].Set( corners[1].x, corners[7].y, corners[1].z );
			corners[3].Set( corners[7].x, corners[7].y, corners[1].z );
			corners[4].Set( corners[7].x, corners[1].y, corners[7].z );
			corners[5].Set( corners[1].x, corners[1].y, corners[7].z );
			corners[6].Set( corners[1].x, corners[7].y, corners[7].z );
			for( j=0; j<8; j++ ){
				corners[ j ] = matrix * corners[ j ];
			}
			
			pMinExtend = pMaxExtend = corners[ 0 ];
			for( j=1; j<8; j++ ){
				pMinExtend.SetSmallest( corners[ j ] );
				pMaxExtend.SetLargest( corners[ j ] );
			}
		}
	}
	
	const decDVector position( pMatrix.GetPosition() );
	pLocalMinExtend = ( pMinExtend - position ).ToVector();
	pLocalMaxExtend = ( pMaxExtend - position ).ToVector();
	
	NotifyBoundariesChanged();
}



void deoglRComponent::SetSortDistance( float distance ){
	pSortDistance = distance;
}



void deoglRComponent::SetLit( bool lit ){
	pLit = lit;
}

void deoglRComponent::SetOccluded( bool occluded ){
	pOccluded = occluded;
}

void deoglRComponent::TestCameraInside( const decDVector &position ){
	const decVector relPos( position - pMatrix.GetPosition() );
	const decVector minExtend( pLocalMinExtend - decVector( 0.1f, 0.1f, 0.1f ) );
	const decVector maxExtend( pLocalMaxExtend + decVector( 0.1f, 0.1f, 0.1f ) );
	pCameraInside = ( relPos >= minExtend && relPos <= maxExtend );
}



void deoglRComponent::SetRenderVisible( bool visible ){
	pRenderVisible = visible;
}

void deoglRComponent::SetRenderStatic( bool isStatic ){
	if( isStatic == pRenderStatic ){
		return;
	}
	
	pRenderStatic = isStatic;
	
	const int count = pLightList.GetCount();
	int i;
	
	for( i=0; i<count; i++ ){
		deoglRLight &light = *pLightList.GetAt( i );
		light.RemoveComponent( this );
		light.AddComponent( this );
	}
}

void deoglRComponent::ResetRenderStatic(){
	pStaticSince = 0.0f;
	SetRenderStatic( false );
}



void deoglRComponent::SetSkyShadowSplitMask( int mask ){
	pSkyShadowSplitMask = mask;
}



void deoglRComponent::SetRenderEnvMap( deoglEnvironmentMap *envmap ){
	// note about the switch process. we have to wait setting the fading environment map until the
	// new environment map has been set. if this is not done the SetRenderEnvMapFade function tries
	// to add the component to the component list of the same environment map as the current one
	// resulting in an exception. to avoid this the fade environment map is first cleared, then
	// the environment set and then the fade environment map is set again
	// 
	// another note. if a new environment map is set while the fading is still in progress a hard
	// switch may still occur. a possible solution would be to delay the switch until the fading
	// is finished. for this we would have to keep the dirty flag set, which is currently set
	// outside somewhere
	if( envmap == pRenderEnvMap ){
		return;
	}
	
	deoglEnvironmentMap * const prevEnvMap = pRenderEnvMap;
	if( prevEnvMap ){
		prevEnvMap->AddReference(); // guard reference
	}
	
	try{
		if( pRenderEnvMap ){
			pRenderEnvMap->GetComponentList().RemoveIfExisting( this );
			pRenderEnvMap->FreeReference();
		}
		
		pRenderEnvMap = envmap;
		
		if( envmap ){
			envmap->AddReference();
			envmap->GetComponentList().Add( this );
		}
		
		// now it is safe to set the fade env map
		SetRenderEnvMapFade( prevEnvMap );
		pRenderEnvMapFadeFactor = 0.0f;
		
		if( ! prevEnvMap ){ // in case SetRenderEnvMapFade did not mark all textures dirty yet
			MarkAllTexturesTUCsDirty();
		}
		
	}catch( const deException & ){
		if( prevEnvMap ){
			prevEnvMap->FreeReference(); // drop guard reference
		}
		throw;
	}
	
	if( prevEnvMap ){
		prevEnvMap->FreeReference(); // drop guard reference
	}
}

void deoglRComponent::SetRenderEnvMapFade( deoglEnvironmentMap *envmap ){
	if( envmap == pRenderEnvMapFade ){
		return;
	}
	
	if( pRenderEnvMapFade ){
		pRenderEnvMapFade->GetComponentList().RemoveIfExisting( this );
		pRenderEnvMapFade->FreeReference();
	}
	
	pRenderEnvMapFade = envmap;
	
	if( envmap ){
		envmap->AddReference();
		envmap->GetComponentList().Add( this );
	}
	
	MarkAllTexturesTUCsDirty();
}

void deoglRComponent::SetRenderEnvMapFadePerTime( float fadePerTime ){
	if( fadePerTime < 0.1f ){
		pRenderEnvMapFadePerTime = 0.1f;
		
	}else{
		pRenderEnvMapFadePerTime = fadePerTime;
	}
}

void deoglRComponent::SetRenderEnvMapFadeFactor( float factor ){
	if( factor < 0.0f ){
		factor = 0.0f;
		
	}else if( factor > 1.0f ){
		factor = 1.0f;
	}
	
	if( fabsf( factor - pRenderEnvMapFadeFactor ) > 0.001f ){
		pRenderEnvMapFadeFactor = factor;
		
	}
}

void deoglRComponent::WorldEnvMapLayoutChanged(){
	pDirtyRenderEnvMap = true;
}

void deoglRComponent::UpdateRenderEnvMap(){
	if( ! pDirtyRenderEnvMap ){
		return;
	}
	
	if( ! pParentWorld ){
		DETHROW( deeInvalidParam );
	}
	
	// for the time being we simply pick the environment map that is closest to the component position.
	// this can lead to wrong picks and harshly switching environment maps but this is enough for the
	// first test.
	// 
	// for the time being we choose no region but visit all environment maps existing in the world.
	// a more optimal solution would be to search in an area somewhat larger than the distance to the
	// currently used environment map. the new environment map has to be closer than the current one
	// to be better. if no environment map exists yet a full search could be done.
	// 
	// better solutions could be using grid like connection between environment maps. this way the next
	// best environment map can be searched by following the grid.
	// 
	// for the time being the center of the bounding box is used as the reference point. for large
	// components we've got the problem of what environment map to use. this though has to be solved
	// later on most probably by splitting up the component into smaller components.
	deoglFindBestEnvMap visitor;
	decDVector position;
	
	position = ( pMinExtend + pMaxExtend ) * 0.5;
	
	visitor.SetPosition( position );
	//pParentWorld->VisitRegion( pMinExtend, pMaxExtend, visitor );
	visitor.VisitList( pParentWorld->GetEnvMapList() );
	
	if( visitor.GetEnvMap() ){
		SetRenderEnvMap( visitor.GetEnvMap() );
		
	}else if( pParentWorld->GetSkyEnvironmentMap() ){
		SetRenderEnvMap( pParentWorld->GetSkyEnvironmentMap() );
		
	}else{
		SetRenderEnvMap( NULL );
		SetRenderEnvMapFade( NULL );
		pRenderEnvMapFadeFactor = 1.0f;
	}
	//pOgl->LogInfoFormat( "update component %p render env map %p\n", pComponent, pRenderEnvMap );
	
	pDirtyRenderEnvMap = false;
}

void deoglRComponent::InvalidateRenderEnvMap(){
	if( ! pRenderEnvMap && ! pRenderEnvMapFade ){
		return;
	}
	
	SetRenderEnvMap( NULL );
	SetRenderEnvMapFade( NULL );
	pDirtyRenderEnvMap = true;
}

void deoglRComponent::InvalidateRenderEnvMapIf( deoglEnvironmentMap *envmap ){
	if( pRenderEnvMap == envmap || pRenderEnvMapFade == envmap ){
		InvalidateRenderEnvMap();
	}
}



void deoglRComponent::WorldReferencePointChanged(){
	MarkAllTexturesParamBlocksDirty();
	MarkAllDecalTexturesParamBlocksDirty();
	MarkOccMeshParamBlockDirty();
}



void deoglRComponent::PrepareQuickDispose(){
	pParentWorld = NULL;
	pOctreeNode = NULL;
	
	pLightList.RemoveAll();
	
	const int count = pDecals.GetCount();
	int i;
	for( i=0; i<count; i++ ){
		( ( deoglRDecal* )pDecals.GetAt( i ) )->PrepareQuickDispose();
	}
	pDecals.RemoveAll();
	
	pNotifiers.RemoveAll();
}



// LODs
/////////

int deoglRComponent::GetLODCount() const{
	return pLODs.GetCount();
}

deoglRComponentLOD &deoglRComponent::GetLODAt( int index ) const{
	return *( ( deoglRComponentLOD* )pLODs.GetAt( index ) );
}

void deoglRComponent::RemoveAllLODs(){
	pLODs.RemoveAll();
}

void deoglRComponent::AddLOD( deoglRComponentLOD *lod ){
	pLODs.Add( lod );
}

void deoglRComponent::SetLODErrorScaling( float errorScaling ){
	pLODErrorScaling = errorScaling;
}



// Textures
/////////////

int deoglRComponent::GetTextureCount() const{
	return pTextures.GetCount();
}

deoglRComponentTexture &deoglRComponent::GetTextureAt( int index ){
	if( pDirtyTextureUseSkin ){
		const int count = pTextures.GetCount();
		int i;
		
		for( i=0; i<count; i++ ){
			( ( deoglRComponentTexture* )pTextures.GetAt( i ) )->UpdateUseSkin();
		}
		
		pDirtyTextureUseSkin = false;
	}
	
	return *( ( deoglRComponentTexture* )pTextures.GetAt( index ) );
}

void deoglRComponent::RemoveAllTextures(){
	pTextures.RemoveAll();
}

void deoglRComponent::AddTexture( deoglRComponentTexture *texture ){
	pTextures.Add( texture );
}

void deoglRComponent::MarkTextureUseSkinDirty(){
	pDirtyTextureUseSkin = true;
	pSkinRendered.SetDirty();
}

void deoglRComponent::InvalidateAllTexturesParamBlocks(){
	const int count = pTextures.GetCount();
	int i;
	
	for( i=0; i<count; i++ ){
		( ( deoglRComponentTexture* )pTextures.GetAt( i ) )->InvalidateParamBlocks();
	}
}

void deoglRComponent::MarkAllTexturesParamBlocksDirty(){
	const int count = pTextures.GetCount();
	int i;
	
	for( i=0; i<count; i++ ){
		( ( deoglRComponentTexture* )pTextures.GetAt( i ) )->MarkParamBlocksDirty();
	}
}

void deoglRComponent::MarkAllTexturesTUCsDirty(){
	const int count = pTextures.GetCount();
	int i;
	
	for( i=0; i<count; i++ ){
		( ( deoglRComponentTexture* )pTextures.GetAt( i ) )->MarkTUCsDirty();
	}
}



// Decals
///////////

int deoglRComponent::GetDecalCount() const{
	return pDecals.GetCount();
}

deoglRDecal *deoglRComponent::GetDecalAt( int index ){
	return ( deoglRDecal* )pDecals.GetAt( index );
}

void deoglRComponent::SyncDecalReferences( const deComponent &engComponent ){
	// clear parent component on all decals marked removed. this invalidates decals and
	// forces rebuilding the decal should they be added somewhere else
	const int count = pDecals.GetCount();
	int i;
	
	for( i=0; i<count; i++ ){
		deoglRDecal &decal = *( ( deoglRDecal* )pDecals.GetAt( i ) );
		if( decal.GetComponentMarkedRemove() ){
			decal.SetParentComponent( NULL );
		}
	}
	
	// clear the list and add decals from component. sets the parent component on all decals.
	// this will trigger a rebuild of the decal only if the parent component changed
	pDecals.RemoveAll();
	
	deDecal *engDecal = engComponent.GetRootDecal();
	while( engDecal ){
		deoglRDecal * const decal = ( ( deoglDecal* )engDecal->GetPeerGraphic() )->GetRDecal();
		pDecals.Add( decal );
		decal->SetComponentMarkedRemove( false );
		decal->SetParentComponent( this );
		engDecal = engDecal->GetLLComponentNext();
	}
}

void deoglRComponent::MarkAllDecalTexturesParamBlocksDirty(){
	const int count = pDecals.GetCount();
	int i;
	
	for( i=0; i<count; i++ ){
		( ( deoglRDecal* )pDecals.GetAt( i ) )->MarkParamBlocksDirty();
	}
}



// Culling
////////////

const decDVector &deoglRComponent::GetCullSphereCenter(){
	if( pDirtyCulling ){
		pUpdateCullSphere();
		pDirtyCulling = false;
	}
	
	return pCullSphereCenter;
}

float deoglRComponent::GetCullSphereRadius(){
	if( pDirtyCulling ){
		pUpdateCullSphere();
		pDirtyCulling = false;
	}
	
	return pCullSphereRadius;
}

void deoglRComponent::SetDirtyCulling(){
	pDirtyCulling = true;
}



void deoglRComponent::StartOcclusionTest( const decDVector &cameraPosition ){
	if( ! pModel || ! pSkin ){
		return;
	}
	/*
	if( pRenderMode == ermDynamic ){
		if( pComponent->GetBoneCount() > 8 ){
			return;
		}
	}
	*/
	
	const decVector minExtend = ( pMinExtend - cameraPosition ).ToVector();
	const decVector maxExtend = ( pMaxExtend - cameraPosition ).ToVector();
	
	pRenderThread.GetOcclusionTest().AddInputData( minExtend, maxExtend, this );
}

void deoglRComponent::OcclusionTestInvisible(){
	pRenderVisible = false;
}



// Notifiers
//////////////

void deoglRComponent::AddNotifier( deoglComponentNotifier *notifier ){
	if( ! notifier ){
		DETHROW( deeInvalidParam );
	}
	pNotifiers.Add( notifier );
}

void deoglRComponent::RemoveNotifier( deoglComponentNotifier *notifier ){
	const int index = pNotifiers.IndexOf( notifier );
	if( index == -1 ){
		return;
	}
	
	pNotifiers.Remove( notifier );
	
	if( pNotifierIndex >= index ){
		pNotifierIndex--;
	}
}

void deoglRComponent::NotifyBoundariesChanged(){
	pNotifierIndex = 0;
	while( pNotifierIndex < pNotifiers.GetCount() ){
		( ( deoglComponentNotifier* )pNotifiers.GetAt( pNotifierIndex ) )->BoundariesChanged();
		pNotifierIndex++;
	}
}

void deoglRComponent::NotifyComponentDestroyed(){
	pNotifierIndex = 0;
	while( pNotifierIndex < pNotifiers.GetCount() ){
		( ( deoglComponentNotifier* )pNotifiers.GetAt( pNotifierIndex ) )->ComponentDestroyed();
		pNotifierIndex++;
	}
}

void deoglRComponent::NotifyLightsDirtyLightVolume(){
	if( ! pRenderStatic || ! pOcclusionMesh ){
		return;
	}
	
	const int lightCount = pLightList.GetCount();
	int i;
	
	for( i=0; i<lightCount; i++ ){
		pLightList.GetAt( i )->SetLightVolumeDirty();
	}
}



// Render world usage
///////////////////////

void deoglRComponent::SetWorldMarkedRemove( bool marked ){
	pWorldMarkedRemove = marked;
}

void deoglRComponent::SetLLWorldPrev( deoglRComponent *component ){
	pLLWorldPrev = component;
}

void deoglRComponent::SetLLWorldNext( deoglRComponent *component ){
	pLLWorldNext = component;
}



// Private Functions
//////////////////////

class deoglRComponentDeletion : public deoglDelayedDeletion{
public:
	deoglSkinState *skinState;
	deoglSPBlockUBO *paramBlockOccMesh;
	deoglSPBlockUBO *paramBlockSpecial;
	
	deoglRComponentDeletion() :
	skinState( NULL ),
	paramBlockOccMesh( NULL ),
	paramBlockSpecial( NULL ){
	}
	
	virtual ~deoglRComponentDeletion(){
	}
	
	virtual void DeleteObjects( deoglRenderThread& ){
		if( paramBlockSpecial ){
			paramBlockSpecial->FreeReference();
		}
		if( paramBlockOccMesh ){
			paramBlockOccMesh->FreeReference();
		}
		if( skinState ){
			delete skinState;
		}
	}
};

void deoglRComponent::pCleanUp(){
	SetParentWorld( NULL );
	
	RemoveAllTextures();
	RemoveAllLODs();
	
	pRemoveFromAllLights();
	
	NotifyComponentDestroyed();
	pNotifiers.RemoveAll();
	
	if( pDynamicOcclusionMesh ){
		delete pDynamicOcclusionMesh;
	}
	
	if( pBoneMatrices ){
		delete [] pBoneMatrices;
	}
	
	if( pOccMeshSharedSPBDoubleSided ){
		pOccMeshSharedSPBDoubleSided->FreeReference();
			// has to be done before pOcclusionMesh mesh is released
	}
	if( pOccMeshSharedSPBSingleSided ){
		pOccMeshSharedSPBSingleSided->FreeReference();
			// has to be done before pOcclusionMesh mesh is released
	}
	if( pOccMeshSharedSPBElement ){
		pOccMeshSharedSPBElement->FreeReference();
			// has to be done before pOcclusionMesh mesh is released
	}
	
	if( pSkin ){
		pSkin->FreeReference();
	}
	if( pModel ){
		pModel->FreeReference();
	}
	if( pDynamicSkin ){
		pDynamicSkin->FreeReference();
	}
	if( pOcclusionMesh ){
		pOcclusionMesh->FreeReference();
	}
	if( pEnvMap ){
		pEnvMap->FreeReference();
	}
	if( pRenderEnvMap ){
		pRenderEnvMap->FreeReference();
	}
	if( pRenderEnvMapFade ){
		pRenderEnvMapFade->FreeReference();
	}
	
	// drop reference otherwise deletion can cause other deletions to be generated
	// causing a deletion race
	if( pSkinState ){
		pSkinState->DropDelayedDeletionObjects();
	}
	
	// delayed deletion of opengl containing objects
	deoglRComponentDeletion *delayedDeletion = NULL;
	
	try{
		delayedDeletion = new deoglRComponentDeletion;
		delayedDeletion->skinState = pSkinState;
		delayedDeletion->paramBlockOccMesh = pParamBlockOccMesh;
		delayedDeletion->paramBlockSpecial = pParamBlockSpecial;
		pRenderThread.GetDelayedOperations().AddDeletion( delayedDeletion );
		
	}catch( const deException &e ){
		if( delayedDeletion ){
			delete delayedDeletion;
		}
		pRenderThread.GetLogger().LogException( e );
		throw;
	}
}



void deoglRComponent::pInvalidateLODVBOs(){
	const int count = pLODs.GetCount();
	int i;
	
	for( i=0; i<count; i++ ){
		( ( deoglRComponentLOD* )pLODs.GetAt( i ) )->InvalidateVBO();
	}
}



void deoglRComponent::pUpdateModelSkinMappings(){
	if( ! pModel || ! pSkin ){
		const int count = pModelSkinMappings.GetCount();
		int i;
		
		for( i=0; i<count; i++ ){
			pModelSkinMappings.SetAt( i, -1 );
		}
		return;
	}
	
	// determine mappings of textures from model to skin
	const decStringList &textureNames = pModel->GetTextureNames();
	const int count = pModelSkinMappings.GetCount();
	int i, j;
	
	for( i=0; i<count; i++ ){
		pModelSkinMappings.SetAt( i, -1 );
		if( ! pSkin ){
			continue;
		}
		
		const decString &name = textureNames.GetAt( i );
		for( j=0; j<pSkin->GetTextureCount(); j++ ){
			if( pSkin->GetTextureAt( j ).GetName() == name ){
				pModelSkinMappings.SetAt( i, j );
				break;
			}
		}
	}
	
	// mark all textures dirty
	InvalidateAllTexturesParamBlocks();
	MarkAllTexturesTUCsDirty();
	MarkTextureUseSkinDirty();
	
	// hack
	/*if( pSkin ){
		if( pEnvMap ){
			if( pParentWorld ){
				pParentWorld->RemoveEnvMap( pEnvMap );
			}
			delete pEnvMap;
			pEnvMap = NULL;
		}
		
		pDirtyRenderEnvMap = true;
	}*/
}

void deoglRComponent::pResizeModelSkinMappings(){
	int count = 0;
	if( pModel ){
		count = pModel->GetTextureNames().GetCount();
	}
	
	if( count == pModelSkinMappings.GetCount() ){
		return;
	}
	
	pModelSkinMappings.RemoveAll();
	int i;
	for( i=0; i<count; i++ ){
		pModelSkinMappings.Add( -1 );
	}
}

void deoglRComponent::pUpdateModelRigMappings( deComponent &component ){
	if( ! pDirtyModelRigMappings ){
		return;
	}
	
	pDirtyModelRigMappings = false;
	
	const deRig * const rig = component.GetRig();
	const int count = pModelRigMappings.GetCount();
	int i;
	
	if( ! pModel || ! rig ){
		for( i=0; i<count; i++ ){
			pModelRigMappings.SetAt( i, -1 );
		}
		return;
	}
	
	for( i=0; i<count; i++ ){
		pModelRigMappings.SetAt( i, rig->IndexOfBoneNamed( pModel->GetBoneNames().GetAt( i ) ) );
	}
}

void deoglRComponent::pResizeModelRigMappings(){
	int count = 0;
	if( pModel ){
		count = pModel->GetBoneNames().GetCount();
	}
	
	if( count == pModelRigMappings.GetCount() ){
		return;
	}
	
	pModelRigMappings.RemoveAll();
	int i;
	for( i=0; i<count; i++ ){
		pModelRigMappings.Add( -1 );
	}
	
	pDirtyModelRigMappings = true;
}

void deoglRComponent::pCheckRenderModifier( deoglRCamera *rcamera ){
	// TODO implement modifiers in a proper way
#if 0
	int i, count = pComponent->GetModifierCount();
	int renModIndex, renderModifierCount = 0;
	deComponentModifier *modifier;
	deoglRSkin *skin;
	deSkin *finalSkin = pComponent->GetSkin();
	
	// determine the skin to use
	if( rcamera ){
		const deCamera &camera = rcamera->GetCamera()->GetCamera();
		renderModifierCount = camera.GetRenderModifierCount();
		
		for( i=0; i<count; i++ ){
			modifier = pComponent->GetModifierAt( i );
			renModIndex = modifier->GetModifierIndex();
			if( renModIndex < renderModifierCount && camera.GetRenderModifierEnabledAt( renModIndex ) ){
				if( modifier->GetSkin() ){
					finalSkin = modifier->GetSkin();
				}
			}
		}
	}
	
	// if the skin changed update the mappings
	skin = ( ( deoglSkin* )finalSkin->GetPeerGraphic() )->GetRSkin();
	
	if( pSkin != skin ){
		if( pSkin ){
			pSkin->FreeReference();
		}
		pSkin = skin;
		if( skin ){
			skin->AddReference();
		}
		
		pUpdateModelSkinMappings();
	}
#endif
}

void deoglRComponent::pUpdateRenderMode(){
	if( ! pModel ){
		pRenderMode = ermStatic;
		return;
	}
	
	const deoglModelLOD &modelLOD = pModel->GetLODAt( 0 );
	
	if( modelLOD.GetWeightsEntryCount() > 0 && pModel->GetBoneCount() > 0 ){
		if( pBoneMatrixCount == 0 ){
			pRenderMode = ermStatic;
			
		}else{
			pRenderMode = ermDynamic;
		}
		
	}else{
		pRenderMode = ermStatic;
	}
}

void deoglRComponent::pUpdateRenderables(){
	const int textureCount = pTextures.GetCount();
	int i;
	
	// TODO this situation here is not blistering at all. usually there happens only one update at the beginning
	// of the component life time where the renderables are set up. then usually no change happens at all anymore
	// during the life time of the component. with the current code though a lot of counter checking is done for
	// all the components having a dynamic skin to verify no change happened. a notification system would be a
	// better here not only to remove the counter handling but also to remove potential errors.
	
	// update dynamic skins to check
	if( pDynamicSkin && pSkin && pSkin->GetHasRenderables() ){
		const int updateNumber = pDynamicSkin->Update();
		
		if( updateNumber != pSkinState->GetUpdateNumber() ){
			pSkinState->SetUpdateNumber( updateNumber );
			
			pDirtyRenderables = true;
			
			// mark all textures dirty that could be affected by the update. textures are safe if they have a
			// custom texture assigned or if the corresponding skin texture (if existing) is not marked as dynamic.
			// later on this will be made better by usin notifications so this check is automatic since only
			// textures listening for dynamic changes are affected.
			for( i=0; i<textureCount; i++ ){
				deoglRComponentTexture &texture = *( ( deoglRComponentTexture* )pTextures.GetAt( i ) );
				if( texture.GetSkin() || ! pSkin ){
					continue;
				}
				
				const int mapping = pModelSkinMappings.GetAt( i );
				if( mapping == -1 ){
					continue;
				}
				
				const deoglSkinTexture &skinTexture = pSkin->GetTextureAt( mapping );
				if( ! skinTexture.GetDynamicChannels() ){
					continue;
				}
				
				texture.MarkParamBlocksDirty();
				texture.MarkTUCsDirty();
			}
		}
	}
	
	for( i=0; i<textureCount; i++ ){
		deoglRComponentTexture &texture = *( ( deoglRComponentTexture* )pTextures.GetAt( i ) );
		deoglSkinState * const skinState = texture.GetSkinState();
		if( ! skinState || ( ! texture.GetDynamicSkin() && ! pDynamicSkin ) ){
			continue;
		}
		
		const int updateNumber = texture.GetDynamicSkin() ? texture.GetDynamicSkin()->Update() : pDynamicSkin->Update();
		if( updateNumber == skinState->GetUpdateNumber() ){
			continue;
		}
		
		skinState->SetUpdateNumber( updateNumber );
		
		texture.MarkParamBlocksDirty();
		texture.MarkTUCsDirty();
		pDirtyRenderables = true;
	}
	
	// update renderable mappings in the dynamic skins
	if( pDirtyRenderables ){
		// udpate mappings of dynamic skin of component itself
		pSkinState->RemoveAllRenderables();
		if( pSkin && pDynamicSkin ){
			pSkinState->AddRenderables( *pSkin, *pDynamicSkin );
		}
		
		// update mappings of dynamic skins of component textures if existing
		for( i=0; i<textureCount; i++ ){
			deoglRComponentTexture &texture = *( ( deoglRComponentTexture* )pTextures.GetAt( i ) );
			deoglSkinState * const skinState = texture.GetSkinState();
			if( ! skinState ){
				continue;
			}
			
			skinState->RemoveAllRenderables();
			
			deoglRDynamicSkin * const dynamicSkin = texture.GetDynamicSkin() ? texture.GetDynamicSkin() : pDynamicSkin;
			if( texture.GetSkin() && dynamicSkin ){
				skinState->AddRenderables( *texture.GetSkin(), *dynamicSkin );
			}
		}
		
		MarkAllTexturesParamBlocksDirty();
		MarkAllTexturesTUCsDirty();
		MarkTextureUseSkinDirty(); // required?
		
		pDirtyRenderables = false;
	}
}

void deoglRComponent::pUpdateCullSphere(){
	deoglDCollisionSphere sphere;
	deoglDCollisionBox box;
	
	box.SetFromExtends( pMinExtend, pMaxExtend );
	box.GetEnclosingSphere( &sphere );
	
	pCullSphereCenter = sphere.GetCenter();
	pCullSphereRadius = ( float )sphere.GetRadius();
}

void deoglRComponent::pUpdateSolid(){
	pSolid = true;
	pOutlineSolid = true;
	
	if( ! pModel ){
		return;
	}
	
	const int textureCount = pTextures.GetCount();
	int i;
	
	if( pSkin ){
		// if a skin is assigned check each texture using the components for transparency
		for( i=0; i<textureCount; i++ ){
			deoglRComponentTexture &texture = *( ( deoglRComponentTexture* )pTextures.GetAt( i ) );
			deoglRSkin *skin = texture.GetSkin();
			int textureNumber = 0;
			
			if( skin && skin->GetTextureCount() == 0 ){
				skin = NULL;
			}
			
			if( ! skin ){
				const int mapping = pModelSkinMappings.GetAt( i );
				if( mapping != -1 ){
					textureNumber = mapping;
					skin = pSkin;
				}
			}
			
			if( ! skin ){
				continue;
			}
			
			const deoglSkinTexture &skinTexture = skin->GetTextureAt( textureNumber );
			pSolid &= skinTexture.GetSolid();
			pOutlineSolid &= skinTexture.GetIsOutlineSolid();
		}
		
	}else{
		// if no skin is assigned the component can only be transparent
		// if one or more assigned texture skins are transparent
		for( i =0; i <textureCount; i++ ){
			deoglRComponentTexture &texture = *( ( deoglRComponentTexture* )pTextures.GetAt( i ) );
			deoglRSkin *skin = texture.GetSkin();
			if( ! skin || skin->GetTextureCount() == 0 ){
				continue;
			}
			
			const deoglSkinTexture &skinTexture = skin->GetTextureAt( 0 );
			pSolid &= skinTexture.GetSolid();
			pOutlineSolid &= skinTexture.GetIsOutlineSolid();
		}
	}
}



void deoglRComponent::pResizeBoneMatrices(){
	int boneCount = 0;
	if( pModel ){
		boneCount = pModel->GetBoneCount();
	}
	
	oglMatrix3x4 *matrices = NULL;
	
	if( boneCount > 0 ){
		matrices = new oglMatrix3x4[ boneCount ];
	}
	
	if( pBoneMatrices ){
		delete [] pBoneMatrices;
	}
	pBoneMatrices = matrices;
	pBoneMatrixCount = boneCount;
	
	if( pDynamicOcclusionMesh ){
		pDynamicOcclusionMesh->ComponentStateChanged();
	}
}



void deoglRComponent::pRemoveFromAllLights(){
	const int count = pLightList.GetCount();
	int i;
	
	for( i=0; i< count; i++ ){
		pLightList.GetAt( i )->RemoveComponent( this );
	}
	
	pLightList.RemoveAll();
}
