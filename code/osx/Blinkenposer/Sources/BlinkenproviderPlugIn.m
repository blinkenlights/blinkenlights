/* It's highly recommended to use CGL macros instead of changing the current context for plug-ins that perform OpenGL rendering */
#import <OpenGL/CGLMacro.h>

#import "Blinkenlistener.h"
#import "BlinkenproviderPlugIn.h"
#import "BlinkenImageProvider.h"

#define	kQCPlugIn_Name				@"Blinkenprovider"
#define	kQCPlugIn_Description		@"This plugin will provide blinken images deliviered on port 2323"


@implementation BlinkenproviderPlugIn

@dynamic outputBlinkenImage;
@dynamic outputPixelWidth;
@dynamic outputPixelHeight;
@dynamic outputBlinkenStructure;

@dynamic inputUseProxyOption;
@dynamic inputListeningPort;
@dynamic inputProxyAddress;
@dynamic inputProxyPort;

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
    	@"inputUseProxyOption",
    	@"inputListeningPort",
    	@"inputProxyAddress",
    	@"inputProxyPort",
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

	if ([inKey isEqualToString:@"inputUseProxyOption"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                	@"Use Proxy", QCPortAttributeNameKey,
                	[NSNumber numberWithBool:NO], QCPortAttributeDefaultValueKey,
                	[NSArray arrayWithObjects:@"Listen to localhost on Listening Port",@"Ping and use Proxy Address and Port",nil],QCPortAttributeMenuItemsKey,
                	[NSNumber numberWithInt:0],QCPortAttributeMinimumValueKey,
                	[NSNumber numberWithInt:1],QCPortAttributeMaximumValueKey,
                nil];


	if ([inKey isEqualToString:@"inputListeningPort"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                	@"Local Listening Port", QCPortAttributeNameKey,
                	[NSNumber numberWithInt:2323], QCPortAttributeDefaultValueKey,
                	[NSNumber numberWithInt:0],QCPortAttributeMinimumValueKey,
                	[NSNumber numberWithInt:65535],QCPortAttributeMaximumValueKey,
                nil];

	if ([inKey isEqualToString:@"inputProxyAddress"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                	@"Proxy Address", QCPortAttributeNameKey,
                	@"localhost", QCPortAttributeDefaultValueKey,
                nil];

	if ([inKey isEqualToString:@"inputProxyPort"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                	@"Proxy Port", QCPortAttributeNameKey,
                	[NSNumber numberWithInt:4242], QCPortAttributeDefaultValueKey,
                	[NSNumber numberWithInt:0],QCPortAttributeMinimumValueKey,
                	[NSNumber numberWithInt:65535],QCPortAttributeMaximumValueKey,
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
//	NSLog(@"%s",__FUNCTION__);
	_stopListeningThread == YES;
	[_waitToStopLock lock];
	id tmp = _waitToStopLock;
	 _waitToStopLock = nil;
	[tmp unlock];
	[tmp release];
	
	[_blinkenImageProvider release];
	_blinkenImageProvider = nil;
	self.blinkenStructure = nil;
	
	[super dealloc];
}

@end

@implementation BlinkenproviderPlugIn (Execution)

- (void)startListeningThread
{
//	NSLog(@"%s",__FUNCTION__);
	[_waitToStopLock lock];
	_stopListeningThread = NO;
	[_waitToStopLock unlock];

	[NSThread detachNewThreadSelector:@selector(listenOnThread:) toTarget:self withObject:nil];
//	NSLog(@"%s end",__FUNCTION__);
}

- (void)stopListeningThread {
//	NSLog(@"%s",__FUNCTION__);
	_stopListeningThread = YES;
	// make sure thread is gone
	[_waitToStopLock lock];
	[_waitToStopLock unlock];
//	NSLog(@"%s end",__FUNCTION__);
}

- (BOOL)startExecution:(id<QCPlugInContext>)inContext
{
//	NSLog(@"%s",__FUNCTION__);
	/*
	Called by Quartz Composer when rendering of the composition starts: perform any required setup for the plug-in.
	Return NO in case of fatal failure (this will prevent rendering of the composition to start).
	*/
	_executedOnce = NO;
    		
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

    if ([self didValueForInputKeyChange:@"inputProxyAddress"] ||
    	[self didValueForInputKeyChange:@"inputProxyPort"] ||
    	[self didValueForInputKeyChange:@"inputListeningPort"] ||
    	[self didValueForInputKeyChange:@"inputUseProxyOption"] || !_executedOnce)
    {
    	if (self.inputUseProxyOption > 0 ) {
    		self.proxyAddress = [NSString stringWithFormat:@"%@:%d",self.inputProxyAddress,self.inputProxyPort];
    	} else {
    		self.proxyAddress = nil;
    		_listeningPort = self.inputListeningPort;
    	}
        [self stopListeningThread];
        [self startListeningThread];
    }

	if (_needsOutputUpdate)
	{
        NSRect imageBounds = [_blinkenImageProvider imageBounds];
		self.outputBlinkenImage = (imageBounds.size.width != 0 && imageBounds.size.height != 0) ? _blinkenImageProvider : nil;
		self.outputBlinkenStructure = self.blinkenStructure;
		self.outputPixelHeight = NSHeight(imageBounds);
		self.outputPixelWidth  = NSWidth(imageBounds);
//		NSLog(@"%s",__FUNCTION__);
		_needsOutputUpdate = NO;
	}

	_executedOnce = YES;
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
	_executedOnce = NO;

}

- (void)listenOnThread:(id)aSender
{	
	NSAutoreleasePool *pool = [NSAutoreleasePool new];
//	NSLog(@"%s",__FUNCTION__);
	if ([_waitToStopLock tryLock])
	{
		NSRunLoop *runLoop = [NSRunLoop currentRunLoop];
		BlinkenListener *blinkenListener = [BlinkenListener new];
		[blinkenListener setDelegate:self];
        [blinkenListener setProxyAddress:self.proxyAddress];
        if (!self.proxyAddress) [blinkenListener setPort:_listeningPort];
		[blinkenListener listen];
		
		while (!_stopListeningThread)
		{
			NSAutoreleasePool *innerPool = [NSAutoreleasePool new];
			[runLoop runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.25]];
			[innerPool release];
		}
		
		[blinkenListener stopListening];
		[blinkenListener setDelegate:nil];
		[blinkenListener release];

		[_waitToStopLock unlock];
	}
//	NSLog(@"%s end",__FUNCTION__);
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
