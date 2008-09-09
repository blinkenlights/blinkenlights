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
/*
 This example uses art assets from the PowerVR SDK. Imagination Technologies / PowerVR allowed us to use those art assets and we are thankful for this. 
 Having art assets that are optimized for the underlying hardware allows us to show off the capabilties of the graphics chip better.
 */

#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/gl.h>

#include "App.h"
#include "Mathematics.h"
#include "GraphicsDevice.h"
#include "UI.h"
#include "Macros.h"
#include "Geometry.h"
#include "Memory.h"
#include "Macros.h"
#include "Pathes.h"

#include <stdio.h>
#include <sys/time.h>

#include "stereoscope.h"



CDisplayText * AppDisplayText;
CTexture * Textures;
int iCurrentTick = 0, iStartTick = 0, iFps = 0, iFrames = 0;

int frames;
float frameRate;

/****************************************************************************
 ** DEFINES                                                                **
 ****************************************************************************/

/* Assuming a 4:3 aspect ratio: */
#define WIDTH 320
#define HEIGHT 480
#define CAM_ASPECT	((float)WIDTH / (float) HEIGHT)
#define CAM_NEAR	(1.0f)
#define CAM_FAR		(10000.0f)

#define SKYBOX_ZOOM			150.0f
#define SKYBOX_ADJUSTUVS	true

#ifndef PI
#define PI 3.14159f
#endif

/* Texture IDs */
GLuint balloonTex[7];
GLuint skyboxTex[6];

/* Print3D, Extension and POD Class Objects */
CPODScene		g_sScene;

/* View and Projection Matrices */
MATRIX	g_mView, g_mProj;

/* Skybox */
VERTTYPE* g_skyboxVertices;
VERTTYPE* g_skyboxUVs;

/* View Variables */
VERTTYPE fViewAngle;
VERTTYPE fViewDistance, fViewAmplitude, fViewAmplitudeAngle;
VERTTYPE fViewUpDownAmplitude, fViewUpDownAngle;

/* Vectors for calculating the view matrix and saving the camera position*/
VECTOR3 vTo, vUp, vCameraPosition;

/****************************************************************************
 ** Function Definitions
 ****************************************************************************/
void CameraGetMatrix();
void ComputeViewMatrix();
void DrawSkybox();
void DrawBalloon();
void CreateSkybox(float scale, bool adjustUV, int textureSize, VERTTYPE** Vertices, VERTTYPE** UVs);
void DestroySkybox(VERTTYPE* Vertices, VERTTYPE* UVs);


