#import <Cocoa/Cocoa.h>
#import <Quartz/Quartz.h>

@interface StereoscopeAppController : NSObject {
    IBOutlet NSWindow *_ibWindow;
    IBOutlet QCView *_ibQCView;
}

- (IBAction)changeViewPosition:(id)sender;
- (IBAction)openBlinkenlightsHomepage:(id)inSender;

@end
