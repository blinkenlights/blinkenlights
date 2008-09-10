//
//  BlinkenhallAppDelegate.h
//  Blinkenhall
//
//  Created by Dominik Wagner on 17.05.08.
//  Copyright TheCodingMonkeys 2008. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "TCMHost.h"

@class EAGLView, BlinkenListener, SettingsController;

@interface BlinkenhallAppDelegate : NSObject <UIApplicationDelegate> {
	IBOutlet UIWindow  *window;
	IBOutlet EAGLView  *glView;
	BlinkenListener    *_blinkenListener;
	IBOutlet UIToolbar *toolbar;
	IBOutlet SettingsController *_settingsController;
	IBOutlet UINavigationController *_mainNavigationController;
	TCMHost *hostToResolve;
	NSURLConnection *proxyListConnection;
	NSMutableData *_responseData;
	NSMutableDictionary *_blinkenStreamsDict;
	IBOutlet UILabel *_loadingLabel;

}

@property (nonatomic, retain) NSURLConnection *proxyListConnection;
@property (nonatomic, retain) TCMHost *hostToResolve;
@property (nonatomic, retain) UIWindow *window;
@property (nonatomic, retain) UILabel *loadingLabel;
@property (nonatomic, retain) EAGLView *glView;
@property (nonatomic, retain) UIToolbar *toolbar;
@property (nonatomic, retain) UINavigationController *mainNavigationController;
@property (nonatomic, retain) SettingsController *settingsController;

- (IBAction)presentSettings:(id)aSender;


@end

