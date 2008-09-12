//
//  BlinkenposerPlugIn.h
//  Blinkenposer
//
//  Created by Dominik Wagner on 31.05.08.
//  Copyright (c) 2008 TheCodingMonkeys. All rights reserved.
//

#import <Quartz/Quartz.h>

@class BlinkenListener,BlinkenImageProvider;

@interface BlinkenproviderPlugIn : QCPlugIn
{
	BOOL _needsOutputUpdate;
	BlinkenImageProvider *_blinkenImageProvider;
	BOOL _stopListeningThread;
	NSLock *_waitToStopLock;
	NSArray *_blinkenStructure;
	NSArray *_screenMetadata;
	NSString *_proxyAddress;
	NSInteger _listeningPort;
	BOOL _executedOnce;
}

/*
Declare here the Obj-C 2.0 properties to be used as input and output ports for the plug-in e.g.
@property double inputFoo;
@property(assign) NSString* outputBar;
You can access their values in the appropriate plug-in methods using self.inputFoo or self.inputBar
*/
@property (retain) NSArray *blinkenStructure;
@property (retain) NSArray *screenMetadata;
@property (retain) NSString *proxyAddress;

@property (assign)id <QCPlugInOutputImageProvider> outputBlinkenImage;
@property NSUInteger outputPixelWidth;
@property NSUInteger outputPixelHeight;
@property (assign) NSArray *outputBlinkenStructure;
@property (assign) NSArray *outputScreenMetadata;

@property NSUInteger inputUseProxyOption;
@property NSUInteger inputListeningPort;
@property (assign) NSString *inputProxyAddress;
@property NSUInteger inputProxyPort;

@end
