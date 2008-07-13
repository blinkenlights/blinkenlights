//
//  BlinkenImageProvider.h
//  Blinkenposer
//
//  Created by Dominik Wagner on 12.07.08.
//  Copyright 2008 TheCodingMonkeys. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <Quartz/Quartz.h>

@interface BlinkenImageProvider : NSObject <QCPlugInOutputImageProvider>
{
	NSData *_frameData;
	NSSize _frameSize;
	int    _numberOfChannels;
	unsigned char _maxValue;
}

@property (retain) NSData *frameData;

- (void)setFrameData:(NSData *)inFrameData size:(CGSize)inSize channels:(int)inChannels maxValue:(unsigned char)inMaxValue;

@end