bool CShell::InitApplication()
{
	AppDisplayText = new CDisplayText;  
	Textures = new CTexture;
	
	if(AppDisplayText->SetTextures(WindowHeight, WindowWidth))
		printf("Display text textures loaded\n");

	/* Init values to defaults */
	fViewAngle = PIOVERTWO;
	
	fViewDistance = f2vt(100.0f);
	fViewAmplitude = f2vt(60.0f);
	fViewAmplitudeAngle = f2vt(0.0f);
	
	fViewUpDownAmplitude = f2vt(50.0f);
	fViewUpDownAngle = f2vt(0.0f);
	
	vTo.x = f2vt(0);
	vTo.y = f2vt(0);
	vTo.z = f2vt(0);
	
	vUp.x = f2vt(0);
	vUp.y = f2vt(1);
	vUp.z = f2vt(0);
	
	g_sScene.ReadFromMemory(c_STEREOSCOPE_H);
	
	char *buffer = new char[2048];
	
	GetResourcePathASCII(buffer, 2048);

//	NSString* readPath = [[NSBundle mainBundle] resourcePath];
//	[readPath getCString:buffer maxLength:2048 encoding:NSASCIIStringEncoding];
	
	int		i;
	
	/* Gets the Data Path */
	char		*filename = new char[2048];
	
	/******************************
	 ** Create Textures           **
	 *******************************/
	for (i=0; i<6; i++)
	{
		sprintf(filename, "%s/skybox%d.pvr", buffer, (i+1));
		if(!Textures->LoadTextureFromPVR(filename, &skyboxTex[i]))
		{
			printf("**ERROR** Failed to load texture for skybox.\n");
		}
		myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	
	sprintf(filename, "%s/CityHall_1024.pvr", buffer);
	if(!Textures->LoadTextureFromPVR(filename, &balloonTex[0]))
	{
		printf("**ERROR** Failed to load texture for Background.\n");
	}
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    sprintf(filename, "%s/Pavillon_1024.pvr", buffer);
	if(!Textures->LoadTextureFromPVR(filename, &balloonTex[1]))
	{
		printf("**ERROR** Failed to load texture for Background.\n");
	}
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    sprintf(filename, "%s/Ramp_1024.pvr", buffer);
	if(!Textures->LoadTextureFromPVR(filename, &balloonTex[2]))
	{
		printf("**ERROR** Failed to load texture for Background.\n");
	}
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    sprintf(filename, "%s/Window_On.pvr", buffer);
	if(!Textures->LoadTextureFromPVR(filename, &balloonTex[3]))
	{
		printf("**ERROR** Failed to load texture for Background.\n");
	}
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    sprintf(filename, "%s/Window_On.pvr", buffer);
	if(!Textures->LoadTextureFromPVR(filename, &balloonTex[4]))
	{
		printf("**ERROR** Failed to load texture for Background.\n");
	}
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    sprintf(filename, "%s/Window_On.pvr", buffer);
	if(!Textures->LoadTextureFromPVR(filename, &balloonTex[5]))
	{
		printf("**ERROR** Failed to load texture for Background.\n");
	}
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    sprintf(filename, "%s/Window_On.pvr", buffer);
	if(!Textures->LoadTextureFromPVR(filename, &balloonTex[6]))
	{
		printf("**ERROR** Failed to load texture for Background.\n");
	}
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	myglTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
	delete [] filename;
	delete [] buffer;
	
	/*********************/
	/* Create the skybox */
	/*********************/
	CreateSkybox( SKYBOX_ZOOM, SKYBOX_ADJUSTUVS, 512, &g_skyboxVertices, &g_skyboxUVs );
	
	/**********************
	 ** Projection Matrix **
	 **********************/
	
	/* Projection */
	MatrixPerspectiveFovRH(g_mProj, f2vt(PI / 6), f2vt(CAM_ASPECT), f2vt(CAM_NEAR), f2vt(CAM_FAR), true);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	myglMultMatrix(g_mProj.f);
	
	/******************************
	 ** GENERIC RENDER STATES     **
	 ******************************/
	
	/* The Type Of Depth Test To Do */
	glDepthFunc(GL_LEQUAL);
	
	/* Enables Depth Testing */
	glEnable(GL_DEPTH_TEST);
	
	/* Enables Smooth Color Shading */
	glShadeModel(GL_SMOOTH);
	
	/* Enable texturing */
	glEnable(GL_TEXTURE_2D);
	
	/* Define front faces */
	glFrontFace(GL_CW);
	
	/* Enables texture clamping */
	myglTexParameter( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	myglTexParameter( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	
	/* Sets the clear color */
	myglClearColor(f2vt(0.5f), f2vt(0.5f), f2vt(0.5f), 0);
	
	/* Reset the model view matrix to position the light */
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	/* Setup ambiant light */
    glEnable(GL_LIGHTING);
	VERTTYPE lightGlobalAmbient[] = {f2vt(0.4f), f2vt(0.4f), f2vt(0.4f), f2vt(1.0f)};
    myglLightModelv(GL_LIGHT_MODEL_AMBIENT, lightGlobalAmbient);
	
	/* Setup a directional light source */
	VERTTYPE lightPosition[] = {f2vt(+0.7f), f2vt(+1.0f), f2vt(-0.2f), f2vt(0.0f)};
    VERTTYPE lightAmbient[]  = {f2vt(0.6f), f2vt(0.6f), f2vt(0.6f), f2vt(1.0f)};
    VERTTYPE lightDiffuse[]  = {f2vt(1.0f), f2vt(1.0f), f2vt(1.0f), f2vt(1.0f)};
    VERTTYPE lightSpecular[] = {f2vt(1.0f), f2vt(1.0f), f2vt(1.0f), f2vt(1.0f)};
	
    glEnable(GL_LIGHT0);
    myglLightv(GL_LIGHT0, GL_POSITION, lightPosition);
    myglLightv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    myglLightv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    myglLightv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
	
	/* Setup the balloon material */
	VERTTYPE objectMatAmb[] = {f2vt(0.7f), f2vt(0.7f), f2vt(0.7f), f2vt(1.0f)};
	VERTTYPE objectMatDiff[] = {f2vt(1.0f), f2vt(1.0f), f2vt(1.0f), f2vt(1.0f)};
	VERTTYPE objectMatSpec[] = {f2vt(0.0f), f2vt(0.0f), f2vt(0.0f), f2vt(0.0f)};
	myglMaterialv(GL_FRONT_AND_BACK, GL_AMBIENT, objectMatAmb);
	myglMaterialv(GL_FRONT_AND_BACK, GL_DIFFUSE, objectMatDiff);
	myglMaterialv(GL_FRONT_AND_BACK, GL_SPECULAR, objectMatSpec);
	
	
	return true;
}

bool CShell::QuitApplication()
{
	AppDisplayText->ReleaseTextures();
	
	delete AppDisplayText;
	
	int i;
	
	/* Release all Textures */
	for (i = 0; i < 7; i++)
	{
		Textures->ReleaseTexture(balloonTex[i]);
	}
	for (i = 0; i < 6; i++)
	{
		Textures->ReleaseTexture(skyboxTex[i]);
	}
	
	/* Destroy the skybox */
	DestroySkybox( g_skyboxVertices, g_skyboxUVs );
	
	delete Textures;
	
	g_sScene.Destroy();

	return true;
}

bool CShell::InitView()
{
    glEnable(GL_DEPTH_TEST);
	glClearColor(0.3f, 0.3f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	return true;
}

bool CShell::ReleaseView()
{
	return true;
}

bool CShell::UpdateScene()
{
 	static struct timeval time = {0,0};
	struct timeval currTime = {0,0};
 
 	frames++;
	gettimeofday(&currTime, NULL); // gets the current time passed since the last frame in seconds
	
	if (currTime.tv_usec - time.tv_usec) 
	{
		frameRate = ((float)frames/((currTime.tv_usec - time.tv_usec) / 1000000.0f));
		AppDisplayText->DisplayText(0, 10, 0.4f, RGBA(255,255,255,255), "fps: %3.2f", frameRate);
		time = currTime;
		frames = 0;
	}

	return true;
}


bool CShell::RenderScene()
{
	/* Clear the depth and frame buffer */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	/* Set Z compare properties */
	glEnable(GL_DEPTH_TEST);
	
	/* Disable Blending*/
	glDisable(GL_BLEND);
	
	/* Calculate the model view matrix turning around the balloon */
	ComputeViewMatrix();
	
	/* Draw the skybox */
	DrawSkybox();
	
	/* Draw the balloon */
	DrawBalloon();
	
	// show text on the display
	AppDisplayText->DisplayDefaultTitle("Stereoscope", "Debug Info", eDisplayTextLogoNone);
	
	AppDisplayText->Flush();	
	
	return true;
}

/*******************************************************************************
 * Function Name  : ComputeViewMatrix
 * Description    : Calculate the view matrix turning around the balloon
 *******************************************************************************/
void ComputeViewMatrix()
{
	VECTOR3 vFrom;
	
	/* Calculate the distance to balloon */
	VERTTYPE distance = fViewDistance + VERTTYPEMUL(fViewAmplitude, SIN(fViewAmplitudeAngle));
	distance = VERTTYPEDIV(distance, f2vt(5.0f));
	fViewAmplitudeAngle += f2vt(.004f);
	
	/* Calculate the vertical position of the camera */
	VERTTYPE updown = VERTTYPEMUL(fViewUpDownAmplitude, SIN(fViewUpDownAngle));
	updown = VERTTYPEDIV(updown, f2vt(5.0f));
	fViewUpDownAngle += f2vt(0.005f);
	
	/* Calculate the angle of the camera around the balloon */
	vFrom.x = VERTTYPEMUL(distance, COS(fViewAngle));
	vFrom.y = updown;
	vFrom.z = VERTTYPEMUL(distance, SIN(fViewAngle));
	fViewAngle += f2vt(0.003f);
	
	/* Compute and set the matrix */
	MatrixLookAtRH(g_mView, vFrom, vTo, vUp);
	glMatrixMode(GL_MODELVIEW);
	myglLoadMatrix(g_mView.f);
	
	/* Remember the camera position to draw the skybox around it */
	vCameraPosition = vFrom;
}

/*******************************************************************************
 * Function Name  : DrawSkybox
 * Description    : Draws the skybox
 *******************************************************************************/
void DrawSkybox()
{
	/* Only use the texture color */
	myglTexEnv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	
	/* Draw the skybox around the camera position */
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	myglTranslate(-vCameraPosition.x, -vCameraPosition.y, -vCameraPosition.z);
	
	/* Disable lighting */
	glDisable(GL_LIGHTING);
	
	/* Enable backface culling for skybox; need to ensure skybox faces are set up properly */
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	
	/* Enable States */
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	
	for (int i = 0; i<6; i++)
	{
		/* Set Data Pointers */
		glBindTexture(GL_TEXTURE_2D, skyboxTex[i]);
		glVertexPointer(3, VERTTYPEENUM, sizeof(VERTTYPE)*3, &g_skyboxVertices[i*4*3]);
		glTexCoordPointer(2, VERTTYPEENUM, sizeof(VERTTYPE)*2, &g_skyboxUVs[i*4*2]);
		/* Draw */
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
	
	/* Disable States */
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	
	glPopMatrix();
}

/*******************************************************************************
 * Function Name  : DrawBalloon
 * Description    : Draws the balloon
 *******************************************************************************/
void DrawBalloon()
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	
	MATRIX worldMatrix;
	g_sScene.GetWorldMatrix(worldMatrix, g_sScene.pNode[0]);
	myglMultMatrix(worldMatrix.f);
	
	/* Modulate with vertex color */
	myglTexEnv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
	/* Enable lighting */
	glEnable(GL_LIGHTING);
	
	
	/* Enable back face culling */
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	
	/* Enable States */
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	
	/* Set Data Pointers */
	SPODMesh* mesh;

    int meshNo;
    for (meshNo=0;meshNo<7;meshNo++) {
        mesh = &g_sScene.pMesh[meshNo];
        
        /* Bind the Texture */
        glBindTexture(GL_TEXTURE_2D, balloonTex[meshNo]);

        // Used to display interleaved geometry
        glVertexPointer(3, VERTTYPEENUM, mesh->sVertex.nStride, mesh->pInterleaved + (long)mesh->sVertex.pData);
        glNormalPointer(VERTTYPEENUM, mesh->sNormals.nStride, mesh->pInterleaved + (long)mesh->sNormals.pData);
        glTexCoordPointer(2, VERTTYPEENUM, mesh->psUVW[0].nStride, mesh->pInterleaved + (long)mesh->psUVW[0].pData);
        
        
        /* Draw */
        glDrawElements(GL_TRIANGLES, mesh->nNumFaces*3, GL_UNSIGNED_SHORT, mesh->sFaces.pData);
    }
    
	/* Disable States */
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	
	glPopMatrix();
}


/*******************************************************************************
 * Function Name  : CameraGetMatrix
 * Global Used    :
 * Description    : Function to setup camera position
 *
 *******************************************************************************/
void CameraGetMatrix()
{
	VECTOR3	vFrom, vTo, vUp;
	VERTTYPE	fFOV;
	
	vUp.x = f2vt(0.0f);
	vUp.y = f2vt(1.0f);
	vUp.z = f2vt(0.0f);
	
	if(g_sScene.nNumCamera)
	{
		/* Get Camera data from POD Geometry File */
		fFOV = g_sScene.GetCameraPos(vFrom, vTo, 0);
		fFOV = VERTTYPEMUL(fFOV, f2vt(0.75f));		// Convert from horizontal FOV to vertical FOV (0.75 assumes a 4:3 aspect ratio)
	}
	else
	{
		fFOV = f2vt(PI / 6);
	}
	
	/* View */
	MatrixLookAtRH(g_mView, vFrom, vTo, vUp);
	
	/* Projection */
	MatrixPerspectiveFovRH(g_mProj, fFOV, f2vt(CAM_ASPECT), f2vt(CAM_NEAR), f2vt(CAM_FAR), true);
}

/*!***************************************************************************
 @Function			SetVertex
 @Modified			Vertices
 @Input				index
 @Input				x
 @Input				y
 @Input				z
 @Description		Writes a vertex in a vertex array
 *****************************************************************************/
void SetVertex(VERTTYPE** Vertices, int index, VERTTYPE x, VERTTYPE y, VERTTYPE z)
{
	(*Vertices)[index*3+0] = x;
	(*Vertices)[index*3+1] = y;
	(*Vertices)[index*3+2] = z;
}

/*!***************************************************************************
 @Function			SetUV
 @Modified			UVs
 @Input				index
 @Input				u
 @Input				v
 @Description		Writes a texture coordinate in a texture coordinate array
 *****************************************************************************/
void SetUV(VERTTYPE** UVs, int index, VERTTYPE u, VERTTYPE v)
{
	(*UVs)[index*2+0] = u;
	(*UVs)[index*2+1] = v;
}

/*!***************************************************************************
 @Function		PVRTCreateSkybox
 @Input			scale			Scale the skybox
 @Input			adjustUV		Adjust or not UVs for PVRT compression
 @Input			textureSize		Texture size in pixels
 @Output		Vertices		Array of vertices
 @Output		UVs				Array of UVs
 @Description	Creates the vertices and texture coordinates for a skybox
 *****************************************************************************/
void CreateSkybox(float scale, bool adjustUV, int textureSize, VERTTYPE** Vertices, VERTTYPE** UVs)
{
	*Vertices = new VERTTYPE[24*3];
	*UVs = new VERTTYPE[24*2];
	
	VERTTYPE unit = f2vt(1);
	VERTTYPE a0 = 0, a1 = unit;
	
	if (adjustUV)
	{
		VERTTYPE oneover = f2vt(1.0f / textureSize);
		a0 = VERTTYPEMUL(f2vt(4.0f), oneover);
		a1 = unit - a0;
	}
	
	// Front
	SetVertex(Vertices, 0, -unit, +unit, -unit);
	SetVertex(Vertices, 1, +unit, +unit, -unit);
	SetVertex(Vertices, 2, -unit, -unit, -unit);
	SetVertex(Vertices, 3, +unit, -unit, -unit);
	SetUV(UVs, 0, a0, a1);
	SetUV(UVs, 1, a1, a1);
	SetUV(UVs, 2, a0, a0);
	SetUV(UVs, 3, a1, a0);
	
	// Right
	SetVertex(Vertices, 4, +unit, +unit, -unit);
	SetVertex(Vertices, 5, +unit, +unit, +unit);
	SetVertex(Vertices, 6, +unit, -unit, -unit);
	SetVertex(Vertices, 7, +unit, -unit, +unit);
	SetUV(UVs, 4, a0, a1);
	SetUV(UVs, 5, a1, a1);
	SetUV(UVs, 6, a0, a0);
	SetUV(UVs, 7, a1, a0);
	
	// Back
	SetVertex(Vertices, 8 , +unit, +unit, +unit);
	SetVertex(Vertices, 9 , -unit, +unit, +unit);
	SetVertex(Vertices, 10, +unit, -unit, +unit);
	SetVertex(Vertices, 11, -unit, -unit, +unit);
	SetUV(UVs, 8 , a0, a1);
	SetUV(UVs, 9 , a1, a1);
	SetUV(UVs, 10, a0, a0);
	SetUV(UVs, 11, a1, a0);
	
	// Left
	SetVertex(Vertices, 12, -unit, +unit, +unit);
	SetVertex(Vertices, 13, -unit, +unit, -unit);
	SetVertex(Vertices, 14, -unit, -unit, +unit);
	SetVertex(Vertices, 15, -unit, -unit, -unit);
	SetUV(UVs, 12, a0, a1);
	SetUV(UVs, 13, a1, a1);
	SetUV(UVs, 14, a0, a0);
	SetUV(UVs, 15, a1, a0);
	
	// Top
	SetVertex(Vertices, 16, -unit, +unit, +unit);
	SetVertex(Vertices, 17, +unit, +unit, +unit);
	SetVertex(Vertices, 18, -unit, +unit, -unit);
	SetVertex(Vertices, 19, +unit, +unit, -unit);
	SetUV(UVs, 16, a0, a1);
	SetUV(UVs, 17, a1, a1);
	SetUV(UVs, 18, a0, a0);
	SetUV(UVs, 19, a1, a0);
	
	// Bottom
	SetVertex(Vertices, 20, -unit, -unit, -unit);
	SetVertex(Vertices, 21, +unit, -unit, -unit);
	SetVertex(Vertices, 22, -unit, -unit, +unit);
	SetVertex(Vertices, 23, +unit, -unit, +unit);
	SetUV(UVs, 20, a0, a1);
	SetUV(UVs, 21, a1, a1);
	SetUV(UVs, 22, a0, a0);
	SetUV(UVs, 23, a1, a0);
	
	for (int i=0; i<24*3; i++) (*Vertices)[i] = VERTTYPEMUL((*Vertices)[i], f2vt(scale));
}

/*!***************************************************************************
 @Function		PVRTDestroySkybox
 @Input			Vertices	Vertices array to destroy
 @Input			UVs			UVs array to destroy
 @Description	Destroy the memory allocated for a skybox
 *****************************************************************************/
void DestroySkybox(VERTTYPE* Vertices, VERTTYPE* UVs)
{
	delete [] Vertices;
	delete [] UVs;
}

