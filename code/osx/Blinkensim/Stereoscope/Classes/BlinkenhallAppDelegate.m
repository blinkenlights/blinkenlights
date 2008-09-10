//
//  BlinkenhallAppDelegate.m
//  Blinkenhall
//
//  Created by Dominik Wagner on 17.05.08.
//  Copyright TheCodingMonkeys 2008. All rights reserved.
//

#import "BlinkenhallAppDelegate.h"
#import "EAGLView.h"
#import "Blinkenlistener.h"
#import "SettingsController.h"


@implementation BlinkenhallAppDelegate

@synthesize window;
@synthesize glView;
@synthesize toolbar;
@synthesize mainNavigationController = _mainNavigationController;
@synthesize settingsController = _settingsController;
@synthesize hostToResolve;
@synthesize proxyListConnection;
@synthesize loadingLabel = _loadingLabel;

static id s_sharedInstance = nil;

#define FRAMERATE 40.

- (void)awakeFromNib {
	[super awakeFromNib];
	s_sharedInstance = nil;
}

- (void)settingsChanged {
	[_blinkenListener stopListening];
	if (hostToResolve) [hostToResolve setDelegate:nil];
	NSString *hostName = [[[[NSUserDefaults standardUserDefaults] stringForKey:@"blinkenproxyAddress"] componentsSeparatedByString:@":"] objectAtIndex:0];
	if (hostName && hostName.length) {
		[UIApplication sharedApplication].networkActivityIndicatorVisible = YES;
		_loadingLabel.text = [NSString stringWithFormat:@"Resolving %@",hostName];
		[UIView beginAnimations:@"ProxyConnectionAnimation" context:NULL];
		[UIView setAnimationDuration:3.0];
		_loadingLabel.alpha = 1.0;
		[UIView commitAnimations];
		self.hostToResolve = [TCMHost hostWithName:hostName port:1234 userInfo:nil];
		[hostToResolve setDelegate:self];
		[hostToResolve resolve];
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
	[_blinkenListener setProxyAddress:address];
	[_blinkenListener listen];
}

- (void)host:(TCMHost *)inHost didNotResolve:(NSError *)inError;
{
	[UIApplication sharedApplication].networkActivityIndicatorVisible = NO;
	_loadingLabel.text = [NSString stringWithFormat:@"Could not resolve %@",[(id)inHost name]];
 	NSLog(@"%s %@ %@",__FUNCTION__,inHost, inError);
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
	[glView blinkenListener:inListener receivedFrames:inFrames atTimestamp:inTimestamp];
}



- (void)applicationDidFinishLaunching:(UIApplication *)inApplication {
	// try to fetch the blinkenstreams.xml file
	NSURLRequest *request = [NSURLRequest requestWithURL:[NSURL URLWithString:@"http://www.blinkenlights.net/config/blinkenstreams.xml"]];
	_responseData = [NSMutableData new];
	self.proxyListConnection = [[[NSURLConnection alloc] initWithRequest:request delegate:self] autorelease];
	inApplication.idleTimerDisabled = YES; // don't sleep when blinkenlights is on - we want to look at it!

	glView.animationInterval = 1.0 / FRAMERATE;
	_blinkenListener = [BlinkenListener new];
	[_blinkenListener setDelegate:self];
	[self settingsChanged];
	NSLog(@"%s %@",__FUNCTION__, self.toolbar.items);
	[self.window addSubview:[self.mainNavigationController view]];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(settingsChanged) name:@"SettingChange" object:nil];
}


- (void)applicationWillResignActive:(UIApplication *)application {
	glView.animationInterval = 1.0 / 1.0;
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
	glView.animationInterval = 1.0 / FRAMERATE;
}

- (IBAction)presentSettings:(id)aSender
{
	[_mainNavigationController pushViewController:_settingsController animated:YES]; 
}

- (void)dealloc {
	self.proxyListConnection = nil;
	[_blinkenListener stopListening];
	[_blinkenListener release];
	[window release];
	[glView release];
	[super dealloc];
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
