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


@interface BlinkensenderPlugIn : QCPlugIn
{
	BlinkenImageProvider *_blinkenImageProvider;
	NSArray *_blinkenStructure;
	NSString *_BMLBaseDirectory;
	NSString *_targetAddress;
	BlinkenSender *_blinkenSender;
	NSTimeInterval _lastFrameTime;
	NSTimeInterval _minimumFrameTimeDifference;
}

@property (retain) NSArray  *blinkenStructure;
@property (retain) NSString *targetAddress;
@property (retain) NSString *BMLBaseDirectory;


@property (assign) NSString * inputTargetAddress;
@property (assign) id<QCPlugInInputImageSource> inputImage;
@property (assign) NSArray * inputBlinkenStructure;
@property double inputFPSCap;
//@property double inputBitsPerPixel;

@property (assign)id <QCPlugInOutputImageProvider> outputImage;
@property NSUInteger outputPixelWidth;
@property NSUInteger outputPixelHeight;
@property (assign) NSArray * outputBlinkenStructure;


@end
