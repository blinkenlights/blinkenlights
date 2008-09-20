//
//  SettingsController.h
//  Blinkenhall
//
//  Created by Dominik Wagner on 10.06.08.
//  Copyright 2008 TheCodingMonkeys. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "EditingViewController.h"

@interface SettingsController : UITableViewController {
    EditingViewController *editingViewController;
	NSMutableArray *projectTableSections;
	IBOutlet UILabel *_ibLinkLabel;
	IBOutlet UIImageView *_ibLogoImageView;
	IBOutlet UIView *_ibTopView;
	IBOutlet UIViewController *_ibAboutViewController;
	IBOutlet UIButton *_ibCodingMonkeysButton;
	int _animationState;
}

@property (nonatomic, retain) EditingViewController *editingViewController;
@property (nonatomic, retain) NSMutableArray *projectTableSections;

- (void)updateWithBlinkenstreams:(NSArray *)inBlinkenArray;

- (IBAction)gotoBlinkenlightsWebsite:(id)inSender;
- (IBAction)gotoCodingMonkeysWebsite:(id)inSender;
- (IBAction)dismissAbout:(id)inSender;
- (IBAction)showAbout:(id)inSender;

@end
