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

#include "deoglREffectOverlayImage.h"
#include "../../rendering/defren/deoglDeferredRendering.h"
#include "../../rendering/plan/deoglRenderPlan.h"
#include "../../renderthread/deoglRenderThread.h"
#include "../../renderthread/deoglRTShader.h"
#include "../../renderthread/deoglRTTexture.h"
#include "../../renderthread/deoglRTLogger.h"
#include "../../shaders/deoglShaderCompiled.h"
#include "../../shaders/deoglShaderDefines.h"
#include "../../shaders/deoglShaderManager.h"
#include "../../shaders/deoglShaderProgram.h"
#include "../../shaders/deoglShaderSources.h"
#include "../../texture/deoglRImage.h"
#include "../../texture/deoglTextureStageManager.h"
#include "../../texture/texture2d/deoglTexture.h"
#include "../../delayedoperation/deoglDelayedDeletion.h"
#include "../../delayedoperation/deoglDelayedOperations.h"

#include <dragengine/common/exceptions.h>



// Definitions
////////////////

enum eSPEffect{
	spePosTransform,
	speTCTransform,
	speGamma,
	speColor
};

// Class deoglREffectOverlayImage
///////////////////////////////////

// Constructor, destructor
////////////////////////////

deoglREffectOverlayImage::deoglREffectOverlayImage( deoglRenderThread &renderThread ) :
deoglREffect( renderThread ),
pImage( NULL ),
pShader( NULL ){
	LEAK_CHECK_CREATE( renderThread, EffectOverlayImage );
}

class deoglREffectOverlayImageDeletion : public deoglDelayedDeletion{
public:
	deoglShaderProgram *shader;
	
	deoglREffectOverlayImageDeletion() :
	shader( NULL ){
	}
	
	virtual ~deoglREffectOverlayImageDeletion(){
	}
	
	virtual void DeleteObjects( deoglRenderThread &renderThread ){
		if( shader ){
			shader->RemoveUsage();
		}
	}
};

deoglREffectOverlayImage::~deoglREffectOverlayImage(){
	LEAK_CHECK_FREE( GetRenderThread(), EffectOverlayImage );
	if( pImage ){
		pImage->FreeReference();
	}
	
	// delayed deletion of opengl containing objects
	deoglREffectOverlayImageDeletion *delayedDeletion = NULL;
	
	try{
		delayedDeletion = new deoglREffectOverlayImageDeletion;
		delayedDeletion->shader = pShader;
		GetRenderThread().GetDelayedOperations().AddDeletion( delayedDeletion );
		
	}catch( const deException &e ){
		if( delayedDeletion ){
			delete delayedDeletion;
		}
		GetRenderThread().GetLogger().LogException( e );
		//throw; -> otherwise terminate
	}
}



// Management
///////////////

void deoglREffectOverlayImage::SetTransparency( float transparency ){
	pTransparency = decMath::clamp( transparency, 0.0f, 1.0f );
}

void deoglREffectOverlayImage::SetImage( deoglRImage *image ){
	if( image == pImage ){
		return;
	}
	
	if( pImage ){
		pImage->FreeReference();
	}
	
	pImage = image;
	
	if( image ){
		image->AddReference();
	}
}



deoglShaderProgram *deoglREffectOverlayImage::GetShader(){
	if( ! pShader ){
		deoglShaderManager &shaderManager = GetRenderThread().GetShader().GetShaderManager();
		
		deoglShaderSources * const sources = shaderManager.GetSourcesNamed( "Effect Overlay" );
		if( ! sources ){
			DETHROW( deeInvalidParam );
		}
		
		pShader = shaderManager.GetProgramWith( sources, deoglShaderDefines() );
	}
	
	return pShader;
}

void deoglREffectOverlayImage::PrepareForRender(){
	if( pImage ){
		pImage->PrepareForRender();
	}
}

void deoglREffectOverlayImage::Render( deoglRenderPlan &plan ){
	if( ! pImage ){
		return;
	}
	
	deoglTexture * const texture = pImage->GetTexture();
	if( ! texture ){
		return;
	}
	
	deoglRenderThread &renderThread = GetRenderThread();
	deoglTextureStageManager &tsmgr = renderThread.GetTexture().GetStages();
	deoglDeferredRendering &defren = renderThread.GetDeferredRendering();
	deoglRTShader &rtshader = renderThread.GetShader();
	
	float scaleU = pImage->GetScaleFactorU();
	float scaleV = pImage->GetScaleFactorV();
	/*
	const decVector2 &tc1 = pEffectOI->GetTextureCoordinatesFor( 0 );
	const decVector2 &tc2 = pEffectOI->GetTextureCoordinatesFor( 1 );
	const decVector2 &tc3 = pEffectOI->GetTextureCoordinatesFor( 2 );
	const decVector2 &tc4 = pEffectOI->GetTextureCoordinatesFor( 3 );
	*/
	
	// swap render texture
	defren.ActivatePostProcessFBO( false );
	tsmgr.EnableTexture( 0, *texture, *rtshader.GetTexSamplerConfig( deoglRTShader::etscClampLinear ) );
	
	// set states
	OGL_CHECK( renderThread, glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE ) );
	OGL_CHECK( renderThread, glDepthMask( GL_FALSE ) );
	
	OGL_CHECK( renderThread, glDisable( GL_DEPTH_TEST ) );
		
	OGL_CHECK( renderThread, glDisable( GL_CULL_FACE ) );
	
	OGL_CHECK( renderThread, glEnable( GL_BLEND ) );
	OGL_CHECK( renderThread, glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) );
	
	// set shader program
	// [-1,1] * su/2 + su/2 = [0,su]
	// [-1,1] * sv/2 + sv/2 = [0,sv]
	deoglShaderProgram * const shaderProgram = GetShader();
	rtshader.ActivateShader( shaderProgram );
	deoglShaderCompiled &shader = *shaderProgram->GetCompiled();
	
	shader.SetParameterFloat( spePosTransform, 1.0f, 1.0f, 0.0f, 0.0f );
	shader.SetParameterFloat( speTCTransform, scaleU * 0.5f, scaleV * 0.5f, scaleU * 0.5f, scaleV * 0.5f );
	shader.SetParameterFloat( speGamma, OGL_RENDER_GAMMA, OGL_RENDER_GAMMA, OGL_RENDER_GAMMA, 1.0 );
	shader.SetParameterFloat( speColor, 1.0f, 1.0f, 1.0f, pTransparency );
	
	defren.RenderFSQuadVAO();
}
