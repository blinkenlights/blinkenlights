//
//  StereoscopeAppDelegate.m
//  Stereoscope
//
//  Created by Martin Pittenauer on 06.09.08.
//  Copyright TheCodingMonkeys 2008. All rights reserved.
//

#import "StereoscopeAppDelegate.h"
#import "EAGLView.h"

@implementation StereoscopeAppDelegate

@synthesize window;
@synthesize glView;

- (void)applicationDidFinishLaunching:(UIApplication *)application {
    
	glView.animationInterval = 1.0 / 60.0;
	[glView startAnimation];
}


- (void)applicationWillResignActive:(UIApplication *)application {
	glView.animationInterval = 1.0 / 5.0;
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
	glView.animationInterval = 1.0 / 60.0;
}


- (void)dealloc {
	[window release];
	[glView release];
	[super dealloc];
}

@end
