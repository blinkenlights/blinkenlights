#import "GraphicsDevice.h"
#import "SettingsController.h"
#import "TCMHost.h"
#import "BlinkenListener.h"

@interface AppController : NSObject
{
	IBOutlet UIWindow *_window;
	EAGLView *_glView;
	IBOutlet UILabel *_framerateLabel;
	IBOutlet UILabel *_loadingLabel;
	IBOutlet UIButton *_infoButton;
	IBOutlet SettingsController *_settingsController;
	IBOutlet UINavigationController *_mainNavigationController;
	NSTimer *_updateTimer;
	TCMHost *_hostToResolve;
	NSURLConnection *_proxyListConnection;
	NSMutableData *_responseData;
	NSMutableDictionary *_blinkenStreamsDict;
	BlinkenListener    *_blinkenListener;
	NSMutableArray *_frameQueue;
	NSTimeInterval _maxTimeDifference;
	int            _timeSamplesTaken;
	NSTimeInterval _lastDrawTime;
	char displayState[23][54];
}

@property (nonatomic, retain) NSURLConnection *proxyListConnection;
@property (nonatomic, retain) TCMHost *hostToResolve;
@property (nonatomic, retain) UIWindow *window;
@property (nonatomic, retain) UILabel *framerateLabel;
@property (nonatomic, retain) UILabel *loadingLabel;
@property (nonatomic, retain) UIButton *infoButton;
@property (nonatomic, retain) UINavigationController *mainNavigationController;
@property (nonatomic, retain) SettingsController *settingsController;

- (void)shellReportsFrameRate:(float)inCurrentFrameRate;
- (IBAction)showSettings:(id)inSender;
- (IBAction)doneWithSettings:(id)inSender;
@end



