#import <OpenGL/CGLMacro.h>

#import "BlinkenListener.h"
#import "BlinkenSender.h"
#import "BlinkensenderPlugIn.h"

#define	kQCPlugIn_Name				@"Blinkensender"
#define	kQCPlugIn_Description		@"This plugin will send the image input via the Blinkenprotocol and/or generate bml files"



@implementation BlinkensenderPlugIn

@synthesize blinkenStructure = _blinkenStructure;
@synthesize targetAddress = _targetAddress;
@synthesize BMLBaseDirectory = _BMLBaseDirectory;

@dynamic inputTargetAddress;
@dynamic inputImage;
@dynamic inputFPSCap;
@dynamic inputBitsPerPixel;
@dynamic inputRecordToBML;
@dynamic inputOutputBaseDirectory;

@dynamic outputBlinkenImage;
@dynamic outputPixelWidth;
@dynamic outputPixelHeight;
@dynamic outputBlinkenStructure;


+ (NSArray*) sortedPropertyPortKeys {
    return [NSArray arrayWithObjects:
    
		@"inputTargetAddress",
		@"inputImage",
		@"inputFPSCap",
		@"inputBitsPerPixel",
		@"inputRecordToBML",
		@"inputOutputBaseDirectory",
    
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

//	if ([inKey isEqualToString:@"inputProxyAddress"])
//        return [NSDictionary dictionaryWithObjectsAndKeys:
//                	@"Proxy Address", QCPortAttributeNameKey,
//                nil];
	
	return nil;
}

+ (QCPlugInExecutionMode)executionMode
{
	/*
	Return the execution mode of the plug-in: kQCPlugInExecutionModeProvider, kQCPlugInExecutionModeProcessor, or kQCPlugInExecutionModeConsumer.
	*/
	
	return kQCPlugInExecutionModeProcessor;
}

+ (QCPlugInTimeMode)timeMode
{
	/*
	Return the time dependency mode of the plug-in: kQCPlugInTimeModeNone, kQCPlugInTimeModeIdle or kQCPlugInTimeModeTimeBase.
	*/
	
	return kQCPlugInTimeModeNone;
}

- (id)init
{
	if (self = [super init]) {
		
	}
	
	return self;
}

- (void)dealloc
{

	[_blinkenImageProvider release];
	_blinkenImageProvider = nil;
	self.blinkenStructure = nil;
	
	[super dealloc];
}



- (BOOL)startExecution:(id<QCPlugInContext>)inContext
{
//	NSLog(@"%s",__FUNCTION__);
	/*
	Called by Quartz Composer when rendering of the composition starts: perform any required setup for the plug-in.
	Return NO in case of fatal failure (this will prevent rendering of the composition to start).
	*/
    		
	return YES;
}

- (void)enableExecution:(id<QCPlugInContext>)inContext
{
	/*
	Called by Quartz Composer when the plug-in instance starts being used by Quartz Composer.
	*/
	_blinkenSender = [BlinkenSender new];
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

	static int frameCapper = 0;
	static int valueBase = 0;
	if (frameCapper == 0) {
	
		// generate data
		NSMutableArray *blinkenStructure = [NSMutableArray array];
		int x = 0, y=0;
		int value = valueBase;
		for (y = 0; y<32; y++) {
			NSMutableArray *blinkenRow = [NSMutableArray array];
			for (x = 0; x<96; x++) {
				[blinkenRow addObject:[NSNumber numberWithInt:value]];
				value = (value + 1) % 16;
			}
			[blinkenStructure addObject:blinkenRow];
			value = (value + 1) % 16;
		}
	
		[_blinkenSender sendBlinkenStructure:blinkenStructure];
	} 
	
	valueBase++;
	
	frameCapper = (frameCapper + 1) % 4;
	frameCapper = 0;
	
	return YES;
}

- (void)disableExecution:(id<QCPlugInContext>)inContext
{
	/*
	Called by Quartz Composer when the plug-in instance stops being used by Quartz Composer.
	*/
	[_blinkenSender release];
	_blinkenSender = nil;
}

- (void) stopExecution:(id<QCPlugInContext>)context
{
//	NSLog(@"%s",__FUNCTION__);
	/*
	Called by Quartz Composer when rendering of the composition stops: perform any required cleanup for the plug-in.
	*/
}


@end
