#import <OpenGL/CGLMacro.h>

#import "BlinkenListener.h"
#import "BlinkenSender.h"
#import "BlinkenBMLWriterPlugIn.h"
#import "BlinkenImageProvider.h"

#define	kQCPlugIn_Name				@"Blinken BML Writer"
#define	kQCPlugIn_Description		@"This plugin will write out the input Blinkenstructure to an BML file as long as it is enabled"



@implementation BlinkenBMLWriterPlugIn

@synthesize blinkenStructure = _blinkenStructure;
@synthesize BMLBaseDirectory = _BMLBaseDirectory;
@synthesize writingFileHandle = _writingFileHandle;

@dynamic inputBaseDirectory;
@dynamic inputBlinkenStructure;
@dynamic inputFPSCap;
@dynamic inputColor;


+ (NSArray*) sortedPropertyPortKeys {
    return [NSArray arrayWithObjects:
    
		@"inputBlinkenStructure",
		@"inputFPSCap",
		@"inputBaseDirectory",
		@"inputColor",

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
	if ([inKey isEqualToString:@"inputBlinkenStructure"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                	@"Blinken Structure", QCPortAttributeNameKey,
                nil];

	
	if ([inKey isEqualToString:@"inputFPSCap"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                	@"FPS Cap", QCPortAttributeNameKey,
                	[NSNumber numberWithInt:15], QCPortAttributeDefaultValueKey,
                nil];

	if ([inKey isEqualToString:@"inputBaseDirectory"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                	@"Destination Path", QCPortAttributeNameKey,
                	@"~/Desktop", QCPortAttributeDefaultValueKey,
                nil];

	if ([inKey isEqualToString:@"inputColor"])
        return [NSDictionary dictionaryWithObjectsAndKeys:
                	@"Color?", QCPortAttributeNameKey,
                	[NSNumber numberWithInt:0], QCPortAttributeDefaultValueKey,
                nil];
	
	return nil;
}

+ (QCPlugInExecutionMode)executionMode
{
	/*
	Return the execution mode of the plug-in: kQCPlugInExecutionModeProvider, kQCPlugInExecutionModeProcessor, or kQCPlugInExecutionModeConsumer.
	*/
	
	return kQCPlugInExecutionModeConsumer;
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

	self.blinkenStructure = nil;
	
	[super dealloc];
}



- (BOOL)startExecution:(id<QCPlugInContext>)inContext
{
//	NSLog(@"%s",__FUNCTION__);
    _renderedOnce = NO;
	return YES;
}

- (void)enableExecution:(id<QCPlugInContext>)inContext
{
//	NSLog(@"%s",__FUNCTION__);
	_renderedOnce = NO;
}

- (BOOL)blinkenStructureIsDifferentFromLastFrame:(NSArray *)inStructure {
	BOOL result = ![inStructure isEqualToArray:self.blinkenStructure];
	return result;
}

#pragma mark -
#pragma mark BML writing


- (void)startBMLFile {
//	NSLog(@"%s, %@",__FUNCTION__, self.blinkenStructure);
	NSMutableString *xmlStart = [NSMutableString stringWithString:@"<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n"];
	NSArray *structure = self.inputBlinkenStructure;
	int channels = self.inputColor ? 3 : 1;
	[xmlStart appendFormat:@"<blm width=\"%d\" height=\"%d\" bits=\"%d\" channels=\"%d\">\n",[[structure lastObject] count] / channels,[structure count],self.inputColor ? 8 : 4,channels];
	[xmlStart appendFormat:@"<header>\n"];
	[xmlStart appendFormat:@"  <title></title>\n"];
	[xmlStart appendFormat:@"  <description></description>\n"];
	[xmlStart appendFormat:@"  <author></author>\n"];
	[xmlStart appendFormat:@"  <email></email>\n"];
	[xmlStart appendFormat:@"  <url></url>\n"];
	[xmlStart appendFormat:@"  <loop>no</loop>\n"];
	[xmlStart appendFormat:@"  <max_duration>60</max_duration>\n"];
	[xmlStart appendFormat:@"  <recorder>Blinken BML Writer Plugin</recorder>\n"];
	NSCalendarDate *date = [NSCalendarDate date];
	[xmlStart appendFormat:@"  <recordingDate>%@</recordingDate>\n",[date descriptionWithCalendarFormat:@"%Y-%m-%d"]];
	[xmlStart appendFormat:@"  <recordingTime>%@</recordingTime>\n",[date descriptionWithCalendarFormat:@"%H:%M:%S"]];
	[xmlStart appendFormat:@"</header>\n"];
	NSLog(@"%s writing %@ to %@",__FUNCTION__,xmlStart,_writingFileHandle);
	[_writingFileHandle writeData:[xmlStart dataUsingEncoding:NSUTF8StringEncoding allowLossyConversion:YES]];
}

// warning: if !_rendered once this only may be called in an execute context
- (void)writeBlinkenFrame {
//	NSLog(@"%s",__FUNCTION__);
	if (!_renderedOnce) {
		NSString *basePath = [self.inputBaseDirectory stringByStandardizingPath];
		if (basePath) {
			NSFileManager *fm = [NSFileManager defaultManager];
			if (![fm fileExistsAtPath:basePath])
			{
				NSLog(@"%s creating directory:%@",__FUNCTION__,basePath);
				[fm createDirectoryAtPath:basePath withIntermediateDirectories:YES attributes:nil error:nil];
			}
			BOOL isDirectory;
			if ([fm fileExistsAtPath:basePath isDirectory:&isDirectory] && isDirectory) {
				NSString *filePath = [basePath stringByAppendingPathComponent:[NSString stringWithFormat:@"%@.bml",[[NSCalendarDate date] descriptionWithCalendarFormat:@"%Y-%m-%d_%H-%M-%S"]]];
				[fm createFileAtPath:filePath contents:nil attributes:nil];
				self.writingFileHandle = [NSFileHandle fileHandleForWritingAtPath:filePath];
				NSLog(@"%s created filehandle:%@",__FUNCTION__,self.writingFileHandle);
				
				[self startBMLFile];
			}
		}
	}
	BOOL isColor = self.inputColor;
	NSString *entryRepresentations[] = {@"0",@"1",@"2",@"3",@"4",@"5",@"6",@"7",@"8",@"9",@"a",@"b",@"c",@"d",@"e",@"f"};
	if (_writingFileHandle && _blinkenStructure)
	{
		NSInteger frameDuration = ceil((_currentFrameTime - _lastFrameTime) * 1000);
		if (frameDuration > 0) {
			NSMutableArray *rows = [NSMutableArray array];
			for (NSArray *dataRow in _blinkenStructure) {
				NSMutableString *rowString = [NSMutableString string];
				for (NSNumber *dataPoint in dataRow) {
					if (isColor) {
						[rowString appendFormat:@"%02x",[dataPoint unsignedCharValue]];
					} else {
						int index = MIN(15,MAX(0,[dataPoint intValue]));
						[rowString appendString:entryRepresentations[index]];
					}
				}
				[rows addObject:[NSString stringWithFormat:@"      <row>%@</row>",rowString]];
			}
			NSString *frameString = [NSString stringWithFormat:@"    <frame duration=\"%d\">\n%@\n    </frame>\n",frameDuration,
					[rows componentsJoinedByString:@"\n"]];
			[_writingFileHandle writeData:[frameString dataUsingEncoding:NSUTF8StringEncoding allowLossyConversion:YES]];
		}
	}
}

- (void)finishBMLFile {
//	NSLog(@"%s",__FUNCTION__);
	if (_writingFileHandle) {
		// writeout last frame
		[self writeBlinkenFrame]; 
		// close up xml
		[_writingFileHandle writeData:[@"</blm>\n" dataUsingEncoding:NSUTF8StringEncoding allowLossyConversion:YES]];
		[_writingFileHandle closeFile];
		self.writingFileHandle = nil;
	}
}


#pragma mark -

- (BOOL)execute:(id<QCPlugInContext>)inContext atTime:(NSTimeInterval)time withArguments:(NSDictionary*)arguments
{
	if (!self.inputBlinkenStructure) return YES;
	_currentFrameTime = time;
	if ([self didValueForInputKeyChange:@"inputFPSCap"]) {
		_minimumFrameTimeDifference = 1.0 / self.inputFPSCap;
	}

	if ([self didValueForInputKeyChange:@"inputBlinkenStructure"] || 
		self.blinkenStructure == nil)
	{
		_blinkenInputChanged = YES;
	}

	if ((_blinkenInputChanged && time - _lastFrameTime + 1.0 / 60. >= _minimumFrameTimeDifference) ||
		 !_renderedOnce) {
		
		NSArray *blinkenStructure = self.blinkenStructure;
		
		if (_blinkenInputChanged) {
			blinkenStructure = self.inputBlinkenStructure;
			
			if ([self blinkenStructureIsDifferentFromLastFrame:blinkenStructure]) {
				[self writeBlinkenFrame];
				 self.blinkenStructure = blinkenStructure;
				_lastFrameTime = time;
			}
			_renderedOnce = YES;
			_blinkenInputChanged = NO;
		}
	}
	
	return YES;
}

- (void)disableExecution:(id<QCPlugInContext>)inContext
{
//	NSLog(@"%s",__FUNCTION__);
	[self finishBMLFile];
	self.blinkenStructure = nil;
}

- (void)stopExecution:(id<QCPlugInContext>)context
{
//	NSLog(@"%s",__FUNCTION__);
	[self finishBMLFile];
	self.blinkenStructure = nil;
}


@end
