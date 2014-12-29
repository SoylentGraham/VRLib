/************************************************************************************

Filename    :   GlStateSave.h
Content     :   
Created     :   July 14, 2014
Authors     :   John Carmack

Copyright   :   Copyright 2014 Oculus VR, LLC. All Rights reserved.


*************************************************************************************/

#ifndef OVR_GLSTATESAVE_H
#define OVR_GLSTATESAVE_H

#include "GlUtils.h"

namespace OVR
{

class GLStateSave
{
public:
	// If our code only uses VertexArrayObjects, we don't need to
	// save and restore all the buffer related state
	static const bool	exclusivelyVAO = true;

	GLStateSave()
	{
		glGetIntegerv( GL_BLEND, &Blend );
		glGetIntegerv( GL_BLEND_DST_ALPHA, &BlendDstAlpha );
		glGetIntegerv( GL_BLEND_DST_RGB, &BlendDstRGB );
		glGetIntegerv( GL_BLEND_EQUATION_ALPHA, &BlendEquationAlpha );
		glGetIntegerv( GL_BLEND_EQUATION_RGB, &BlendEquationRgb );
		glGetIntegerv( GL_BLEND_SRC_ALPHA, &BlendSrcAlpha );
		glGetIntegerv( GL_BLEND_SRC_RGB, &BlendSrcRGB );

		// We update an array for debug graphs, so this needs to
		// be saved even though we use VAO, because GL_ARRAY_BUFFER_BINDING
		// isn't part of the VAO state.
		glGetIntegerv( GL_ARRAY_BUFFER_BINDING, &ArrayBuffer );

		if ( !exclusivelyVAO )
		{
			glGetIntegerv( GL_ELEMENT_ARRAY_BUFFER_BINDING, &ElementArrayBuffer );
			for ( int i = 0 ; i < MAX_ATTRIBS ; i++ )
			{
				glGetVertexAttribiv( i,	GL_VERTEX_ATTRIB_ARRAY_ENABLED, &vertexAttribArrayEnabled[i] );
			}
		}

		glGetIntegerv( GL_SCISSOR_TEST, &ScissorTest );
		glGetIntegerv( GL_SCISSOR_BOX, ScissorBox );

		glGetIntegerv( GL_DEPTH_TEST, &DepthTest );
		glGetIntegerv( GL_DEPTH_FUNC, &DepthFunc );
		glGetIntegerv( GL_DEPTH_WRITEMASK, &DepthWriteMask );
		glGetIntegerv( GL_CULL_FACE, &CullFace );
	}

	~GLStateSave()
	{
		glBlendEquationSeparate( BlendEquationRgb, BlendEquationAlpha );
		glBlendFuncSeparate( BlendSrcRGB, BlendDstRGB, BlendSrcAlpha, BlendDstAlpha );
		GL_Enable( GL_BLEND, Blend );

		glBindBuffer( GL_ARRAY_BUFFER, ArrayBuffer );

		if ( exclusivelyVAO )
		{
			glBindVertexArrayOES_( 0 );
		}
		else
		{
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ElementArrayBuffer );
			for ( int i = 0 ; i < MAX_ATTRIBS ; i++ )
			{
				if ( vertexAttribArrayEnabled[i] )
				{
					glEnableVertexAttribArray(i);
				}
				else
				{
					glDisableVertexAttribArray(i);
				}
			}
		}

		GL_Enable( GL_SCISSOR_TEST, ScissorTest );
		glScissor( ScissorBox[0], ScissorBox[1], ScissorBox[2], ScissorBox[3] );

		GL_Enable( GL_DEPTH_TEST, DepthTest );
		glDepthFunc( DepthFunc );

		glDepthMask( DepthWriteMask );

		GL_Enable( GL_CULL_FACE, CullFace );
	}

	void GL_Enable( const GLenum feature, GLboolean enabled )
	{
		if ( enabled )
		{
			glEnable( feature );
		}
		else
		{
			glDisable( feature );
		}
	}


	GLint	Blend;
	GLint	BlendDstAlpha;
	GLint	BlendDstRGB;
	GLint	BlendEquationAlpha;
	GLint	BlendEquationRgb;
	GLint	BlendSrcAlpha;
	GLint	BlendSrcRGB;

	GLint	ElementArrayBuffer;
	GLint	ArrayBuffer;
	static const int MAX_ATTRIBS = 4;
	GLint	vertexAttribArrayEnabled[MAX_ATTRIBS];

	GLint	ScissorTest;
	GLint	ScissorBox[4];

	GLint	DepthTest;
	GLint	DepthFunc;
	GLint	DepthWriteMask;

	GLint   CullFace;
};

}	// namespace OVR

#endif // OVR_GLSTATESAVE_H
