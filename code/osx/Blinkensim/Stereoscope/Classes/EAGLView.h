//
//  EAGLView.h
//  Blinkenhall
//
//  Created by Dominik Wagner on 17.05.08.
//  Copyright TheCodingMonkeys 2008. All rights reserved.
//


#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import "Texture2D.h"

/*
This class wraps the CAEAGLLayer from CoreAnimation into a convenient UIView subclass.
The view content is basically an EAGL surface you render your OpenGL scene into.
Note that setting the view non-opaque will only work if the EAGL surface has an alpha channel.
*/
@interface EAGLView : UIView <UIAccelerometerDelegate> {
	
@private
	/* The pixel dimensions of the backbuffer */
	GLint backingWidth;
	GLint backingHeight;
	
	EAGLContext *context;
	
	/* OpenGL names for the renderbuffer and framebuffers used to render to this view */
	GLuint viewRenderbuffer, viewFramebuffer;
	
	/* OpenGL name for the depth buffer that is attached to viewFramebuffer, if it exists (0 if it does not exist) */
	GLuint depthRenderbuffer;
	
	NSTimer *animationTimer;
	NSTimeInterval animationInterval;
    
    CGPoint _displacement;
    CGFloat _pinchChange;

	UIAccelerationValue		_accelerometer[3];
	NSData *_lastFrameData;
	CGSize  _lastFrameSize;
	int     _numberOfChannels;
	CGFloat _maxFrameValue;
	
	NSMutableArray *_frameQueue;
	NSTimeInterval _maxTimeDifference;
	int            _timeSamplesTaken;
	NSTimeInterval _lastDrawTime;
	char displayState[23][54];
	
	Texture2D *_windowTexture;
	int _displayMode;
}

@property NSTimeInterval animationInterval;
@property (retain) NSData *lastFrameData;

- (void)startAnimation;
- (void)stopAnimation;
- (void)drawView;

@end
