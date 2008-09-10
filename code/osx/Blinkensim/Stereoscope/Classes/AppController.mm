//
//  Delegate.m
//  Skeleton
//
//  Created by Wolfgang Engel on 3/21/08.
//  Copyright __MyCompanyName__ 2008. All rights reserved.
//

#import "App.h"
#import "AppController.h"
#include "Camera.h"

//CONSTANTS:
#define kFPS			30.0
#define kSpeed			10.0

static CShell *shell = NULL;

@implementation AppController

@synthesize framerateLabel = _framerateLabel;
@synthesize loadingLabel   = _loadingLabel;
@synthesize infoButton     = _infoButton;
@synthesize window         = _window;
@synthesize mainNavigationController = _mainNavigationController;
@synthesize settingsController = _settingsController;

- (void)update
{
	// renderer entry
	if(!shell->InitView())
		printf("InitView error\n");
		
	if(!shell->UpdateScene())
		printf("UpdateScene error\n");
	
    if(!shell->RenderScene())
		printf("RenderScene error\n");
	
	[_glView swapBuffers];
	
	if(!shell->ReleaseView())
		printf("ReleaseView error\n");	
}

- (void)startRendering
{
	
	_updateTimer = [NSTimer scheduledTimerWithTimeInterval:(1.0 / kFPS) target:self selector:@selector(update) userInfo:nil repeats:YES];

}

- (void)stopRendering
{
	[_updateTimer invalidate];
	_updateTimer=nil;
}

- (void)applicationDidFinishLaunching:(UIApplication*)application
{
	CGRect	rect = [[UIScreen mainScreen] bounds];
		
	// create the OpenGL view and add it to the window
	//_glView = [[EAGLView alloc] initWithFrame:rect];
	_glView = [[EAGLView alloc] initWithFrame:rect pixelFormat:GL_RGB565_OES depthFormat:GL_DEPTH_COMPONENT16_OES preserveBackbuffer:NO];
	
	[_window addSubview:_glView];

	// add info button, fps and status label
	[_glView addSubview:_framerateLabel];
	[_glView addSubview:_loadingLabel];
	[_glView addSubview:_infoButton];

	// show the window
	[_window makeKeyAndVisible];
	
	if(!shell->InitApplication())
		printf("InitApplication error\n");
	
	[self startRendering];
}

#define InFramerateWeight 0.3
- (void)shellReportsFrameRate:(float)inCurrentFrameRate
{
	static float framerate = 30.0;
	framerate = inCurrentFrameRate * InFramerateWeight + framerate * (1.0 - InFramerateWeight);
	static int filter = 0;
	if (filter == 0) {
		_framerateLabel.text = [NSString stringWithFormat:@"%3.2f fps",framerate];

//		NSLog(@"%s %3.2f",__FUNCTION__,framerate);
	}
	filter = (filter + 1) % 10;
}

- (IBAction)showSettings:(id)inSender
{
//	[self.mainNavigationController presentModalViewController:self.mainNavigationController animated:YES];

	UIView *view = _mainNavigationController.view;
	CGRect	rect = [[UIScreen mainScreen] applicationFrame];
	CGPoint targetCenter = CGPointMake(CGRectGetMidX(rect),CGRectGetMidY(rect));
	view.center = CGPointMake(CGRectGetMidX(rect),CGRectGetMidY(rect) + view.bounds.size.height);
	[UIView beginAnimations:@"SettingsAnimation" context:NULL];
	[self.window addSubview:view];
	view.center = targetCenter;
	[UIView commitAnimations];
	[self stopRendering];
}

- (IBAction)doneWithSettings:(id)inSender
{
	UIView *view = _mainNavigationController.view;
	CGRect	rect = [[UIScreen mainScreen] applicationFrame];
	CGPoint targetCenter = CGPointMake(CGRectGetMidX(rect),CGRectGetMidY(rect) + view.bounds.size.height);
	[UIView beginAnimations:@"SettingsAnimation" context:NULL];
	[self.window addSubview:view];
	view.center = targetCenter;
	[UIView setAnimationDidStopSelector:@selector(settingsDidMoveOut:context:)];
	[UIView setAnimationDelegate:self];
	[UIView commitAnimations];
	[self startRendering];
}

- (void)settingsDidMoveOut:(id)animationId context:(void *)inContext
{
	[_mainNavigationController.view removeFromSuperview];
}

- (void)dealloc
{
	if(!shell->QuitApplication())
		printf("QuitApplication error\n");

	[_glView release];
	[_window release];
	
	[super dealloc];
}

@end
