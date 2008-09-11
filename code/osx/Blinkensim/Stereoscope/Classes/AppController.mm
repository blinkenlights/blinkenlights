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

//CONSTANTS:
#define kFPS			60.0
#define kSpeed			10.0

static CShell *shell = NULL;

@interface AppController ()
- (void)settingsChanged;
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

- (void)update
{
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
	NSLog(@"%s",__FUNCTION__);
	_maxTimeDifference = -999999999999999.0;
	_timeSamplesTaken = 0;
}

- (void)startRendering
{
	if (!_updateTimer) {
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
	NSLog(@"%s",__FUNCTION__);
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
	
	[_window addSubview:_glView];

	// add info button, fps and status label
	[_glView addSubview:_framerateLabel];
	[_glView addSubview:_loadingLabel];
	[_glView addSubview:_infoButton];

	// show the window
	[_window makeKeyAndVisible];
	
	if(!shell->InitApplication())
		printf("InitApplication error\n");
	shell->UpdateWindows((unsigned char *)displayState);
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(settingsChanged) name:@"SettingChange" object:nil];

}

- (void)applicationDidBecomeActive:(UIApplication *)application {
	NSLog(@"%s",__FUNCTION__);
	[_blinkenListener listen];

	[self startRendering];


	NSURLRequest *request = [NSURLRequest requestWithURL:[NSURL URLWithString:@"http://www.blinkenlights.net/config/blinkenstreams.xml"]];
	_responseData = [NSMutableData new];
	self.proxyListConnection = [[[NSURLConnection alloc] initWithRequest:request delegate:self] autorelease];

	[self settingsChanged];
}

- (void)applicationWillResignActive:(UIApplication *)application {
	NSLog(@"%s",__FUNCTION__);	
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


- (void)settingsChanged {
	[_blinkenListener stopListening];
	if (_hostToResolve) [_hostToResolve setDelegate:nil];
	NSString *hostName = [[[[NSUserDefaults standardUserDefaults] stringForKey:@"blinkenproxyAddress"] componentsSeparatedByString:@":"] objectAtIndex:0];
	if (hostName && hostName.length) {
		[UIApplication sharedApplication].networkActivityIndicatorVisible = YES;
		_loadingLabel.text = [NSString stringWithFormat:@"Resolving %@",hostName];
		[UIView beginAnimations:@"ProxyConnectionAnimation" context:NULL];
		[UIView setAnimationDuration:3.0];
		_loadingLabel.alpha = 1.0;
		[UIView commitAnimations];
		self.hostToResolve = [TCMHost hostWithName:hostName port:1234 userInfo:nil];
		[_hostToResolve setDelegate:self];
		[_hostToResolve resolve];
	}
}

- (void)hostDidResolveAddress:(TCMHost *)inHost;
{
	NSString *address = [[NSUserDefaults standardUserDefaults] stringForKey:@"blinkenproxyAddress"];
	_loadingLabel.text = [NSString stringWithFormat:@"Connecting to %@",address];
	[UIView beginAnimations:@"ProxyConnectionAnimation" context:NULL];
	[UIView setAnimationDuration:3.0];
	_loadingLabel.alpha = 1.0;
	[UIView commitAnimations];
	
	// create this one lazyly
	if (!_blinkenListener) {
		_blinkenListener = [BlinkenListener new];
		[_blinkenListener setDelegate:self];
	}
	
	[_blinkenListener setProxyAddress:address];
	[_blinkenListener listen];
}

- (void)host:(TCMHost *)inHost didNotResolve:(NSError *)inError;
{
	[UIApplication sharedApplication].networkActivityIndicatorVisible = NO;
	_loadingLabel.text = [NSString stringWithFormat:@"Could not resolve %@",[(id)inHost name]];
 	NSLog(@"%s %@ %@",__FUNCTION__,inHost, inError);
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
	if (_loadingLabel.alpha > 0) {
		[UIView beginAnimations:@"ProxyConnectionAnimation" context:NULL];
		[UIView setAnimationDuration:3.0];
		_loadingLabel.alpha = 0.0;
		[UIView commitAnimations];
		[UIApplication sharedApplication].networkActivityIndicatorVisible = NO;
	}


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
	NSLog(@"%s",__FUNCTION__);
}

- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)inData
{
	NSLog(@"%s %@",__FUNCTION__,inData);
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
	NSLog(@"%s \n%@",__FUNCTION__, responseString);
	[responseString release];
	[_responseData release];
	_responseData = nil;
	if ([_blinkenStreamsDict count])
	{
		[_settingsController updateWithBlinkenstreams:_blinkenStreamsDict];
		[_blinkenStreamsDict release];
		_blinkenStreamsDict = nil;
	}
}

#pragma mark -
#pragma mark XML Parsing

- (void)parser:(NSXMLParser *)parser didStartElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qualifiedName attributes:(NSDictionary *)attributeDict
{
	static NSMutableArray *proxyArray = nil;
	       if ([elementName isEqualToString:@"blinkenstreams"]) {
		_blinkenStreamsDict = [NSMutableDictionary new];
	} else if ([elementName isEqualToString:@"project"]) {
		if ([attributeDict objectForKey:@"name"]) {
			proxyArray = [NSMutableArray array];
			NSMutableDictionary *projectDict = [[attributeDict mutableCopy] autorelease];
			[projectDict setObject:proxyArray forKey:@"proxies"];
			[_blinkenStreamsDict setObject:projectDict forKey:[attributeDict objectForKey:@"name"]];
		}
	} else if ([elementName isEqualToString:@"proxy"]) {
		[proxyArray addObject:attributeDict];
	}
}

- (BOOL)application:(UIApplication *)application handleOpenURL:(NSURL *)inURL
{
	NSString *addressString = [NSString stringWithFormat:@"%@:%d",[inURL host],[inURL port] ? [[inURL port] intValue] : 4242];
	NSLog(@"%s %@",__FUNCTION__,[inURL absoluteString],addressString);
	if (addressString) {
		[[NSUserDefaults standardUserDefaults] setObject:addressString forKey:@"blinkenproxyAddress"];
		[self settingsChanged];
	}
	return YES;
}

@end
