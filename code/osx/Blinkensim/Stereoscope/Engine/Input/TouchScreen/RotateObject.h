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

#import "EAGLView.h"

@class EAGLCameraView;

////////////////////////////////////////////////////////////////////////////////
// Provides a UIGLView with built-in view controls:
//
//   single-finger drag: rotate around and up/down
//   double-finger drag (or anchor one finger and drag the second up and down): zoom in/out
//   triple-finger drag (anchor two fingers, drag the other left/right or up/down): translate viewpoint
//   two-finger tap: reset view
//
// In drawRect, call positionCamera when you need to set the camera matrices.
@interface EAGLCameraView : EAGLView
//@interface UIGLCameraView : UIGLView
{
    bool    mZUp;               // Set to 0 or 1 to indicate whether we do Z-up or Y-up
    
    // Camera internals
    GLfloat  mCameraTheta;
    GLfloat  mCameraPhi;
    GLfloat  mCameraDistance;
    GLfloat  mCameraOffsetX;
    GLfloat  mCameraOffsetY;
    GLfloat  mCameraFOV;

    // dragging internals
    GLfloat  mStartCameraTheta;
    GLfloat  mStartCameraPhi;
    GLfloat  mStartCameraDistance;
    GLfloat  mStartCameraOffsetX;
    GLfloat  mStartCameraOffsetY;

    int      mButtonDown;
    
    float    mMouseX;
    float    mMouseY;
    float    mMouseDownX;
    float    mMouseDownY;
	
//	TouchScreen* gTouch;
}

- (void) UpdateCamera;
// Call to set camera matrices

- (void) resetView;
// override this to provide your own initial settings for mCamera* above.
// also, call to manually reset the view.

@end

void UpdatePolarCamera(void);
