//
//  MCUSetupAppDelegate.m
//  MCUSetup
//
//  Created by Dominik Wagner on 29.09.08.
//  Copyright TheCodingMonkeys 2008. All rights reserved.
//

#import "MCUSetupAppDelegate.h"
#import "RootViewController.h"


@implementation MCUSetupAppDelegate

@synthesize window;
@synthesize navigationController;


- (void)applicationDidFinishLaunching:(UIApplication *)application {
	
	// Configure and show the window
	[window addSubview:[navigationController view]];
	[window makeKeyAndVisible];
}


- (void)applicationWillTerminate:(UIApplication *)application {
	// Save data if appropriate
}


- (void)dealloc {
	[navigationController release];
	[window release];
	[super dealloc];
}

@end
