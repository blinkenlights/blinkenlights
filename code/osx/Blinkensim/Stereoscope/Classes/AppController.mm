//
//  Delegate.m
//  Skeleton
//
//  Created by Wolfgang Engel on 3/21/08.
//  Copyright __MyCompanyName__ 2008. All rights reserved.
//

#import "App.h"
#import "AppController.h"
#include "Camera.h"
#import "TableSection.h"

//CONSTANTS:
#define kFPS			24.0
#define kSpeed			10.0

// five seconds without a frame means timeout
#define CONNECTIION_NO_FRAME_TIMEOUT 5.0

static CShell *shell = NULL;

@interface AppController ()
- (void)fadeoutStartscreen;
- (void)connectToAutoconnectProxy;
- (void)handleConnectionFailure;
@end

@implementation AppController

@synthesize hostToResolve = _hostToResolve;
@synthesize proxyListConnection = _proxyListConnection;
@synthesize framerateLabel = _framerateLabel;
@synthesize loadingLabel   = _loadingLabel;
@synthesize infoButton     = _infoButton;
@synthesize window         = _window;
@synthesize mainNavigationController = _mainNavigationController;
@synthesize settingsController = _settingsController;
@synthesize currentProxy = _currentProxy;

static AppController *s_sharedAppController;

+ (AppController *)sharedAppController 
{
	return s_sharedAppController;
}

- (void)awakeFromNib
{
	s_sharedAppController = self;
}

+ (void)initialize
{
	[[NSUserDefaults standardUserDefaults] registerDefaults:[NSDictionary dictionaryWithObjectsAndKeys:
		[NSNumber numberWithBool:YES],@"autoselectProxy",
		nil]
	];
	srandomdev(); // have a good seed.
	// just to be sure a quick test
	NSLog(@"%s random numbers: 0x%x 0x%x 0x%x 0x%x",__FUNCTION__,random(),random(),random(),random());
}

- (void)update
{
	// check connection
	if ([NSDate timeIntervalSinceReferenceDate] > _connectionLostTime) {
		// connection is lost
		_connectionLostTime = DBL_MAX;
		[self setStatusText:@"No Frames"];
		[_blinkenListener stopListening];
		[self handleConnectionFailure];
	}

	// renderer entry
	if(!shell->InitView())
		printf("InitView error\n");
		
	if(!shell->UpdateScene())
		printf("UpdateScene error\n");
	
    if(!shell->RenderScene())
		printf("RenderScene error\n");
	
	[_glView swapBuffers];
	
	if(!shell->ReleaseView())
		printf("ReleaseView error\n");	
}

- (void)resetTimeCompensation
{
	_maxTimeDifference = -999999999999999.0;
	_timeSamplesTaken = 0;
}

- (void)startRendering
{
	if (!_updateTimer) {
		[self update]; // for quicker display of the first frame
		_updateTimer = [NSTimer scheduledTimerWithTimeInterval:(1.0 / kFPS) target:self selector:@selector(update) userInfo:nil repeats:YES];
	}
}

- (void)stopRendering
{
	[_updateTimer invalidate];
	_updateTimer=nil;
}

- (void)applicationDidFinishLaunching:(UIApplication*)inApplication
{
	_connectionLostTime = DBL_MAX;

	_frameQueue = [NSMutableArray new];

	int maxcount = 23*54;
	int value = 0;
	for (int i=0;i<maxcount;i++)
	{
		*(((char *)displayState)+i)=value;
		value = (i / 54) % 16;
	}
	
	[self resetTimeCompensation];

	CGRect	rect = [[UIScreen mainScreen] bounds];
	inApplication.idleTimerDisabled = YES; // don't sleep when blinkenlights is on - we want to look at it!
		
	// create the OpenGL view and add it to the window
	//_glView = [[EAGLView alloc] initWithFrame:rect];
	_glView = [[EAGLView alloc] initWithFrame:rect pixelFormat:GL_RGB565_OES depthFormat:GL_DEPTH_COMPONENT16_OES preserveBackbuffer:NO];
	[_glView setDelegate:self];
    [_glView setShell:shell];
    
	[_window addSubview:_glView];

	_titleView = [[UIImageView alloc] initWithFrame:rect];
	_titleView.image = [UIImage imageNamed:@"Default.png"];
	[_window addSubview:_titleView];
	
	
	// add info button, fps and status label
	[_glView addSubview:_framerateLabel];
	[_glView addSubview:_loadingLabel];
	[_glView addSubview:_infoButton];
	[_glView addSubview:_cameraButton];
	[_glView addSubview:_liveLabel];

	// show the window
	[_window makeKeyAndVisible];
	
	if(!shell->InitApplication())
		printf("InitApplication error\n");
	shell->UpdateWindows((unsigned char *)displayState);
}

