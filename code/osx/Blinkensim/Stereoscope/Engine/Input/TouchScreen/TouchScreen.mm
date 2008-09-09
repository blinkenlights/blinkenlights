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
#import "TouchScreen.h"
#include "Mathematics.h"
#include "UI.h"


id idFrame;

extern CDisplayText * AppDisplayText;

// C wrapper
TouchScreenValues* GetValuesTouchScreen()
{
	return [idFrame GetValuesTouchScreen];
}


//void UpdatePolarCamera()
//{
//	[idFrame UpdateCamera];
//}

@implementation EAGLCameraView

- (id) initWithFrame:(CGRect)frame pixelFormat:(GLuint)format depthFormat:(GLuint)depth preserveBackbuffer:(BOOL)retained
{
	if((self = [super initWithFrame:(CGRect)frame pixelFormat:(GLuint)format depthFormat:(GLuint)depth preserveBackbuffer:(BOOL)retained])) 
	{
		[self setMultipleTouchEnabled:YES];

		TouchScreen.TouchesEnd = true;
	}
	
	idFrame = self; 
    return idFrame;
}

-(TouchScreenValues*)GetValuesTouchScreen
{
	return &TouchScreen;
}


- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	UITouch* touch = [touches anyObject];
	
	CGPoint	 location = [touch locationInView:self];
    TouchScreen.LocationXTouchesBegan = location.x;
    TouchScreen.LocationYTouchesBegan = location.y;
	TouchScreen.CountTouchesBegan = (int) [touches count];
	TouchScreen.TapCountTouchesBegan = [touch tapCount];
	
	TouchScreen.TouchesEnd = false;
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	TouchScreen.TouchesEnd = true;
    TouchScreen.LocationXTouchesBegan = 0.0;
    TouchScreen.LocationYTouchesBegan = 0.0;
	TouchScreen.CountTouchesBegan = (int) 0;
	TouchScreen.TapCountTouchesBegan = 0;
	
    TouchScreen.LocationXTouchesMoved = 0.0;
    TouchScreen.LocationYTouchesMoved =  0.0;
	TouchScreen.CountTouchesMoved = (int) 0;
	TouchScreen.TapCountTouchesMoved = 0;
	
	
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	UITouch* touch = [touches anyObject];

	CGPoint	 location = [touch locationInView:self];
    TouchScreen.LocationXTouchesMoved = location.x;
    TouchScreen.LocationYTouchesMoved = location.y;
	TouchScreen.CountTouchesMoved = (int) [touches count];
	TouchScreen.TapCountTouchesMoved = [touch tapCount];
	
	TouchScreen.TouchesEnd = false;
}

@end



