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
#include <string.h>
#include <unistd.h>

#include "deoglRenderWindow.h"
#include "deoglRRenderWindow.h"
#include "../deGraphicOpenGl.h"
#include "../canvas/deoglCanvasView.h"
#include "../texture/deoglImage.h"
#include "../texture/pixelbuffer/deoglPixelBuffer.h"
#include "../renderthread/deoglRenderThread.h"

#include <dragengine/deEngine.h>
#include <dragengine/app/deOS.h>
#include <dragengine/common/exceptions.h>
#include <dragengine/resources/canvas/deCanvasView.h>
#include <dragengine/resources/image/deImage.h>
#include <dragengine/resources/rendering/deRenderWindow.h>
#include <dragengine/systems/deGraphicSystem.h>
#include <dragengine/systems/deScriptingSystem.h>

#ifdef OS_BEOS
#include <DirectWindow.h>
#endif



// Class deoglRenderWindow
////////////////////////////

// Constructor, destructor
////////////////////////////

deoglRenderWindow::deoglRenderWindow( deGraphicOpenGl &ogl, deRenderWindow &renderWindow ) :
pOgl( ogl ),
pRenderWindow( renderWindow ),

pCanvasView( NULL ),
pDirtyParams( false ),
pDirtySize( true ),
pDirtyWindowTitle( true ),
pDirtyFullScreen( true ),
pDirtyIcon( true ),

pRRenderWindow( NULL )
{
	try{
		pCanvasView = ( deoglCanvasView* )renderWindow.GetCanvasView()->GetPeerGraphic();
		
		pRRenderWindow = new deoglRRenderWindow( ogl.GetRenderThread() );
		
		pRRenderWindow->SetHostWindow( renderWindow.GetHostWindow() );
		pRRenderWindow->SetSize( renderWindow.GetWidth(), renderWindow.GetHeight() );
		pRRenderWindow->SetTitle( renderWindow.GetTitle() );
		
		ogl.GetRenderThread().CreateRenderWindow( pRRenderWindow );
		
		renderWindow.SetWindow( pRRenderWindow->GetWindow() );
		
		// set application as active. x-system does not tell the new window it obtained
		// the focus so we do this instead. (is this a bug or feature of x-system?)
		pOgl.GetOS()->SetAppActive( true );
		
	}catch( const deException & ){
		pCleanUp();
		throw;
	}
	
	ogl.GetRenderWindowList().Add( this );
}

deoglRenderWindow::~deoglRenderWindow(){
	pOgl.GetRenderWindowList().Remove( this );
	pCleanUp();
}



// Management
///////////////



// Notifications
//////////////////

void deoglRenderWindow::HostWindowChanged(){
	pRRenderWindow->SetHostWindow( pRenderWindow.GetHostWindow() );
}

void deoglRenderWindow::SizeChanged(){
	pDirtySize = true;
}

void deoglRenderWindow::TitleChanged(){
	pDirtyWindowTitle = true;
}

void deoglRenderWindow::FullScreenChanged(){
	pDirtyFullScreen = true;
}

void deoglRenderWindow::ScriptNotifyChanged(){
}

void deoglRenderWindow::PaintEnabledChanged(){
	pDirtyParams = true;
}

void deoglRenderWindow::IconChanged(){
	pDirtyIcon = true;
}



void deoglRenderWindow::SyncToRender(){
	if( pDirtyFullScreen ){
		pRRenderWindow->SetFullScreen( pRenderWindow.GetFullScreen() );
		pDirtyFullScreen = false;
	}
	
	if( pDirtySize ){
		pRRenderWindow->SetSize( pRenderWindow.GetWidth(), pRenderWindow.GetHeight() );
		pDirtySize = false;
	}
	
	if( pDirtyWindowTitle ){
		pRRenderWindow->SetTitle( pRenderWindow.GetTitle() );
		pDirtyWindowTitle = false;
	}
	
	if( pDirtyIcon ){
		if( pRenderWindow.GetIcon() ){
			deoglImage &image = *( ( deoglImage* )pRenderWindow.GetIcon()->GetPeerGraphic() );
			deoglPixelBuffer *pixelBuffer = NULL;
			
			image.CreatePixelBuffer();
			
			try{
				pixelBuffer = new deoglPixelBuffer( *image.GetPixelBuffer() );
				pRRenderWindow->SetIcon( pixelBuffer );
				
			}catch( const deException & ){
				if( pixelBuffer ){
					delete pixelBuffer;
				}
				image.ReleasePixelBuffer();
				throw;
			}
			image.ReleasePixelBuffer();
			
		}else{
			pRRenderWindow->SetIcon( NULL );
		}
		pDirtyIcon = false;
	}
	
	// if parameters changed set them to the window. this has to be done always
	if( pDirtyParams ){
		//pRRenderWindow->SetHostWindow( pRenderWindow.GetHostWindow() );
		pRRenderWindow->SetPaint( pRenderWindow.GetPaint() );
		pDirtyParams = false;
	}
	
	// sync canvas view only if painting is enabled. saves performance on invisible windows
	if( pRenderWindow.GetPaint() ){
		pCanvasView->SyncToRender();
		pRRenderWindow->SetRCanvasView( pCanvasView->GetRCanvasView() );
	}
	
	// window size changed due to OS events
	if( pRRenderWindow->GetNotifySizeChanged() ){
		pRenderWindow.SetSize( pRRenderWindow->GetWidth(), pRRenderWindow->GetHeight() );
	}
}



// Private Functions
//////////////////////

void deoglRenderWindow::pCleanUp(){
	if( pRRenderWindow ){
		pRRenderWindow->SetFullScreen( false ); // make sure full screen is disabled if enabled
		pRRenderWindow->FreeReference();
	}
	
	#if defined OS_UNIX && ! defined ANDROID && ! defined OS_BEOS && ! defined OS_MACOS
	pRenderWindow.SetWindow( 0 );
	#endif
	
	#ifdef ANDROID
	pRenderWindow.SetWindow( NULL );
	#endif
	
	#ifdef OS_BEOS
	pRenderWindow.SetWindow( NULL );
	#endif
	
	#ifdef OS_W32
	pRenderWindow.SetWindow( NULL );
	#endif
	
	#ifdef OS_MACOS
	pRenderWindow.SetWindow( NULL );
	#endif
}