- (void)fadeoutStartscreen 
{
	[UIView beginAnimations:@"StartupAnimation" context:NULL];
	[UIView setAnimationDidStopSelector:@selector(startupAnimationDidEnd:context:)];
	[UIView setAnimationDelegate:self];
	[UIView setAnimationDuration:3.0];
	_titleView.alpha = 0;
	[UIView commitAnimations];
}

- (void)startupAnimationDidEnd:(id)animationId context:(void *)inContext
{
	[_titleView removeFromSuperview];
	[_titleView release];
	_titleView = nil;
}



- (void)applicationDidBecomeActive:(UIApplication *)application {
	[_blinkenListener listen];
	if (!_blinkenListener) {
		if ([[[NSUserDefaults standardUserDefaults] objectForKey:@"autoselectProxy"] boolValue])
		{
			[self connectToAutoconnectProxy];
		}
	}

	[self startRendering];


	NSURLRequest *request = [NSURLRequest requestWithURL:[NSURL URLWithString:@"http://www.blinkenlights.net/config/blinkenstreams.xml"] cachePolicy:NSURLRequestReloadIgnoringLocalCacheData timeoutInterval:30.0];
	_responseData = [NSMutableData new];
	self.proxyListConnection = [[[NSURLConnection alloc] initWithRequest:request delegate:self] autorelease];

	if (_titleView) [self fadeoutStartscreen];
}


- (void)applicationWillResignActive:(UIApplication *)application {
//	NSLog(@"%s",__FUNCTION__);	
	[_blinkenListener stopListening];
	[self stopRendering];
}



#define InFramerateWeight 0.3
- (void)shellReportsFrameRate:(float)inCurrentFrameRate
{
	static float framerate = 30.0;
	framerate = inCurrentFrameRate * InFramerateWeight + framerate * (1.0 - InFramerateWeight);
	static int filter = 0;
	if (filter == 0) {
		_framerateLabel.text = [NSString stringWithFormat:@"%3.2f fps",framerate];

//		NSLog(@"%s %3.2f",__FUNCTION__,framerate);
	}
	filter = (filter + 1) % 6;
}

- (IBAction)showSettings:(id)inSender
{
//	[self.mainNavigationController presentModalViewController:self.mainNavigationController animated:YES];

	UIView *view = _mainNavigationController.view;
	CGRect	rect = [[UIScreen mainScreen] applicationFrame];
	CGPoint targetCenter = CGPointMake(CGRectGetMidX(rect),CGRectGetMidY(rect));
	view.center = CGPointMake(CGRectGetMidX(rect),CGRectGetMidY(rect) + view.bounds.size.height);
	[UIView beginAnimations:@"SettingsAnimation" context:NULL];
	[self.window addSubview:view];
	view.center = targetCenter;
	[UIView commitAnimations];
	[self stopRendering];
}

- (IBAction)doneWithSettings:(id)inSender
{
	UIView *view = _mainNavigationController.view;
	CGRect	rect = [[UIScreen mainScreen] applicationFrame];
	CGPoint targetCenter = CGPointMake(CGRectGetMidX(rect),CGRectGetMidY(rect) + view.bounds.size.height);
	[UIView beginAnimations:@"SettingsAnimation" context:NULL];
	[self.window addSubview:view];
	view.center = targetCenter;
	[UIView setAnimationDidStopSelector:@selector(settingsDidMoveOut:context:)];
	[UIView setAnimationDelegate:self];
	[UIView commitAnimations];
	[self startRendering];
}

- (void)settingsDidMoveOut:(id)animationId context:(void *)inContext
{
	[_mainNavigationController.view removeFromSuperview];
}

- (void)dealloc
{
	[self stopRendering];
	
	if(!shell->QuitApplication())
		printf("QuitApplication error\n");

	self.proxyListConnection = nil;
	[_blinkenListener stopListening];
	[_blinkenListener release];
	
	[_glView release];
	[_window release];
	
	[super dealloc];
}

- (void)connectToProxyFromArray:(NSArray *)inArray {
	NSUInteger count = [inArray count];
	if (count) {
		[self connectToProxy:[inArray objectAtIndex:random() % count]];
	}
}

