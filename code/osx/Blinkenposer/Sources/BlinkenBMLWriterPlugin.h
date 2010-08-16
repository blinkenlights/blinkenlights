//
//  BlinkensenderPlugIn.h
//  Blinkenposer
//
//  Created by Dominik Wagner on 12.07.08.
//  Copyright 2008 TheCodingMonkeys. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <Quartz/Quartz.h>

@class BlinkenImageProvider, BlinkenSender;


@interface BlinkenBMLWriterPlugIn : QCPlugIn
{
	NSArray *_blinkenStructure;
	NSString *_BMLBaseDirectory;
	
	NSTimeInterval _lastFrameTime;
	NSTimeInterval _currentFrameTime;
	NSTimeInterval _minimumFrameTimeDifference;
	NSFileHandle *_writingFileHandle;
	
	BOOL _blinkenInputChanged;
	BOOL _renderedOnce;
}

@property (retain) NSArray  *blinkenStructure;
@property (retain) NSString *BMLBaseDirectory;
@property (retain) NSFileHandle *writingFileHandle;

@property (assign) NSArray * inputBlinkenStructure;
@property double inputFPSCap;
@property BOOL inputColor;
@property (assign) NSString *inputBaseDirectory;


@end
