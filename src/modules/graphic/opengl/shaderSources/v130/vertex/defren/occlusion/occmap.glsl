precision highp float;
precision highp int;

#include "v130/shared/ubo_defines.glsl"

UBOLAYOUT uniform RenderParameters{
	mat4 pMatrixVP[ 6 ];
	mat4x3 pMatrixV[ 6 ];
	vec4 pTransformZ[ 6 ];
	vec2 pZToDepth;
};

#include "v130/shared/defren/occmap.glsl"

#ifdef SHARED_SPB
	#include "v130/shared/defren/skin/shared_spb_index.glsl"
	#define pMatrixModel pSharedSPB[ spbIndex ].pSPBMatrixModel
#endif

in vec3 inPosition;

#if defined PERSPECTIVE_TO_LINEAR && ! defined GS_RENDER_CUBE
out float vDepth;
#endif
#ifdef DEPTH_DISTANCE
out vec3 vPosition;
#endif
#ifdef GS_RENDER_CUBE
	flat out int vSPBIndex;
	#ifdef GS_RENDER_CUBE_CULLING
		flat out int vSPBFlags;
	#endif
#endif

void main( void ){
	#include "v130/shared/defren/skin/shared_spb_index2.glsl"
	
	vec4 position = vec4( pMatrixModel * vec4( inPosition, 1.0 ), 1.0 );
	#ifdef GS_RENDER_CUBE
		gl_Position = position;
	#else
		gl_Position = pMatrixVP[ 0 ] * position;
		#ifdef PERSPECTIVE_TO_LINEAR
			vDepth = dot( pTransformZ[ 0 ], position ) * pZToDepth.x + pZToDepth.y;
		#endif
		#ifdef DEPTH_DISTANCE
			vPosition = pMatrixV[ 0 ] * position;
		#endif
	#endif
	
	#ifdef GS_RENDER_CUBE
		vSPBIndex = spbIndex;
		#ifdef GS_RENDER_CUBE_CULLING
			vSPBFlags = spbFlags;
		#endif
	#endif
}
