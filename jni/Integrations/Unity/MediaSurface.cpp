/************************************************************************************

Filename    :   MediaSurface.cpp
Content     :   Interface to copy/mip a SurfaceTexture stream onto a normal GL texture
Created     :   July 14, 2014
Authors     :   John Carmack

Copyright   :   Copyright 2014 Oculus VR, LLC. All Rights reserved.


*************************************************************************************/

#include "MediaSurface.h"
#include "Log.h"
#include "GlStateSave.h"
#include "../../LibOVR/Src/Kernel/OVR_Types.h"
#include "../../LibOVR/Src/Kernel/OVR_Timer.h"


MediaSurfaceStatsController::MediaSurfaceStatsController() :
	mBeginCopyTime			( 0 ),
	mBeginSurfaceUpdateTime	( 0 ),
	mLastTimestampTime		( 0 )
{
	mTextureCopyMs = 0;
	mSurfaceUpdateMs = 0;
	mTimestampStepMs = 0;
}

void MediaSurfaceStatsController::BeginCopy()
{
	mBeginCopyTime = OVR::Timer::GetTicksMs();
}

void MediaSurfaceStatsController::EndCopy()
{
	OVR::UInt32 Now = OVR::Timer::GetTicksMs();
	mTextureCopyMs = Now - mBeginCopyTime;
}

void MediaSurfaceStatsController::BeginSurfaceUpdate()
{
	mBeginSurfaceUpdateTime = OVR::Timer::GetTicksMs();
}

void MediaSurfaceStatsController::EndSurfaceUpdate()
{
	OVR::UInt32 Now = OVR::Timer::GetTicksMs();
	mSurfaceUpdateMs = Now - mBeginSurfaceUpdateTime;
}

void MediaSurfaceStatsController::OnNewTimestamp()
{
	OVR::UInt32 Now = OVR::Timer::GetTicksMs();
	if ( mLastTimestampTime != 0 )
		mTimestampStepMs = Now - mLastTimestampTime;
	mLastTimestampTime = Now;
}

