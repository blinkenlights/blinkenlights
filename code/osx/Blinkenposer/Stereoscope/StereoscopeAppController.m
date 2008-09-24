#import "StereoscopeAppController.h"

@implementation StereoscopeAppController

@synthesize proxyListConnection = _proxyListConnection;
@synthesize messageDictionary = _messageDictionary;

- (NSArray *)keysToSaveAndRestore
{
	return [NSArray arrayWithObjects:@"Proxy_Address",@"Proxy_Port",@"Use_Proxy", @"Arrangement",nil];
}

- (void)applicationWillTerminate:(NSNotification *)inNotification
{
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	
	for (NSString *key in [self keysToSaveAndRestore]) {
		id object = [_ibQCView valueForInputKey:key];
		if (object) {
			[defaults setObject:object forKey:key];
		}
	}
}

- (void)applicationDidFinishLaunching:(NSNotification *)inNotification 
{
//	NSString *compositionPath = [[NSBundle mainBundle] pathForResource:@"Stereoscope" ofType:@"qtz"];
//	NSLog(@"%s trying do load: %@",__FUNCTION__,compositionPath);
//	[_ibQCView loadCompositionFromFile:compositionPath];

	[_ibQCView setEventForwardingMask:NSAnyEventMask];
	[_ibQCView startRendering];
	
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	
	for (NSString *key in [self keysToSaveAndRestore]) {
		id object = [defaults objectForKey:key];
		if (object) {
			[_ibQCView setValue:object forInputKey:key];
		}
	}
	NSURLRequest *request = [NSURLRequest requestWithURL:[NSURL URLWithString:@"http://www.blinkenlights.net/config/blinkenstreams.xml"] cachePolicy:NSURLRequestReloadIgnoringLocalCacheData timeoutInterval:30.0];
	_responseData = [NSMutableData new];
	self.proxyListConnection = [[[NSURLConnection alloc] initWithRequest:request delegate:self] autorelease];

}

- (IBAction)selectProxy:(id)inSender 
{
	NSDictionary *proxy = [inSender representedObject];
	if (proxy)
	{
		[_ibQCView setValue:[proxy valueForKey:@"address"] forInputKey:@"Proxy_Address"];
		[_ibQCView setValue:[proxy valueForKey:@"port"] forInputKey:@"Proxy_Port"];
		[_ibQCView setValue:[NSNumber numberWithBool:YES] forInputKey:@"Use_Proxy"];
	}
}


- (IBAction)changeViewPosition:(id)sender {
	[_ibQCView setValue:[sender valueForKey:@"tag"] forInputKey:@"Arrangement"];
}

- (BOOL)validateMenuItem:(NSMenuItem *)inMenuItem {
	if ([inMenuItem action] == @selector(changeViewPosition:)) {
		if ([[_ibQCView valueForInputKey:@"Arrangement"] intValue] == [inMenuItem tag]) {
			[inMenuItem setState:NSOnState];
		} else {
			[inMenuItem setState:NSOffState];
		}
	}
	return YES;
}

- (IBAction)openBlinkenlightsHomepage:(id)inSender
{
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://blinkenlights.net/"]];
}

- (IBAction)openStereoscopeHomepage:(id)inSender
{
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://blinkenlights.net/stereoscope"]];
}

- (IBAction)openBlinkenlightsBlog:(id)inSender
{
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://blinkenlights.net/blog"]];
}

- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)inSender
{
	[_ibWindow makeKeyAndOrderFront:self];
	return NO;
}

- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error
{
	NSLog(@"%s",__FUNCTION__);
}

- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)inData
{
	[_responseData appendData:inData];
}

