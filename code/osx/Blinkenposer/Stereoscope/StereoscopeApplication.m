//
//  StereoscopeApplication.m
//  Blinkenposer
//
//  Created by Dominik Wagner on 23.08.08.
//  Copyright 2008 TheCodingMonkeys. All rights reserved.
//

#import "StereoscopeApplication.h"


@implementation StereoscopeApplication

+ (void)initialize
{
	// load quartz composer plugin
	NSString *poserPath = [[[NSBundle mainBundle] builtInPlugInsPath] stringByAppendingPathComponent:@"Blinkenposer.plugin"];
	
	BOOL success = [QCPlugIn loadPlugInAtPath:poserPath];
	NSLog(@"%s loading %@ was a %d",__FUNCTION__,poserPath,success);
}


@end
