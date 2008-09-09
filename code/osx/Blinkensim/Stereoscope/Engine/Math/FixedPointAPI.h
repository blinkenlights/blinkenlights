/*
Oolong Engine for the iPhone / iPod touch
Copyright (c) 2007-2008 Wolfgang Engel  http://code.google.com/p/oolongengine/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#ifndef FIXEDPOINTAPI_H_
#define FIXEDPOINTAPI_H_

#include <OpenGLES/ES1/gl.h>


//
// allows to switch between fixed point and float API and math by toggling the define
// 
#ifndef FIXEDPOINTENABLE

	#define VERTTYPE GLfloat
	#define VERTTYPEENUM GL_FLOAT

	#define myglFog				glFogf
	#define myglFogv			glFogfv

	#define myglLightv			glLightfv
	#define myglLight			glLightf

	#define myglLightModelv		glLightModelfv
	#define myglLightModel		glLightModelf

	#define myglAlphaFunc		glAlphaFunc

	#define myglMaterialv		glMaterialfv
	#define myglMaterial		glMaterialf

	#define myglTexParameter	glTexParameterf
	#define myglTexEnv			glTexEnvf

	#define myglOrtho			glOrthof
	#define myglFrustum			glFrustumf

	#define myglTranslate		glTranslatef
	#define myglScale			glScalef
	#define myglRotate			glRotatef

	#define myglColor4			glColor4f

	#define myglClearColor		glClearColor

	#define myglMultMatrix		glMultMatrixf

	#define myglNormal3			glNormal3f

	#define myglLoadMatrix		glLoadMatrixf

	#define myglPolygonOffset	glPolygonOffset

	#define myglPointSize		glPointSize

	// GL_IMG_VERTEX_PROGRAM extensions
	#define myglProgramLocalParameter4v		glProgramLocalParameter4fvARB
	#define myglProgramLocalParameter4		glProgramLocalParameter4fARB
	#define myglProgramEnvParameter4v		glProgramEnvParameter4fvARB
	#define myglProgramEnvParameter4		glProgramEnvParameter4fARB
	#define myglVertexAttrib4v				glVertexAttrib4fvARB

	#define myglClipPlane					glClipPlanef

	#define myglPointParameter				glPointParameterf

	#define myglPointParameterv				glPointParameterfv


#else

	#define VERTTYPE GLfixed
	#define VERTTYPEENUM GL_FIXED

	#define myglFog				glFogx
	#define myglFogv			glFogxv

	#define myglLight			glLightx
	#define myglLightv			glLightxv

	#define myglLightModel		glLightModelx
	#define myglLightModelv		glLightModelxv

	#define myglAlphaFunc		glAlphaFuncx

	#define myglMaterial		glMaterialx
	#define myglMaterialv		glMaterialxv

	#define myglTexParameter	glTexParameterx
	#define myglTexEnv			glTexEnvx

	#define myglOrtho			glOrthox
	#define myglFrustum			glFrustumx

	#define myglTranslate		glTranslatex
	#define myglScale			glScalex
	#define myglRotate			glRotatex

	#define myglColor4			glColor4x

	#define myglClearColor		glClearColorx

	#define myglMultMatrix		glMultMatrixx

	#define myglNormal3			glNormal3x

	#define myglLoadMatrix		glLoadMatrixx

	#define myglPolygonOffset	glPolygonOffsetx

	#define myglPointSize		glPointSizex

	// GL_IMG_VERTEX_PROGRAM extensions
	#define myglProgramLocalParameter4v	glProgramLocalParameter4xvIMG
	#define myglProgramLocalParameter4	glProgramLocalParameter4xIMG
	#define myglProgramEnvParameter4v	glProgramEnvParameter4xvIMG
	#define myglProgramEnvParameter4	glProgramEnvParameter4xIMG
	#define myglVertexAttrib4v			glVertexAttrib4xvIMG

	#define myglClipPlane		glClipPlanex

	#define myglPointParameter	glPointParameterx
	#define myglPointParameterv	glPointParameterxv

#endif


#endif // FIXEDPOINT_H_

