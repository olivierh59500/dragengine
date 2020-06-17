/* 
 * FBX Model Module
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
#include <inttypes.h>

#include "fbxModel.h"
#include "fbxModelCluster.h"
#include "fbxRig.h"
#include "fbxRigBone.h"
#include "../fbxConnection.h"
#include "../fbxNode.h"
#include "../fbxScene.h"
#include "../fbxProperty.h"
#include "../property/fbxPropertyString.h"

#include <dragengine/deObjectReference.h>
#include <dragengine/common/exceptions.h>
#include <dragengine/resources/model/deModelWeight.h>
#include <dragengine/systems/modules/deBaseModule.h>


// Class fbxModel
///////////////////

// Constructor, destructor
////////////////////////////

fbxModel::fbxModel( fbxScene &scene, fbxNode &nodeGeometry ) :
pScene( scene ),
pNodeGeomtry( nodeGeometry ),
pNodeModel( NULL ),
pNodeDeformer( NULL ),
pGeometryID( nodeGeometry.GetPropertyAt( 0 )->GetValueAsLong() ),
pModelID( 0 ),
pDeformerID( 0 ),
pWeights( NULL ),
pWeightCount( 0 ),
pWeightSize( 0 ),
pWeightMatchThreshold( 0.001f ),
pVertices( NULL ),
pVertexCount( 0 )
{
	decPointerList consGeometry;
	scene.FindConnections( pGeometryID, consGeometry );
	
	const int conGeomCount = consGeometry.GetCount();
	int i;
	
	for( i=0; i<conGeomCount; i++ ){
		const fbxConnection &connection = *( ( fbxConnection* )consGeometry.GetAt( i ) );
		fbxNode * const node = scene.NodeWithID( connection.OtherID( pGeometryID ) );
		
		if( node->GetName() == "Model" ){
			if( connection.GetSource() == pGeometryID
			&& node->GetPropertyAt( 2 )->CastString().GetValue() == "Mesh" ){
				if( pNodeModel ){
					DETHROW_INFO( deeInvalidParam, "duplicate model" );
				}
				pNodeModel = node;
				pModelID = connection.GetTarget();
			}
			
		}else if( node->GetName() == "Deformer" ){
			if( connection.GetTarget() == pGeometryID
			&& node->GetPropertyAt( 2 )->CastString().GetValue() == "Skin" ){
				if( pNodeDeformer ){
					DETHROW_INFO( deeInvalidParam, "duplicate deformer" );
				}
				pNodeDeformer = node;
				pDeformerID = connection.GetSource();
			}
		}
	}
	
	if( ! pNodeModel ){
		DETHROW_INFO( deeInvalidParam, "model not found" );
	}
	
	pMatrix = pNodeModel->CalcTransformMatrix() * scene.GetTransformation();
	
	if( pNodeDeformer ){
		deObjectReference refCluster;
		decPointerList consDeformer;
		scene.FindConnections( pDeformerID, consDeformer );
		
		const int conDeformCount = consDeformer.GetCount();
		for( i=0; i<conDeformCount; i++ ){
			const fbxConnection &connection = *( ( fbxConnection* )consDeformer.GetAt( i ) );
			if( connection.GetTarget() != pDeformerID ){
				continue;
			}
			
			fbxNode &node = *scene.NodeWithID( connection.GetSource() );
			if( node.GetName() == "Deformer"
			&& node.GetPropertyAt( 2 )->CastString().GetValue() == "Cluster" ){
				refCluster.TakeOver( new fbxModelCluster( *this, node ) );
				pClusters.Add( refCluster );
			}
		}
	}
	
	const int clusterCount = pClusters.GetCount();
	for( i=0; i<clusterCount; i++ ){
		GetClusterAt( i )->Prepare();
	}
	
	// create vertices
	const fbxProperty &propVertices = *nodeGeometry.FirstNodeNamed( "Vertices" )->GetPropertyAt( 0 );
	const int count = propVertices.GetValueCount() / 3;
	
	if( count > 0 ){
		int j;
		
		pVertices = new sVertex[ count ];
		
		for( i=0, j=0; i<count; i++, j+=3 ){
			pVertices[ i ].weightSet = -1;
			pVertices[ i ].position = pMatrix * decVector(
				propVertices.GetValueAtAsFloat( j ),
				propVertices.GetValueAtAsFloat( j + 1 ),
				propVertices.GetValueAtAsFloat( j + 2 ) );
		}
		pVertexCount = count;
	}
}

fbxModel::~fbxModel(){
	if( pVertices ){
		delete [] pVertices;
	}
	if( pWeights ){
		delete [] pWeights;
	}
}



// Management
///////////////

int fbxModel::GetClusterCount() const{
	return pClusters.GetCount();
}

fbxModelCluster *fbxModel::GetClusterAt( int index ) const{
	return ( fbxModelCluster* )pClusters.GetAt( index );
}

fbxModelCluster *fbxModel::GetClusterNamed( const char *name ) const{
	const int count = pClusters.GetCount();
	int i;
	for( i=0; i<count; i++ ){
		fbxModelCluster * const cluster = ( fbxModelCluster* )pClusters.GetAt( i );
		if( cluster->GetName() == name ){
			return cluster;
		}
	}
	return NULL;
}

void fbxModel::MatchClusters( const fbxRig &rig ){
	const int count = pClusters.GetCount();
	int i;
	
	for( i=0; i<count; i++ ){
		fbxModelCluster &cluster = *( ( fbxModelCluster* )( deObject* )pClusters.GetAt( i ) );
		cluster.SetRigBone( rig.GetBoneNamed( cluster.GetName() ) );
	}
}

void fbxModel::BuildWeights(){
	const int count = pClusters.GetCount();
	int i, j;
	
	for( i=0; i<count; i++ ){
		fbxModelCluster &cluster = *( ( fbxModelCluster* )( deObject* )pClusters.GetAt( i ) );
		if( ! cluster.GetRigBone() ){
			continue;
		}
		
		//const fbxScene::eWeightMode weightMode = fbxScene::ConvWeightMode( cluster.GetNodeCluster() );
		
		const fbxProperty &propIndex = *cluster.GetNodeCluster().FirstNodeNamed( "Indexes" )->GetPropertyAt( 0 );
		const fbxProperty &propWeight = *cluster.GetNodeCluster().FirstNodeNamed( "Weights" )->GetPropertyAt( 0 );
		const int entryCount = propIndex.GetValueCount();
		const int bone = cluster.GetRigBone()->GetIndex();
		
		for( j=0; j<entryCount; j++ ){
			const int index = propIndex.GetValueAtAsInt( j );
			const float weight = propWeight.GetValueAtAsFloat( j );
			pVertices[ index ].weights.Add( pAddWeight( bone, weight ) );
		}
	}
}



void fbxModel::DebugPrintStructure( deBaseModule &module, const decString &prefix, bool verbose ) const{
	const int count = pClusters.GetCount();
	int i;
	
	module.LogInfoFormat( "%sModel", prefix.GetString() );
	
	const decString childPrefix( prefix + "  " );
	for( i=0; i<count; i++ ){
		GetClusterAt( i )->DebugPrintStructure( module, childPrefix, verbose );
	}
}



// Private Functions
//////////////////////

int fbxModel::pAddWeight( int bone, float weight ){
	if( pWeightCount == pWeightSize ){
		const int newSize = pWeightSize * 3 / 2 + 1;
		deModelWeight * const newArray = new deModelWeight[ newSize ];
		if( pWeights ){
			memcpy( newArray, pWeights, sizeof( deModelWeight ) * pWeightSize );
			delete [] pWeights;
		}
		pWeights = newArray;
		pWeightSize = newSize;
	}
	
	const int index = pWeightCount++;
	pWeights[ index ].SetBone( bone );
	pWeights[ index ].SetWeight( weight );
	return index;
}