- (void)connectToAutoconnectProxy {
	// collect all live proxies
	NSMutableArray *liveProxiesToChooseFrom = [NSMutableArray array];
	NSMutableArray *proxiesToChooseFrom = [NSMutableArray array];
	for (TableSection *section in [_settingsController projectTableSections]) {
		for (NSDictionary *proxy in [section items]) {
			if ([proxy valueForKey:@"kind"] &&
				[[proxy valueForKey:@"kind"] caseInsensitiveCompare:@"live"] == NSOrderedSame) {
				[liveProxiesToChooseFrom addObject:proxy];
			}
			[proxiesToChooseFrom addObject:proxy];
		}
	}
	if ([liveProxiesToChooseFrom count]) {
		NSLog(@"%s choosing from live proxies %@",__FUNCTION__,liveProxiesToChooseFrom);
		[self connectToProxyFromArray:liveProxiesToChooseFrom];
	} else if ([proxiesToChooseFrom count]) {
		NSLog(@"%s choosing from all %@",__FUNCTION__,proxiesToChooseFrom);
		[self connectToProxyFromArray:proxiesToChooseFrom];
	}
}

- (void)handleConnectionFailure {
	NSLog(@"%s",__FUNCTION__);
	[self connectToAutoconnectProxy];
}


- (void)connectToProxy:(NSDictionary *)inProxy {
	NSLog(@"%s %@",__FUNCTION__,inProxy);
	NSString *address = [inProxy valueForKey:@"address"];
	if (address) {
		[_blinkenListener stopListening];
		[_hostToResolve cancel];
		[_hostToResolve setDelegate:nil];
		self.hostToResolve = nil;
		self.currentProxy = inProxy;
		[self setStatusText:[NSString stringWithFormat:@"Resolving %@",address]];
		
		// this is just done for asychronous resolving so we know we have an ip address for this one
		self.hostToResolve = [TCMHost hostWithName:address port:1234 userInfo:nil];
		
		[_hostToResolve setDelegate:self];
		[_hostToResolve resolve];
		
		NSString *kindString = [inProxy objectForKey:@"kind"];
		if (!kindString) kindString = @"";
		_liveLabel.text = kindString;
	}
}



- (void)setStatusText:(NSString *)inString {
	_loadingLabel.text = inString;
	if (_loadingLabel.alpha > 0.0) {
		[UIView beginAnimations:@"LoadingLabelFadeAnimation" context:NULL];
		[UIView setAnimationDuration:3.0];
		_loadingLabel.alpha = 1.0;
		[UIView commitAnimations];
	}
}

- (void)fadeoutStatusText {
	if (_loadingLabel.alpha > 0.0) {
		[UIView beginAnimations:@"LoadingLabelFadeAnimation" context:NULL];
		[UIView setAnimationDuration:3.0];
		_loadingLabel.alpha = 0.0;
		[UIView commitAnimations];
		[UIApplication sharedApplication].networkActivityIndicatorVisible = NO;
	}
}

- (void)hostDidResolveAddress:(TCMHost *)inHost;
{
	NSString *addressString = [_currentProxy valueForKey:@"address"];
	id portValue = [_currentProxy valueForKey:@"port"];
	if (portValue) addressString = [addressString stringByAppendingFormat:@":%@",portValue];
	[self setStatusText:[NSString stringWithFormat:@"Connecting to %@",addressString]];
	_connectionLostTime = [NSDate timeIntervalSinceReferenceDate] + CONNECTIION_NO_FRAME_TIMEOUT;

	// create this one lazyly
	if (!_blinkenListener) {
		_blinkenListener = [BlinkenListener new];
		[_blinkenListener setDelegate:self];
	}
	
	[_blinkenListener setProxyAddress:addressString];
	[_blinkenListener listen];
}

- (void)host:(TCMHost *)inHost didNotResolve:(NSError *)inError;
{
	[UIApplication sharedApplication].networkActivityIndicatorVisible = NO;
	[self setStatusText:[NSString stringWithFormat:@"Could not resolve %@",[(id)inHost name]]];
 	NSLog(@"%s %@ %@",__FUNCTION__,inHost, inError);
 	[self handleConnectionFailure];
}

