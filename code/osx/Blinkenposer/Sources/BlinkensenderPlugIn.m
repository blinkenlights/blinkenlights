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
@dynamic inputTargetPort;
@dynamic inputImage;
@dynamic inputBlinkenStructure;
@dynamic inputFPSCap;
@dynamic inputMinFPS;
//@dynamic inputBitsPerPixel;

@dynamic outputImage;
@dynamic outputPixelWidth;
@dynamic outputPixelHeight;
@dynamic outputBlinkenStructure;


+ (NSArray*) sortedPropertyPortKeys {
    return [NSArray arrayWithObjects:
    
		@"inputTargetAddress",
		@"inputTargetPort",
		@"inputImage",
		@"inputBlinkenStructure",
		@"inputFPSCap",
		@"inputMinFPS",
//		@"inputBitsPerPixel",
    
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
                	@"localhost", QCPortAttributeDefaultValueKey,
                nil];

	if ([inKey isEqualToString:@"inputTargetPort"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                	@"Target Port", QCPortAttributeNameKey,
                	[NSNumber numberWithInt:2323], QCPortAttributeDefaultValueKey,
                	[NSNumber numberWithInt:0],QCPortAttributeMinimumValueKey,
                	[NSNumber numberWithInt:65535],QCPortAttributeMaximumValueKey,
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
                	[NSNumber numberWithInt:15], QCPortAttributeDefaultValueKey,
                nil];

	if ([inKey isEqualToString:@"inputMinFPS"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                	@"Min FPS", QCPortAttributeNameKey,
                	[NSNumber numberWithInt:1], QCPortAttributeDefaultValueKey,
                nil];

//	if ([inKey isEqualToString:@"inputBitsPerPixel"])
//        return [NSDictionary dictionaryWithObjectsAndKeys:
//                	@"Bits Per Pixel", QCPortAttributeNameKey,
//                	[NSNumber numberWithInt:4], QCPortAttributeDefaultValueKey,
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
	
	return kQCPlugInTimeModeTimeBase;
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
    _renderedOnce = NO;
	return YES;
}

- (void)enableExecution:(id<QCPlugInContext>)inContext
{
	/*
	Called by Quartz Composer when the plug-in instance starts being used by Quartz Composer.
	*/
	_renderedOnce = NO;
	_blinkenSender = [BlinkenSender new];
}

- (NSArray *)blinkenStructureForImageProvider:(id)inImageProvider bitsPerPixel:(int)inBPP {

	CGColorSpaceRef					colorSpace;
	NSString*						pixelFormat;
	NSRect bounds = [inImageProvider imageBounds];

	/* Figure out pixel format and colorspace to use */
	colorSpace = [inImageProvider imageColorSpace];
	if (CGColorSpaceGetModel(colorSpace) == kCGColorSpaceModelMonochrome) {
		pixelFormat = QCPlugInPixelFormatI8;
	} else if (CGColorSpaceGetModel(colorSpace) == kCGColorSpaceModelRGB) {
#if __BIG_ENDIAN__
		pixelFormat = QCPlugInPixelFormatARGB8;
#else
		pixelFormat = QCPlugInPixelFormatBGRA8;
#endif
	} else {
		NSLog(@"%s no adequate color space",__FUNCTION__);
		return nil;
	}
	
	/* Get a buffer representation from the image in its native colorspace */
	if(![inImageProvider lockBufferRepresentationWithPixelFormat:pixelFormat colorSpace:colorSpace forBounds:bounds]) {
		NSLog(@"%s lock buffer representation did fail",__FUNCTION__);
		return nil;
	}
	
	NSUInteger bufferBytesPerRow = [inImageProvider bufferBytesPerRow];
//	NSLog(@"%s inImageProvider %@ %@ %d",__FUNCTION__, NSStringFromRect(bounds), [inImageProvider bufferPixelFormat], bufferBytesPerRow);

	unsigned char *bufferBaseAddress = (unsigned char *)[inImageProvider bufferBaseAddress];

	
	NSMutableArray *blinkenStructure = [NSMutableArray array];
	int x = 0, y=0;

	int value = 0;
	for (y = 0; y< NSHeight(bounds); y++) {
		NSMutableArray *blinkenRow = [NSMutableArray array];
		unsigned char *rowBase = bufferBaseAddress + bufferBytesPerRow * y;
		for (x = 0; x < NSWidth(bounds); x++) {
			if (pixelFormat == QCPlugInPixelFormatI8) {
				value = *rowBase;
				rowBase++;
			} else {
#if __BIG_ENDIAN__
			rowBase++;
#endif
				value = ((int)(*rowBase) + (int)(*(rowBase + 1))+ (int)(*(rowBase + 2)) ) / 3;
#if __BIG_ENDIAN__
			rowBase += 3;
#else
			rowBase += 4;
#endif
			}
			value = (int)roundf((value / 255.) * 15.);
			[blinkenRow addObject:[NSNumber numberWithInt:value]];
		}
		[blinkenStructure addObject:blinkenRow];

	}

	[inImageProvider unlockBufferRepresentation];

	return blinkenStructure;
}

- (BOOL)blinkenStructureIsDifferentFromLastFrame:(NSArray *)inStructure {
	BOOL result = ![inStructure isEqualToArray:self.blinkenStructure];
	return result;
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

	if ([self didValueForInputKeyChange:@"inputTargetAddress"] || [self didValueForInputKeyChange:@"inputTargetPort"] || !_renderedOnce)
	{
		[_blinkenSender setTargetAddress:[NSString stringWithFormat:@"%@:%d", self.inputTargetAddress, self.inputTargetPort]];
	}

	if ([self didValueForInputKeyChange:@"inputFPSCap"]) {
		_minimumFrameTimeDifference = 1.0 / self.inputFPSCap;
	}

	if ([self didValueForInputKeyChange:@"inputMinFPS"]) {
		_maximumFrameTimeDifference = 1.0 / self.inputMinFPS;
	}

	if ([self didValueForInputKeyChange:@"inputImage"] || [self didValueForInputKeyChange:@"inputBlinkenStructure"] || self.blinkenStructure == nil)
	{
		_blinkenInputChanged = YES;
	}

	int bitsPerPixel = 4; //MIN(1,MAX(4,(int)self.inputBitsPerPixel));

	if ((_blinkenInputChanged && time - _lastFrameTime + 1.0 / 60. >= _minimumFrameTimeDifference) ||
		 time - _lastFrameTime >= _maximumFrameTimeDifference ||
		 !_renderedOnce) {
		
		
		NSArray *blinkenStructure = self.blinkenStructure;
		
		if (_blinkenInputChanged) {
			blinkenStructure = self.inputBlinkenStructure;
			
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
			_renderedOnce = YES;
			_blinkenInputChanged = NO;
		} else {
			[_blinkenSender sendBlinkenStructure:blinkenStructure];
//			NSLog(@"%s sent an unchanged frame (yay!)",__FUNCTION__);
			_lastFrameTime = time;
		}

	} else {
//		NSLog(@"%s skipped a frame (yay!)",__FUNCTION__);
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
	self.blinkenStructure = nil;
}

- (void) stopExecution:(id<QCPlugInContext>)context
{
//	NSLog(@"%s",__FUNCTION__);
	/*
	Called by Quartz Composer when rendering of the composition stops: perform any required cleanup for the plug-in.
	*/
	self.blinkenStructure = nil;
}


@end
