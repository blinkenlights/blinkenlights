//
//  RootViewController.h
//  MCUSetup
//
//  Created by Dominik Wagner on 29.09.08.
//  Copyright TheCodingMonkeys 2008. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface RootViewController : UITableViewController {
	CFDataRef _targetAddressData;
    CFSocketRef I_sendSocket;
    NSMutableArray *I_values;
}

- (void)createAddressData;
- (void)createSendSocket;
- (IBAction)sendConfig;
@end
