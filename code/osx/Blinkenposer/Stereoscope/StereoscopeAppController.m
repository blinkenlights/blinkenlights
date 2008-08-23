#import "StereoscopeAppController.h"

@implementation StereoscopeAppController

- (void)applicationDidFinishLaunching:(NSNotification *)inNotification 
{
	[_ibQCView setEventForwardingMask:NSAnyEventMask];
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
