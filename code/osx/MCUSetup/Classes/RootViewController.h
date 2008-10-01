//
//  RootViewController.h
//  MCUSetup
//
//  Created by Dominik Wagner on 29.09.08.
//  Copyright TheCodingMonkeys 2008. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "GammaTableSettingsController.h"

@interface RootViewController : UITableViewController <UIActionSheetDelegate> {
	CFDataRef _targetAddressData;
    CFSocketRef I_sendSocket;
    NSMutableArray *I_values;
    IBOutlet GammaTableSettingsController *gammaTableSettingsController;
}

@property (nonatomic,retain) GammaTableSettingsController *gammaTableSettingsController;

- (IBAction)resetAlertSheet;
- (void)createAddressData;
- (void)createSendSocket;
- (IBAction)sendConfig;
@end