namespace OVR
{


MediaSurface::MediaSurface() :
	jni( NULL ),
	AndroidSurfaceTexture( NULL ),
	SurfaceObject( NULL ),
	LastSurfaceTexTimeStamp( 0 ),
	TexId( 0 ),
	TexIdWidth( 0 ),
	TexIdHeight( 0 ),
	Fbo( 0 ),
	TargetTextureWidth( 1920 ),
	TargetTextureHeight( 640 )
{
}

void MediaSurface::Init( JNIEnv * jni_ )
{
	LOG( "MediaSurface::Init()" );

	jni = jni_;

	LastSurfaceTexTimeStamp = 0;
	TexId = 0;
	Fbo = 0;

	// Setup a surface for playing movies in Unity
	AndroidSurfaceTexture = new SurfaceTexture( jni );

	static const char * className = "android/view/Surface";
	const jclass surfaceClass = jni->FindClass(className);
	if ( surfaceClass == 0 ) {
		FAIL( "FindClass( %s ) failed", className );
	}

	// find the constructor that takes a surfaceTexture object
	const jmethodID constructor = jni->GetMethodID( surfaceClass, "<init>", "(Landroid/graphics/SurfaceTexture;)V" );
	if ( constructor == 0 ) {
		FAIL( "GetMethodID( <init> ) failed" );
	}

	jobject obj = jni->NewObject( surfaceClass, constructor, AndroidSurfaceTexture->javaObject );
	if ( obj == 0 ) {
		FAIL( "NewObject() failed" );
	}

	SurfaceObject = jni->NewGlobalRef( obj );
	if ( SurfaceObject == 0 ) {
		FAIL( "NewGlobalRef() failed" );
	}

	// Now that we have a globalRef, we can free the localRef
	jni->DeleteLocalRef( obj );

	// The class is also a localRef that we can delete
	jni->DeleteLocalRef( surfaceClass );
}

void MediaSurface::Shutdown()
{
	LOG( "MediaSurface::Shutdown()" );

	DeleteProgram( CopyMovieProgram );
	UnitSquare.Free();

	delete AndroidSurfaceTexture;
	AndroidSurfaceTexture = NULL;

	if ( Fbo )
	{
		glDeleteFramebuffers( 1, &Fbo );
		Fbo = 0;
	}

	if ( jni != NULL )
	{
		if ( SurfaceObject != NULL )
		{
			jni->DeleteGlobalRef( SurfaceObject );
			SurfaceObject = NULL;
		}
	}
}

jobject MediaSurface::Bind( int toTexId )
{
	LOG( "BindMediaSurface" );
	if ( SurfaceObject == NULL )
	{
		LOG( "SurfaceObject == NULL" );
		abort();
	}
	TexId = toTexId;
	TexIdWidth = -1;
	TexIdHeight = -1;
	return SurfaceObject;
}

void MediaSurface::Update()
{
	if ( !AndroidSurfaceTexture )
	{
		LOG( "!AndroidSurfaceTexture" );
		return;
	}
	if ( TexId <= 0 )
	{
		//LOG( "TexId <= 0" );
		return;
	}
	Stats.BeginSurfaceUpdate();
	AndroidSurfaceTexture->Update();
	Stats.EndSurfaceUpdate();
	if ( AndroidSurfaceTexture->timestamp == LastSurfaceTexTimeStamp )
	{
		return;
	}
	Stats.OnNewTimestamp();
	LastSurfaceTexTimeStamp = AndroidSurfaceTexture->timestamp;

	//	gr: note we're ignoring the GLStateSave destructor. make this a scoped timer
	Stats.BeginCopy();

	// don't mess up Unity state
	GLStateSave	stateSave;

	// If we haven't allocated our GL objects yet, do it now.
	// This isn't done at Init, because GL may not be current then.
	if ( UnitSquare.vertexArrayObject == 0 )
	{
		LOG( "Allocating GL objects" );

		UnitSquare = BuildTesselatedQuad( 1, 1 );

		CopyMovieProgram = BuildProgram(
			"uniform highp mat4 Mvpm;\n"
			"attribute vec4 Position;\n"
			"attribute vec2 TexCoord;\n"
			"varying  highp vec2 oTexCoord;\n"
			"void main()\n"
			"{\n"
			"   gl_Position = Position;\n"
			"   oTexCoord = TexCoord;\n"
			"}\n"
		,
			"#extension GL_OES_EGL_image_external : require\n"
			"uniform samplerExternalOES Texture0;\n"
			"varying highp vec2 oTexCoord;\n"
			"void main()\n"
			"{\n"
			"	gl_FragColor = texture2D( Texture0, oTexCoord );\n"
			"}\n"
		);
	}

	
	// If the SurfaceTexture has changed dimensions, we need to
	// reallocate the texture and FBO.
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_EXTERNAL_OES, AndroidSurfaceTexture->textureId );
	// FIXME: no way to get texture dimensions even in ES 3.0???
	int width = TargetTextureWidth;
	int height = TargetTextureHeight;
	if ( width != TexIdWidth || height != TexIdHeight )
	{
		LOG( "New surface size: %ix%i", width, height );

		TexIdWidth = width;
		TexIdHeight = height;

		if ( Fbo )
		{
			glDeleteFramebuffers( 1, &Fbo );
		}

		glActiveTexture( GL_TEXTURE1 );
		glBindTexture( GL_TEXTURE_2D, TexId );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA,
				TexIdWidth, TexIdHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

		glBindTexture( GL_TEXTURE_2D, 0 );
		glActiveTexture( GL_TEXTURE0 );

		glGenFramebuffers( 1, &Fbo );
		glBindFramebuffer( GL_FRAMEBUFFER, Fbo );
		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
				TexId, 0 );
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	}
	

	glBindFramebuffer( GL_FRAMEBUFFER, Fbo );
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_SCISSOR_TEST );
	glDisable( GL_STENCIL_TEST );
	glDisable( GL_CULL_FACE );
	glDisable( GL_BLEND );

	const GLenum fboAttachments[1] = { GL_COLOR_ATTACHMENT0 };
	glInvalidateFramebuffer( GL_FRAMEBUFFER, 1, fboAttachments );

	glViewport( 0, 0, TexIdWidth, TexIdHeight );
	glUseProgram( CopyMovieProgram.program );
	UnitSquare.Draw();
	glUseProgram( 0 );
	glBindTexture( GL_TEXTURE_EXTERNAL_OES, 0 );
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	glBindTexture( GL_TEXTURE_2D, TexId );
	glGenerateMipmap( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, 0 );
	
	Stats.EndCopy();
}
	
template<typename T>
inline const T& Max(const T& a,const T& b)
{
	return a > b ? a : b;
}

void MediaSurface::SetTargetSize(int Width,int Height)
{
	//	gr: find device max dimensions, restrict to powers?
	TargetTextureWidth = Max( 1, Width );
	TargetTextureHeight = Max( 1, Height );
}


}	// namespace OVR

