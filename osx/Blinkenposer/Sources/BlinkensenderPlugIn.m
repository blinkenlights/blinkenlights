#import <OpenGL/CGLMacro.h>

#import "BlinkenListener.h"
#import "BlinkenSender.h"
#import "BlinkensenderPlugIn.h"
#import "BlinkenImageProvider.h"

#define	kQCPlugIn_Name				@"Blinkensender"
#define	kQCPlugIn_Description		@"This plugin will send the image input via the Blinkenprotocol and/or generate bml files"



@implementation BlinkensenderPlugIn

@synthesize blinkenStructure = _blinkenStructure;
@synthesize targetAddress = _targetAddress;
@synthesize BMLBaseDirectory = _BMLBaseDirectory;

@dynamic inputTargetAddress;
@dynamic inputImage;
@dynamic inputBlinkenStructure;
@dynamic inputFPSCap;
@dynamic inputBitsPerPixel;

@dynamic outputImage;
@dynamic outputPixelWidth;
@dynamic outputPixelHeight;
@dynamic outputBlinkenStructure;


+ (NSArray*) sortedPropertyPortKeys {
    return [NSArray arrayWithObjects:
    
		@"inputTargetAddress",
		@"inputImage",
		@"inputBlinkenStructure",
		@"inputFPSCap",
		@"inputBitsPerPixel",
    
    	@"outputImage",
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
	if ([inKey isEqualToString:@"outputImage"])
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

	if ([inKey isEqualToString:@"inputTargetAddress"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                	@"Target Address", QCPortAttributeNameKey,
                	@"localhost:2323", QCPortAttributeDefaultValueKey,
                nil];

	if ([inKey isEqualToString:@"inputImage"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                	@"Image", QCPortAttributeNameKey,
                nil];

	if ([inKey isEqualToString:@"inputBlinkenStructure"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                	@"Blinken Structure", QCPortAttributeNameKey,
                nil];

	
	if ([inKey isEqualToString:@"inputFPSCap"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                	@"FPS Cap", QCPortAttributeNameKey,
                	[NSNumber numberWithInt:30], QCPortAttributeDefaultValueKey,
                nil];
	if ([inKey isEqualToString:@"inputBitsPerPixel"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                	@"Bits Per Pixel", QCPortAttributeNameKey,
                	[NSNumber numberWithInt:8], QCPortAttributeDefaultValueKey,
                nil];
	
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

- (NSArray *)blinkenStructureForImageProvider:(id)inImageProvider bitsPerPixel:(int)inBPP {

	NSMutableArray *blinkenStructure = [NSMutableArray array];
	static int valueBase = 0;
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
	valueBase++;

	return blinkenStructure;
}

- (BOOL)blinkenStructureIsDifferentFromLastFrame:(NSArray *)inStructure {
	return YES;
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

	if ([self didValueForInputKeyChange:@"inputFPSCap"]) {
		_minimumFrameTimeDifference = 1.0 / self.inputFPSCap;
	}

	int bitsPerPixel = (int)self.inputBitsPerPixel;

	if (time - _lastFrameTime + 1.0 / 120. >= _minimumFrameTimeDifference) {
		
		NSArray *blinkenStructure = self.inputBlinkenStructure;
		
		id inputImage = self.inputImage;
		if (inputImage) {
			NSArray *structure = [self blinkenStructureForImageProvider:inputImage bitsPerPixel:bitsPerPixel];
			if (structure) blinkenStructure = structure;
		}
		
		if ([self blinkenStructureIsDifferentFromLastFrame:blinkenStructure]) {
			self.blinkenStructure = blinkenStructure;
			[_blinkenSender sendBlinkenStructure:blinkenStructure];
			
			if (!_blinkenImageProvider)
			{
				_blinkenImageProvider = [BlinkenImageProvider new];
			}
			CGSize imageSize = CGSizeMake((CGFloat)[[blinkenStructure lastObject] count],(CGFloat)[blinkenStructure count]);
			
			if ([blinkenStructure count] && [[blinkenStructure lastObject] count]) {
				
				[_blinkenImageProvider setFrameData:[BlinkenSender frameDataForBlinkenStructure:blinkenStructure] size:imageSize channels:1 maxValue:0xff];
	
				self.outputImage = _blinkenImageProvider;
				self.outputBlinkenStructure = blinkenStructure;
				self.outputPixelHeight = imageSize.height;
				self.outputPixelWidth  = imageSize.width;
				_lastFrameTime = time;
			} else {
				self.outputBlinkenStructure = nil;
				self.outputImage = nil;
				self.outputPixelHeight = 0;
				self.outputPixelWidth = 0;
			}
		}
		

	}
	
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
