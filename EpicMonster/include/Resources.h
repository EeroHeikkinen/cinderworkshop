#pragma once
#include "cinder/CinderResources.h"

#define PASSTHROUGH_VERT		CINDER_RESOURCE( ../resources/, passThrough.vert, 128, GLSL )
#define PARTICLES_FRAG			CINDER_RESOURCE( ../resources/, particles.frag, 129, GLSL )
#define VERTEXDISPLACEMENT_VERT	CINDER_RESOURCE( ../resources/, vertexDisplacementVert.glsl, 130, GLSL )
#define VERTEXDISPLACEMENT_FRAG	CINDER_RESOURCE( ../resources/, vertexDisplacementFrag.glsl, 131, GLSL )
#define BAKE_VERT	CINDER_RESOURCE( ../resources/, bakeVert.glsl, 132, GLSL )
#define BAKE_FRAG	CINDER_RESOURCE( ../resources/, bakeFrag.glsl, 133, GLSL )
#define RES_NORMAL	CINDER_RESOURCE( ../resources/, Normal.png, 134, IMAGE )