- (void)consumeFrame
{
	NSArray *frames = [_frameQueue lastObject];
	for (BlinkenFrame *frame in frames)
	{
		CGSize frameSize = frame.frameSize;
		NSData *frameData = frame.frameData;
		BOOL isNibbles = frame.bitsPerPixel != 8;
		unsigned char *bytes = (unsigned char *)frameData.bytes;
		unsigned char *bytesEnd = bytes + frameData.length;
		NSUInteger bytesPerRow = (NSUInteger)frameSize.width;
		unsigned char maxValue = frame.maxValue;
		if (isNibbles) bytesPerRow = (bytesPerRow + 1) / 2;

		unsigned char screenID = frame.screenID;

		if (screenID == 0)
		{
			if ((int)frameSize.width == 96 && (int)frameSize.height == 32)
			{
				int rowIndex = 0;
				for (;bytes < bytesEnd;bytes+=bytesPerRow,rowIndex++)
				{
					if (rowIndex >= 5)
					{
						if (isNibbles)
						{
							int offset = 8;
							for (offset = 8; offset < 8+11; offset++)
							{
								displayState[rowIndex - 5][(offset-8)*2]   = (((int)*(bytes + offset)) >>  4) * 15 / maxValue;
								displayState[rowIndex - 5][(offset-8)*2+1] = (((int)*(bytes + offset)) & 0xf) * 15 / maxValue;
							}
							for (offset = 25; offset < 25+15; offset++)
							{
								displayState[rowIndex - 5][24+(offset-25)*2]   = (((int)*(bytes + offset)) >>  4) * 15 / maxValue;
								displayState[rowIndex - 5][24+(offset-25)*2+1] = (((int)*(bytes + offset)) & 0xf) * 15 / maxValue;
							}
						}
						else
						{
							int offset = 16;
							for (offset = 16; offset < 16+22; offset++)
							{
								displayState[rowIndex - 5][offset - 16] = ((int)*(bytes + offset)) * 15 / maxValue;
							}
							for (offset = 50; offset < 50+30; offset++)
							{
								displayState[rowIndex - 5][offset - 50 + 24] = ((int)*(bytes + offset)) * 15 / maxValue;
							}
						}
					}
					if (rowIndex >= 31 - 4) break;
				}
			}
		} 
		else if (screenID == 5)
		{
			// this is the left tower probably
			if ((int)frameSize.width == 22 && (int)frameSize.height == 17)
			{
				int rowIndex = 0;
				for (;bytes < bytesEnd;bytes+=bytesPerRow,rowIndex++)
				{
					if (isNibbles)
					{
						int x = 0;
						for (x = 0; x < 11; x++)
						{
							displayState[rowIndex + 6][x*2]   = (((int)*(bytes + x)) >>  4) * 15 / maxValue;
							displayState[rowIndex + 6][x*2+1] = (((int)*(bytes + x)) & 0xf) * 15 / maxValue;
						}
					}
					else
					{
						int x = 0;
						for (x = 0; x < 22; x++)
						{
							displayState[rowIndex + 6][x] = ((int)*(bytes + x)) * 15 / maxValue;
						}
					}
				}
			}
		}
		else if (screenID == 6)
		{
			// this is the right tower probably
			if ((int)frameSize.width == 30 && (int)frameSize.height == 23)
			{
				int rowIndex = 0;
				for (;bytes < bytesEnd;bytes+=bytesPerRow,rowIndex++)
				{
					if (isNibbles)
					{
						int x = 0;
						for (x = 0; x < 15; x++)
						{
							displayState[rowIndex][24+x*2]   = (((int)*(bytes + x)) >>  4) * 15 / maxValue;
							displayState[rowIndex][24+x*2+1] = (((int)*(bytes + x)) & 0xf) * 15 / maxValue;
						}
					}
					else
					{
						int x = 0;
						for (x = 0; x < 30; x++)
						{
							displayState[rowIndex][24+x] = ((int)*(bytes + x)) * 15 / maxValue;
						}
					}
				}
			}
		}
	}
	[_frameQueue removeLastObject];
	shell->UpdateWindows((unsigned char *)displayState);
}


