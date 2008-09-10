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
}

@property (nonatomic, retain) EditingViewController *editingViewController;

- (void)updateWithBlinkenstreams:(NSDictionary *)inBlinkenDict;

@end
