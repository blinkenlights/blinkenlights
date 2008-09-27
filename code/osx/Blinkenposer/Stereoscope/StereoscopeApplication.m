//
//  StereoscopeApplication.m
//  Blinkenposer
//
//  Created by Dominik Wagner on 23.08.08.
//  Copyright 2008 TheCodingMonkeys. All rights reserved.
//

#import "StereoscopeApplication.h"
#import "StereoscopeAppController.h"

@implementation StereoscopeApplication

+ (void)initialize
{
	// load quartz composer plugin
	NSString *poserPath = [[[NSBundle mainBundle] builtInPlugInsPath] stringByAppendingPathComponent:@"Blinkenposer.plugin"];

	int installedVersion = [StereoscopeAppController installedBlinkenposerPluginVersion];
	NSLog(@"%s installedVersion %d %@",__FUNCTION__,installedVersion,[QCPlugIn attributes]);
	if (installedVersion == -1) {
		BOOL success = [QCPlugIn loadPlugInAtPath:poserPath];
//		NSLog(@"%s loading %@ was a %d",__FUNCTION__,poserPath,success);
	} else {
//		NSLog(@"%s installed version was %d",__FUNCTION__,installedVersion);
	}
}


@end
