/************************************************************************************

Filename    :   MediaSurface.h
Content     :   Interface to copy/mip a SurfaceTexture stream onto a normal GL texture
Created     :   July 14, 2014
Authors     :   John Carmack

Copyright   :   Copyright 2014 Oculus VR, LLC. All Rights reserved.


*************************************************************************************/
#ifndef OVRMEDIASURFACE_H
#define OVRMEDIASURFACE_H

#include <jni.h>
#include "GlGeometry.h"
#include "GlProgram.h"
#include "SurfaceTexture.h"
#include "GlUtils.h"


struct MediaSurfaceStats
{
	int		mTextureCopyMs;				//	opengl copy time
	int		mSurfaceUpdateMs;		//	time between each frame notification
};

class MediaSurfaceStatsController : public MediaSurfaceStats
{
public:
	MediaSurfaceStatsController();
	
	void		BeginCopy();
	void		EndCopy();
	void		BeginSurfaceUpdate();
	void		EndSurfaceUpdate();
	
private:
	int		mBeginCopyTime;
	int		mBeginSurfaceUpdateTime;
};

namespace OVR {

class MediaSurface
{
public:
					MediaSurface();

	// Must be called on the launch thread so Android Java classes
	// can be looked up.
	void			Init( JNIEnv * jni );
	void			Shutdown();

	// Designates the target texId that Update will render to.
	jobject			Bind( int toTexId );

	// Must be called with a current OpenGL context
	void			Update();

	JNIEnv * 		jni;
	SurfaceTexture	* AndroidSurfaceTexture;
	GlProgram		CopyMovieProgram;
	GlGeometry		UnitSquare;
	jobject			SurfaceObject;
	long long		LastSurfaceTexTimeStamp;
	int				TexId;
	int				TexIdWidth;
	int				TexIdHeight;
	GLuint			Fbo;
	MediaSurfaceStatsController	Stats;
};

}	// namespace OVR

#endif // OVRMEDIASURFACE_H
