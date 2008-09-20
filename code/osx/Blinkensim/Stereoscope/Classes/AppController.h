#include "Application.h"
#import "GraphicsDevice.h"
#import "SettingsController.h"
#import "TCMHost.h"
#import "BlinkenListener.h"
#import "Reachability.h"

@interface AppController : NSObject <EAGLViewDelegate>
{
	IBOutlet UIWindow *_window;
	EAGLView *_glView;
	IBOutlet UILabel *_framerateLabel;
	IBOutlet UILabel *_liveLabel;
	IBOutlet UILabel *_loadingLabel;
	IBOutlet UIButton *_infoButton;
	IBOutlet UIButton *_cameraButton;

    IBOutlet UIButton *_position1Button;
    IBOutlet UIButton *_position2Button;
    IBOutlet UIButton *_position3Button;
    IBOutlet UIButton *_position4Button;
    IBOutlet UIButton *_position5Button;
    IBOutlet UIButton *_position6Button;
    IBOutlet UIButton *_position7Button;
    IBOutlet UIButton *_position8Button;

	NetworkStatus _internetConnectionStatus;
    
    IBOutlet SettingsController *_settingsController;
	IBOutlet UINavigationController *_mainNavigationController;
	NSTimer *_updateTimer;
	TCMHost *_hostToResolve;
	NSURLConnection *_proxyListConnection;
	NSMutableData *_responseData;
	NSMutableArray *_blinkenStreamsArray;
	BlinkenListener    *_blinkenListener;
	NSMutableArray *_frameQueue;
	NSTimeInterval _maxTimeDifference;
	int            _timeSamplesTaken;
	NSTimeInterval _lastDrawTime;
	char displayState[23][54];
	UIImageView *_titleView;
	NSDictionary *_currentProxy;
	NSTimeInterval _connectionLostTime;
	NSTimeInterval _hostResolveFailureTime;
	BOOL _fadeOutOnBlinkenframe;
}

+ (AppController *)sharedAppController;

@property (nonatomic, retain) NSURLConnection *proxyListConnection;
@property (nonatomic, retain) TCMHost *hostToResolve;
@property (nonatomic, retain) UIWindow *window;
@property (nonatomic, retain) UILabel *framerateLabel;
@property (nonatomic, retain) UILabel *loadingLabel;
@property (nonatomic, retain) UIButton *infoButton;
@property (nonatomic, retain) UINavigationController *mainNavigationController;
@property (nonatomic, retain) SettingsController *settingsController;
@property (nonatomic, retain) NSDictionary *currentProxy;
@property NetworkStatus previousInternetConnectionStatus;

- (void)connectToProxy:(NSDictionary *)inProxy;
- (void)shellReportsFrameRate:(float)inCurrentFrameRate;
- (IBAction)showSettings:(id)inSender;
- (IBAction)doneWithSettings:(id)inSender;

- (BOOL)hasConnection;

- (void)setStatusText:(NSString *)inString;
- (void)fadeoutStatusText;

- (IBAction)changeCamera:(id)inSender;
- (IBAction)selectPosition:(id)inSender;

@end



