#import <Cocoa/Cocoa.h>
#import <Quartz/Quartz.h>

@interface StereoscopeAppController : NSObject {
    IBOutlet NSWindow *_ibWindow;
    IBOutlet QCView *_ibQCView;
	NSURLConnection *_proxyListConnection;
	NSMutableData *_responseData;
	NSMutableDictionary *_blinkenStreamsDict;
	IBOutlet NSMenu *_ibStreamsMenu;
	NSMutableDictionary *_messageDictionary;
}

@property (retain) NSDictionary *messageDictionary;
@property (nonatomic, retain) NSURLConnection *proxyListConnection;

- (IBAction)changeViewPosition:(id)sender;
- (IBAction)openBlinkenlightsHomepage:(id)inSender;
- (IBAction)openStereoscopeHomepage:(id)inSender;
- (IBAction)openBlinkenlightsBlog:(id)inSender;

- (IBAction)installBlinkenposer:(id)inSender;
- (IBAction)selectProxy:(id)inSender;

+ (int)installedBlinkenposerPluginVersion;

@end
