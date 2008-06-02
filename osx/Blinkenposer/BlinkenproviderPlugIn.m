//
//  BlinkenposerPlugIn.m
//  Blinkenposer
//
//  Created by Dominik Wagner on 31.05.08.
//  Copyright (c) 2008 TheCodingMonkeys. All rights reserved.
//

/* It's highly recommended to use CGL macros instead of changing the current context for plug-ins that perform OpenGL rendering */
#import <OpenGL/CGLMacro.h>

#import "Blinkenlistener.h"
#import "BlinkenproviderPlugIn.h"

#define	kQCPlugIn_Name				@"Blinkenprovider"
#define	kQCPlugIn_Description		@"This plugin will provide blinken images deliviered on port 2323"

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

@implementation BlinkenImageProvider

@synthesize frameData = _frameData;

- (void)dealloc
{
	self.frameData = nil;
	[super dealloc];
}

- (void)setFrameData:(NSData *)inFrameData size:(CGSize)inSize channels:(int)inChannels maxValue:(unsigned char)inMaxValue
{
	self.frameData = inFrameData;
	_frameSize = NSSizeFromCGSize(inSize);
	_numberOfChannels = inChannels;
	_maxValue = inMaxValue;
}

- (BOOL)canRenderWithCGLContext:(CGLContextObj)cgl_ctx
{
	return NO;
}

- (NSRect)imageBounds
{
//	NSLog(@"%s",__FUNCTION__);
	return NSMakeRect(0.,0.,_frameSize.width,_frameSize.height);
}

- (BOOL)renderToBuffer:(void*)inBaseAddress withBytesPerRow:(NSUInteger)inRowBytes pixelFormat:(NSString*)inFormat forBounds:(NSRect)inBounds
{
//	NSLog(@"%s",__FUNCTION__);
	BOOL isBGRA = [QCPlugInPixelFormatBGRA8 isEqualToString:inFormat];

	unsigned char *src, *dest;
	src = (unsigned char *)[self.frameData bytes];
	dest = inBaseAddress;
	NSUInteger myRowBytes = _frameSize.width;

    NSUInteger numberOfRows = inBounds.size.height;

	unsigned char alpha = 255;

    
    int currentRow = inBounds.origin.y;
    for (currentRow = inBounds.origin.y;currentRow < numberOfRows;currentRow++) {
		unsigned char *sourceRow = src + currentRow * myRowBytes;
		unsigned char *destRow = dest + currentRow * inRowBytes;
        int x = 0;
        for (x=0;x<inBounds.size.width;x++) {
        
            unsigned char grayValue = (*sourceRow) * 255. / _maxValue;
            sourceRow++;
            if (isBGRA) {
                *destRow = grayValue; destRow++;  
                *destRow = grayValue; destRow++; 
                *destRow = grayValue; destRow++; 
                *destRow = alpha;     destRow++; 
            } else {
                *destRow = alpha;     destRow++; 
                *destRow = grayValue; destRow++;  
                *destRow = grayValue; destRow++;  
                *destRow = grayValue; destRow++;  
            }
        }
    }	
	return YES;
}

- (CGColorSpaceRef) imageColorSpace {
	return [[NSColorSpace genericRGBColorSpace] CGColorSpace];
}


- (NSArray*) supportedBufferPixelFormats
{
	NSArray *result = [NSArray arrayWithObjects:QCPlugInPixelFormatARGB8,QCPlugInPixelFormatBGRA8, nil];
	return result;
}

@end

@implementation BlinkenproviderPlugIn

@dynamic outputBlinkenImage;
@dynamic outputPixelWidth;
@dynamic outputPixelHeight;
@dynamic outputBlinkenStructure;
@dynamic inputProxyAddress;

@synthesize proxyAddress = _proxyAddress;
@synthesize blinkenStructure = _blinkenStructure;
/*
Here you need to declare the input / output properties as dynamic as Quartz Composer will handle their implementation
@dynamic inputFoo, outputBar;
*/