- (void)updateProxyMenuWithBlinkenstreams:(NSDictionary *)blinkenStreams
{
	while ([_ibStreamsMenu numberOfItems]) [_ibStreamsMenu removeItemAtIndex:0];
	
	NSMenuItem *item = nil;
	
	item = [[[NSMenuItem alloc] initWithTitle:@"localhost:4242" action:@selector(selectProxy:) keyEquivalent:@""] autorelease];
	[item setRepresentedObject:[NSDictionary dictionaryWithObjectsAndKeys:@"localhost",@"address",[NSNumber numberWithInt:4242],@"port",nil]];
	[_ibStreamsMenu addItem:item];

	BOOL firstProxy = ![[NSUserDefaults standardUserDefaults] boolForKey:@"didRun"];
	
	for (NSString *projectName in [[blinkenStreams allKeys] sortedArrayUsingSelector:@selector(caseInsensitiveCompare:)])
	{
		NSDictionary *project = [blinkenStreams objectForKey:projectName];
		[_ibStreamsMenu addItem:[NSMenuItem separatorItem]];
		item = [[[NSMenuItem alloc] initWithTitle:[NSString stringWithFormat:@"%@ (%@)",[project objectForKey:@"name"],[project objectForKey:@"building"]] action:0 keyEquivalent:@""] autorelease];
		[_ibStreamsMenu addItem:item];
		for (NSDictionary *proxy in [project objectForKey:@"proxies"])
		{
			
			item = [[[NSMenuItem alloc] initWithTitle:
				[NSString stringWithFormat:@"%@ %@ - %@:%@ (%@,%@)",
					[proxy objectForKey:@"name"] ? [proxy objectForKey:@"name"] : @"unnamed",
					[proxy objectForKey:@"kind"] ? [proxy objectForKey:@"kind"] : @"",
					[proxy objectForKey:@"address"],[proxy objectForKey:@"port"],
					[proxy objectForKey:@"size"],
					[proxy objectForKey:@"format"]] action:@selector(selectProxy:) keyEquivalent:@""] autorelease];
			[item setRepresentedObject:proxy];
			[_ibStreamsMenu addItem:item];
			if (firstProxy) {
				[self selectProxy:item];
				[[NSUserDefaults standardUserDefaults] setBool:YES forKey:@"didRun"];
				firstProxy = NO;
			}
		}
	}
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
		[self updateProxyMenuWithBlinkenstreams:_blinkenStreamsDict];
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
	} else if ([elementName isEqualToString:@"message"]) {
		NSMutableDictionary *messageDict = [attributeDict mutableCopy];
		
		self.messageDictionary = messageDict;
		[messageDict release];
	}
}

- (void)parser:(NSXMLParser *)parser foundCharacters:(NSString *)string 
{
	if (_messageDictionary && ![[_messageDictionary objectForKey:@"messageWasShown"] boolValue]) {
		NSMutableString *messageText = [_messageDictionary objectForKey:@"_messageText"];
		if (!messageText) {
			messageText = [NSMutableString string];
			[_messageDictionary setObject:messageText forKey:@"_messageText"];
		}
		[messageText appendString:string];
	}
}

- (void)parser:(NSXMLParser *)parser didEndElement:(NSString *)elementName namespaceURI:(NSString *)namespaceURI qualifiedName:(NSString *)qName
{
	if ([elementName isEqualToString:@"message"]) {
		NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
		if ([defaults integerForKey:@"lastShownMessageNumber"] >= [[_messageDictionary objectForKey:@"message-number"] intValue])
		{
			// now show - already seen
			return;
		}
		
		NSString *messageTitle = [_messageDictionary objectForKey:@"title"];
		if (!messageTitle) messageTitle = @"Message";
		NSString *messageText = [[_messageDictionary objectForKey:@"_messageText"] stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
		BOOL hasURL = [_messageDictionary objectForKey:@"url"] != 0;
		NSString *urlTitle = [_messageDictionary objectForKey:@"url-title"];
		if (!urlTitle) urlTitle = @"Goto Site";
		NSAlert *alert = [NSAlert alertWithMessageText:messageTitle defaultButton:@"OK" alternateButton:hasURL ? urlTitle : nil otherButton:nil informativeTextWithFormat:@"%@",messageText];
		[alert beginSheetModalForWindow:_ibWindow modalDelegate:self didEndSelector:@selector(messageAlertDidEnd:returnCode:contextInfo:) contextInfo:NULL];
		[defaults setObject:[_messageDictionary objectForKey:@"message-number"] forKey:@"lastShownMessageNumber"];
	}
}

- (void)messageAlertDidEnd:(NSAlert *)alert returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
	if ([_messageDictionary objectForKey:@"url"] && NSAlertAlternateReturn == returnCode) 
	{
		[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[_messageDictionary objectForKey:@"url"]]];
	}
}

@end
