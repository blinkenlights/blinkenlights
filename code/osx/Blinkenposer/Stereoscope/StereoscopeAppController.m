#import "StereoscopeAppController.h"

@implementation StereoscopeAppController

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
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://blinkenlights.de/"]];
}

- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)inSender
{
	[_ibWindow makeKeyAndOrderFront:self];
	return NO;
}

@end
