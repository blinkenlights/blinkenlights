//
//  Delegate.h
//
//  Created by Wolfgang Engel on 3/21/08.
//  Copyright __MyCompanyName__ 2008. All rights reserved.
//
#import "GraphicsDevice.h"
#import "SettingsController.h"

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
}

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



