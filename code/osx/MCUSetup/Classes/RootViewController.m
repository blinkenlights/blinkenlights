//
//  RootViewController.m
//  MCUSetup
//
//  Created by Dominik Wagner on 29.09.08.
//  Copyright TheCodingMonkeys 2008. All rights reserved.
//

#import "RootViewController.h"
#import "MCUSetupAppDelegate.h"
#import "MCUGammaAdjustSliderCell.h"

#define NUMBER_OF_SECTIONS 2
#define ADDRESS_SECTION 1
#define SLIDER_SECTION 0



@implementation RootViewController

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return NUMBER_OF_SECTIONS;
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section {
	if (section == ADDRESS_SECTION) {
		return @"Target Address";
	} else if (section == SLIDER_SECTION) {
	    return @"Gamma Table";
	} else {
	    return 0;
	}
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
	if (section == ADDRESS_SECTION) {
		return 1;
	} else if (section == SLIDER_SECTION) {
	    return 16;
	} else {
	    return 0;
	}
}


- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    
    NSUInteger section = indexPath.section;
    NSString *CellIdentifier = @"Cell";
	if (section == ADDRESS_SECTION) {
		CellIdentifier = @"AddressCell";
	} else if (section == SLIDER_SECTION) {
		CellIdentifier = @"SliderCell";
	} else {
	    return 0;
	}
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    if (cell == nil) {
		if (section == ADDRESS_SECTION) {
	        cell = [[[UITableViewCell alloc] initWithFrame:CGRectZero reuseIdentifier:CellIdentifier] autorelease];
		} else if (section == SLIDER_SECTION) {
	        cell = [[[MCUGammaAdjustSliderCell alloc] initWithFrame:CGRectZero reuseIdentifier:CellIdentifier] autorelease];
		} else {
	        cell = [[[UITableViewCell alloc] initWithFrame:CGRectZero reuseIdentifier:CellIdentifier] autorelease];
		}
    }

    // Set up the cell

	if (section == ADDRESS_SECTION) {
		cell.text = @"10.0.1.201";
	} else if (section == SLIDER_SECTION) {
		MCUGammaAdjustSliderCell *adjustCell = (MCUGammaAdjustSliderCell *)cell;
		int shadeNumber = 0xf-indexPath.row;
		adjustCell.shadeNumber = shadeNumber;
		adjustCell.shadeValue  = 10000 * shadeNumber / 0xf;
		adjustCell.delegate = self;
	}
    
    return cell;
}

- (void)sliderCellDidChangeValue:(MCUGammaAdjustSliderCell *)inCell
{
	NSLog(@"%s %d->%d",__FUNCTION__,inCell.shadeNumber,inCell.shadeValue);
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    // Navigation logic -- create and push a new view controller
}


/*
- (void)viewDidLoad {
    [super viewDidLoad];
    // Uncomment the following line to add the Edit button to the navigation bar.
    // self.navigationItem.rightBarButtonItem = self.editButtonItem;
}
*/


/*
// Override to support editing the list
- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath {
    
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        // Delete the row from the data source
        [tableView deleteRowsAtIndexPaths:[NSArray arrayWithObject:indexPath] withRowAnimation:YES];
    }   
    if (editingStyle == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view
    }   
}
*/


/*
// Override to support conditional editing of the list
- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the specified item to be editable.
    return YES;
}
*/


/*
// Override to support rearranging the list
- (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath toIndexPath:(NSIndexPath *)toIndexPath {
}
*/


/*
// Override to support conditional rearranging of the list
- (BOOL)tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath {
    // Return NO if you do not want the item to be re-orderable.
    return YES;
}
*/

/*
- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
}
*/
/*
- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
}
*/
/*
- (void)viewWillDisappear:(BOOL)animated {
}
*/
/*
- (void)viewDidDisappear:(BOOL)animated {
}
*/

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}


- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning]; // Releases the view if it doesn't have a superview
    // Release anything that's not essential, such as cached data
}


- (void)dealloc {
    [super dealloc];
}

- (IBAction)sendConfig
{
	NSLog(@"%s",__FUNCTION__);
}



@end