+ (NSArray*) sortedPropertyPortKeys {
    return [NSArray arrayWithObjects:
    	@"outputBlinkenImage",
    	@"outputPixelWidth",
    	@"outputPixelHeight",
    	@"outputBlinkenStructure",
    nil];
}

+ (NSDictionary*) attributes
{
	/*
	Return a dictionary of attributes describing the plug-in (QCPlugInAttributeNameKey, QCPlugInAttributeDescriptionKey...).
	*/
	
	return [NSDictionary dictionaryWithObjectsAndKeys:kQCPlugIn_Name, QCPlugInAttributeNameKey, kQCPlugIn_Description, QCPlugInAttributeDescriptionKey, nil];
}

+ (NSDictionary*) attributesForPropertyPortWithKey:(NSString*)inKey
{
	/*
	Specify the optional attributes for property based ports (QCPortAttributeNameKey, QCPortAttributeDefaultValueKey...).
	*/
	if ([inKey isEqualToString:@"outputBlinkenImage"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                	@"Blinken Image", QCPortAttributeNameKey,
                nil];

	if ([inKey isEqualToString:@"outputBlinkenStructure"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                	@"Blinken Structure", QCPortAttributeNameKey,
                nil];

	if ([inKey isEqualToString:@"outputPixelWidth"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                	@"Pixel Width", QCPortAttributeNameKey,
                nil];

	if ([inKey isEqualToString:@"outputPixelHeight"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                	@"Pixel Height", QCPortAttributeNameKey,
                nil];

	if ([inKey isEqualToString:@"inputProxyAddress"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                	@"Proxy Address", QCPortAttributeNameKey,
                nil];
	
	return nil;
}

+ (QCPlugInExecutionMode)executionMode
{
	/*
	Return the execution mode of the plug-in: kQCPlugInExecutionModeProvider, kQCPlugInExecutionModeProcessor, or kQCPlugInExecutionModeConsumer.
	*/
	
	return kQCPlugInExecutionModeProvider;
}

+ (QCPlugInTimeMode)timeMode
{
	/*
	Return the time dependency mode of the plug-in: kQCPlugInTimeModeNone, kQCPlugInTimeModeIdle or kQCPlugInTimeModeTimeBase.
	*/
	
	return kQCPlugInTimeModeIdle;
}

- (id)init
{
	if (self = [super init]) {
		/*
		Allocate any permanent resource required by the plug-in.
		*/
		_waitToStopLock = [NSLock new];
	}
	
	return self;
}

- (void)dealloc
{
	/*
	Release any resources created in -init.
	*/
	
	if (![_waitToStopLock tryLock] && _stopListeningThread == YES)
	{
		[_waitToStopLock lock];
	}
	[_waitToStopLock unlock];
	[_waitToStopLock release];
	_waitToStopLock = nil;
	[_blinkenImageProvider release];
	_blinkenImageProvider = nil;
	self.blinkenStructure = nil;
	
	[super dealloc];
}

@end

@implementation BlinkenproviderPlugIn (Execution)

- (void)startListeningThread
{
	if (![_waitToStopLock tryLock])
	{
		[_waitToStopLock lock];
	}
	[_waitToStopLock unlock];
	_stopListeningThread = NO;

	[NSThread detachNewThreadSelector:@selector(listenOnThread:) toTarget:self withObject:nil];

}

- (void)stopListeningThread {
	_stopListeningThread = YES;
}

- (BOOL)startExecution:(id<QCPlugInContext>)inContext
{
//	NSLog(@"%s",__FUNCTION__);
	/*
	Called by Quartz Composer when rendering of the composition starts: perform any required setup for the plug-in.
	Return NO in case of fatal failure (this will prevent rendering of the composition to start).
	*/

    [self startListeningThread];
    		
	return YES;
}

- (void)enableExecution:(id<QCPlugInContext>)inContext
{
	/*
	Called by Quartz Composer when the plug-in instance starts being used by Quartz Composer.
	*/
}

- (BOOL)execute:(id<QCPlugInContext>)inContext atTime:(NSTimeInterval)time withArguments:(NSDictionary*)arguments
{
	/*
	Called by Quartz Composer whenever the plug-in instance needs to execute.
	Only read from the plug-in inputs and produce a result (by writing to the plug-in outputs or rendering to the destination OpenGL context) within that method and nowhere else.
	Return NO in case of failure during the execution (this will prevent rendering of the current frame to complete).
	
	The OpenGL context for rendering can be accessed and defined for CGL macros using:
	CGLContextObj cgl_ctx = [context CGLContextObj];
	*/

    if ([self didValueForInputKeyChange:@"inputProxyAddress"])
    {
        self.proxyAddress = self.inputProxyAddress;
        if ([self.proxyAddress length] == 0) self.proxyAddress = nil;
        [self stopListeningThread];
        [self startListeningThread];
    }

	if (_needsOutputUpdate)
	{
		self.outputBlinkenImage = _blinkenImageProvider;
		self.outputBlinkenStructure = self.blinkenStructure;
		self.outputPixelHeight = NSHeight([_blinkenImageProvider imageBounds]);
		self.outputPixelWidth  = NSWidth([_blinkenImageProvider imageBounds]);
//		NSLog(@"%s",__FUNCTION__);
		_needsOutputUpdate = NO;
	}

	
	return YES;
}

- (void)disableExecution:(id<QCPlugInContext>)inContext
{
	/*
	Called by Quartz Composer when the plug-in instance stops being used by Quartz Composer.
	*/
}

- (void) stopExecution:(id<QCPlugInContext>)context
{
//	NSLog(@"%s",__FUNCTION__);
	/*
	Called by Quartz Composer when rendering of the composition stops: perform any required cleanup for the plug-in.
	*/
    [self stopListeningThread];
}

- (void)listenOnThread:(id)aSender
{	
	NSAutoreleasePool *pool = [NSAutoreleasePool new];
	if ([_waitToStopLock tryLock])
	{
		NSRunLoop *runLoop = [NSRunLoop currentRunLoop];
		BlinkenListener *blinkenListener = [BlinkenListener new];
		[blinkenListener setDelegate:self];
//		[blinkenListener setPort:4561];
//        [blinkenListener setProxyAddress:@"matrix.blinkenlights.de"];
        [blinkenListener setProxyAddress:self.proxyAddress];
		[blinkenListener listen];
		
		while (!_stopListeningThread)
		{
			NSAutoreleasePool *innerPool = [NSAutoreleasePool new];
			[runLoop runUntilDate:[NSDate dateWithTimeIntervalSinceNow:1.]];
			[innerPool release];
		}
		
		[blinkenListener stopListening];
		[blinkenListener setDelegate:nil];
		[blinkenListener release];

		[_waitToStopLock unlock];
	}
	[pool release];
}

- (void)receivedFrameData:(NSData *)inFrameData ofSize:(CGSize)inSize channels:(int)inChannels maxValue:(unsigned char)inMaxValue
{
//	NSLog(@"%s",__FUNCTION__);

	if (!_blinkenImageProvider)
	{
		_blinkenImageProvider = [BlinkenImageProvider new];
	}
	[_blinkenImageProvider setFrameData:inFrameData size:inSize channels:inChannels maxValue:inMaxValue];
	
	unsigned char * bytes = (unsigned char *)[inFrameData bytes];
	
	NSMutableArray *blinkenStructure = [NSMutableArray array];
	int y = 0;
	for (y=0;y<inSize.height;y++)
	{
		NSMutableArray *blinkenRow = [NSMutableArray array];
		int x = 0;
		for (x=0;x<inSize.width;x++)
		{
			[blinkenRow addObject:[NSNumber numberWithInt:(*bytes++)/(float)inMaxValue * 15.]];
		}
		[blinkenStructure addObject:blinkenRow];
	}
	self.blinkenStructure = blinkenStructure;
	
	_needsOutputUpdate = YES;
}


@end
