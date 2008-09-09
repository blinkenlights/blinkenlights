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
#import "RotateObject.h"
#include "Mathematics.h"
#include "UI.h"


id idFrame;

extern CDisplayText * AppDisplayText;


void UpdatePolarCamera()
{
	[idFrame UpdateCamera];
}

@implementation EAGLCameraView

//- (id) initWithFrame:(CGRect)frame
- (id) initWithFrame:(CGRect)frame pixelFormat:(GLuint)format depthFormat:(GLuint)depth preserveBackbuffer:(BOOL)retained
//- (id) initWithFrame:(CGRect)frame pixelFormat:(EAGLPixelFormat)format depthFormat:GL_DEPTH_COMPONENT16_OES preserveBackbuffer:NO
{
	if((self = [super initWithFrame:(CGRect)frame pixelFormat:(GLuint)format depthFormat:(GLuint)depth preserveBackbuffer:(BOOL)retained])) 
	{
		[self setMultipleTouchEnabled:YES];

    [self resetView];
	}
	
	idFrame = self; 
    return idFrame;
}

- (void) resetView
{
    mCameraTheta = 45;
    mCameraPhi = 30;
    mCameraDistance = 7;
    mCameraOffsetX = 0;
    mCameraOffsetY = 0;
    mCameraFOV = 70;

    mStartCameraTheta = 0;
    mStartCameraPhi = 0;
    mStartCameraDistance = 0;
    mStartCameraOffsetX = 0;
    mStartCameraOffsetY = 0;

    mMouseX = 0;
    mMouseY = 0;
    mMouseDownX = 0;
    mMouseDownY = 0;
}

-(void) UpdateCamera
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	
	MATRIX	MyPerspMatrix;
	MatrixPerspectiveFovRH(MyPerspMatrix, f2vt(mCameraFOV), f2vt(((float) 320 / (float) 480)), f2vt(0.1f), f2vt(1000.0f), 0);
	myglMultMatrix(MyPerspMatrix.f);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	myglTranslate(mCameraOffsetX, mCameraOffsetY, -mCameraDistance);

    myglRotate(mCameraPhi,   1, 0, 0);
    myglRotate(mCameraTheta, 0, 0, 1);
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	UITouch* touch = [touches anyObject];
	
	int count = (int) [touches count];

	AppDisplayText->DisplayText(0, 10, 0.4f, RGBA(255,255,255,255), "touchesBegan: count: %d", count);
	
	// location
	CGPoint	location = [touch locationInView:self];

	//If the user double-taps, clear the whole painting
	if([touch tapCount] >= 2)
	{
		AppDisplayText->DisplayText(0, 18, 0.4f, RGBA(255,255,255,255), "Reset view through double tap", 0);
		[self resetView];
	}
    mMouseDownX = location.x;
    mMouseDownY = location.y;
    mButtonDown = count;
    
    mStartCameraTheta    = mCameraTheta;
    mStartCameraPhi      = mCameraPhi;
    mStartCameraDistance = mCameraDistance;
    mStartCameraOffsetX  = mCameraOffsetX;
    mStartCameraOffsetY  = mCameraOffsetY;
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    mButtonDown = 0;    
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	UITouch* touch = [touches anyObject];
	
	// location
	CGPoint	 location = [touch locationInView:self];
    float x = location.x;
    float y = location.y;

	int count = (int) [touches count];
	
	AppDisplayText->DisplayText(0, 14, 0.4f, RGBA(255,255,255,255), "touchesMoved: count: %d", count);
    
    enum tMode
    {
        kModeRotate,
        kModePanXY,
        kModeZoomX,
        kModeZoomY,
        kModeZoomXY,
        kModeNone
    };

    int mode = kModeNone;

    if (count == 3)
        mode = kModePanXY;
    else if (count == 2)
        mode = kModeZoomXY;
    else if (count == 1)
        mode = kModeRotate;

    float dx = -(x - mMouseDownX) / 400.0;
    float dy = (y - mMouseDownY) / 400.0;

    switch (mode)
    {
    case kModePanXY:
        mCameraOffsetX = mStartCameraOffsetX + 4.0 * dx;
        mCameraOffsetY = mStartCameraOffsetY - 4.0 * dy;
        break;
        
    case kModeZoomX:
        mCameraDistance = mStartCameraDistance + 4.0 * dx;
        break;
        
    case kModeZoomY:
        mCameraDistance = mStartCameraDistance - 4.0 * dy;
        break;
        
    case kModeZoomXY:
        mCameraDistance = mStartCameraDistance + 4.0 * (dx - dy);
        break;
        
    case kModeRotate:
        mCameraTheta = mStartCameraTheta + dx * 180;
        mCameraPhi   = mStartCameraPhi   + dy * 180;
        if (mCameraTheta < -90) 
            mCameraTheta = -90;
        if (mCameraTheta >  90) 
            mCameraTheta =  90;
        break;
    }
}

@end