- (void)blinkenListener:(BlinkenListener *)inListener receivedFrames:(NSArray *)inFrames atTimestamp:(uint64_t)inTimestamp
{
	[self fadeoutStatusText];
	_connectionLostTime = [NSDate timeIntervalSinceReferenceDate] + CONNECTIION_NO_FRAME_TIMEOUT;
	// handle the frame

	[_frameQueue insertObject:inFrames atIndex:0];

	if (inTimestamp != 0) 
	{
		NSTimeInterval now = [[NSDate date] timeIntervalSince1970] ;
		NSTimeInterval timeDifference = now - ((NSTimeInterval)inTimestamp / 1000.0);
		NSTimeInterval compensationTime = (_maxTimeDifference - timeDifference);
		if (_timeSamplesTaken > 5 && ABS(timeDifference - _maxTimeDifference) > 3.0) 
		{
			[self resetTimeCompensation];
		}
		_maxTimeDifference = MAX(timeDifference,_maxTimeDifference);
		if (_timeSamplesTaken <= 5 && compensationTime > 0)
		{
			_timeSamplesTaken++;
			[self consumeFrame];
		}
		else
		{
			// compensate
			[self performSelector:@selector(consumeFrame) withObject:nil afterDelay:compensationTime];
			_maxTimeDifference -= 0.01; // shrink the time difference again to catch up if we only had one hickup
		}
		// NSLog(@"%s ts:0x%016qx %@ now: %@ 0x%016qx",__FUNCTION__,inTimestamp,[NSDate dateWithTimeIntervalSince1970:inTimestamp / (double)1000.0],now, (uint64_t)([now timeIntervalSince1970] * 1000));
		//NSLog(@"time difference: %0.5f s - compensation %0.5f s",timeDifference,compensationTime);
	} else {
		[self consumeFrame];
	}
}

- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error
{
//	NSLog(@"%s",__FUNCTION__);
}

- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)inData
{
//	NSLog(@"%s %@",__FUNCTION__,inData);
	[_responseData appendData:inData];
}

- (void)connectionDidFinishLoading:(NSURLConnection *)connection
{
	NSString *responseString = [[NSString alloc] initWithData:_responseData encoding:NSUTF8StringEncoding];

	// parse XML
	NSXMLParser *parser = [[NSXMLParser alloc] initWithData:_responseData];
	[parser setDelegate:self];
	[parser parse];
	[parser release];
	NSLog(@"%s new xml proxy list:\n%@",__FUNCTION__, responseString);
	[responseString release];
	[_responseData release];
	_responseData = nil;
	if ([_blinkenStreamsArray count])
	{
		[_settingsController updateWithBlinkenstreams:_blinkenStreamsArray];
		[_blinkenStreamsArray release];
		_blinkenStreamsArray = nil;
	}
}

- (IBAction)changeCamera:(id)inSender
{
    
}


#pragma mark -
#pragma mark XML Parsing

- (void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qualifiedName attributes:(NSDictionary *)attributeDict
{
	static NSMutableArray *proxyArray = nil;
	       if ([elementName isEqualToString:@"blinkenstreams"]) {
		_blinkenStreamsArray = [NSMutableArray new];
	} else if ([elementName isEqualToString:@"project"]) {
		if ([attributeDict objectForKey:@"name"]) {
			proxyArray = [NSMutableArray array];
			NSMutableDictionary *projectDict = [attributeDict mutableCopy];
			[projectDict setObject:proxyArray forKey:@"proxies"];
			[_blinkenStreamsArray addObject:projectDict];
			[projectDict release];
		}
	} else if ([elementName isEqualToString:@"proxy"]) {
		[proxyArray addObject:attributeDict];
	}
}

- (BOOL)application:(UIApplication *)application handleOpenURL:(NSURL *)inURL
{
	NSString *addressString = [NSString stringWithFormat:@"%@:%d",[inURL host],[inURL port] ? [[inURL port] intValue] : 4242];
	if (inURL) {
		[[NSUserDefaults standardUserDefaults] setObject:addressString forKey:@"blinkenproxyAddress"];
		[self connectToProxy:[NSDictionary dictionaryWithObjectsAndKeys:
				[inURL host],@"address",
				[inURL port], @"port", // warning if no port port dictionary stops here
				nil]];
	}
	return YES;
}

#pragma mark -
#pragma mark Handle touchstuff

- (BOOL)EAGLView:(EAGLView *)inView shouldNotHandleTouch:(UITouch *)inTouch
{
	CGPoint location = [inTouch locationInView:inView];
	if (location.y > 440 && location.x > 260)
	{
		_infoButton.highlighted = YES;
		return YES;
	}
	return NO;
}

- (void)EAGLView:(EAGLView *)inView  movedUnhandledTouch:(UITouch *)inTouch {
	CGPoint location = [inTouch locationInView:inView];
	if (location.y > 440 && location.x > 260) {
		_infoButton.highlighted = YES;
	} else {
		_infoButton.highlighted = NO;
	}
}


- (void)EAGLView:(EAGLView *)inView didEndUnhandledTouch:(UITouch *)inTouch
{
	if (_infoButton.highlighted) {
		 _infoButton.highlighted = NO;
		[_infoButton sendActionsForControlEvents:UIControlEventTouchUpInside];
	}
}


@end
