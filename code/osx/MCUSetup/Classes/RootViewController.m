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

#import <CoreFoundation/CoreFoundation.h>

#import <netinet/in.h>
#import <netinet6/in6.h>
#import <net/if.h>
#import <sys/socket.h>
#import <arpa/inet.h>
#import <unistd.h>
#import <sys/socket.h>

#define NUMBER_OF_SECTIONS 1
#define SETTINGS_SECTION 1
#define ADDRESS_SECTION 2
#define SLIDER_SECTION 0

@interface NSString (NSStringNATPortMapperAdditions)
+ (NSString *)stringWithAddressData:(NSData *)aData;
@end

@implementation NSString (NSStringNATPortMapperAdditions)
+ (NSString *)stringWithAddressData:(NSData *)aData
{
    if (!aData) return nil;
    struct sockaddr *socketAddress = (struct sockaddr *)[aData bytes];
    
    // IPv6 Addresses are "FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF" at max, which is 40 bytes (0-terminated)
    // IPv4 Addresses are "255.255.255.255" at max which is smaller
    
    char stringBuffer[MAX(INET6_ADDRSTRLEN,INET_ADDRSTRLEN)];
    NSString *addressAsString = nil;
    if (socketAddress->sa_family == AF_INET) {
        if (inet_ntop(AF_INET, &(((struct sockaddr_in *)socketAddress)->sin_addr), stringBuffer, INET_ADDRSTRLEN)) {
            addressAsString = [NSString stringWithUTF8String:stringBuffer];
        } else {
            addressAsString = @"IPv4 un-ntopable";
        }
        int port = ntohs(((struct sockaddr_in *)socketAddress)->sin_port);
            addressAsString = [addressAsString stringByAppendingFormat:@":%d", port];
    } else if (socketAddress->sa_family == AF_INET6) {
         if (inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)socketAddress)->sin6_addr), stringBuffer, INET6_ADDRSTRLEN)) {
            addressAsString = [NSString stringWithUTF8String:stringBuffer];
        } else {
            addressAsString = @"IPv6 un-ntopable";
        }
        int port = ntohs(((struct sockaddr_in6 *)socketAddress)->sin6_port);
        
        // Suggested IPv6 format (see http://www.faqs.org/rfcs/rfc2732.html)
        char interfaceName[IF_NAMESIZE];
        if ([addressAsString hasPrefix:@"fe80"] && if_indextoname(((struct sockaddr_in6 *)socketAddress)->sin6_scope_id,interfaceName)) {
            NSString *zoneID = [NSString stringWithUTF8String:interfaceName];
            addressAsString = [NSString stringWithFormat:@"[%@%%%@]:%d", addressAsString, zoneID, port];
        } else {
            addressAsString = [NSString stringWithFormat:@"[%@]:%d", addressAsString, port];
        }
    } else {
        addressAsString = @"neither IPv6 nor IPv4";
    }
    
    return [[addressAsString copy] autorelease];
}
@end


@implementation RootViewController

@synthesize gammaTableSettingsController;

- (IBAction)resetToFalk
{
	I_values = [[NSMutableArray alloc] initWithObjects:
			[NSNumber numberWithInt:0],
			[NSNumber numberWithInt:2684],
			[NSNumber numberWithInt:2947],
			[NSNumber numberWithInt:3203],
			[NSNumber numberWithInt:3356],
			[NSNumber numberWithInt:3612],
			[NSNumber numberWithInt:3766],
			[NSNumber numberWithInt:3919],
			[NSNumber numberWithInt:4124],
			[NSNumber numberWithInt:4175],
			[NSNumber numberWithInt:4482],
			[NSNumber numberWithInt:4943],
			[NSNumber numberWithInt:5352],
			[NSNumber numberWithInt:5711],
			[NSNumber numberWithInt:6632],
			[NSNumber numberWithInt:10000],
		nil];
	[self.tableView reloadData];
}

- (IBAction)resetToDom
{
	I_values = [[NSMutableArray alloc] initWithObjects:
			[NSNumber numberWithInt:0],
			[NSNumber numberWithInt:2600],
			[NSNumber numberWithInt:3130],
			[NSNumber numberWithInt:3348],
			[NSNumber numberWithInt:3565],
			[NSNumber numberWithInt:3934],
			[NSNumber numberWithInt:4446],
			[NSNumber numberWithInt:4729],
			[NSNumber numberWithInt:4878],
			[NSNumber numberWithInt:5045],
			[NSNumber numberWithInt:5301],
			[NSNumber numberWithInt:5506],
			[NSNumber numberWithInt:5876],
			[NSNumber numberWithInt:6478],
			[NSNumber numberWithInt:7109],
			[NSNumber numberWithInt:10000],
		nil];
	[self.tableView reloadData];
}

- (IBAction)resetAlertSheet {
	[[[[UIActionSheet alloc] initWithTitle:nil delegate:self cancelButtonTitle:NSLocalizedString(@"Cancel",@"Cancel Button") destructiveButtonTitle:nil otherButtonTitles:@"Reset to dom's gamma",@"Reset to falk's gamma", nil] autorelease] showInView:self.view];
}

- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex {
	if (buttonIndex == 0) {
		[self resetToDom];
	} else if (buttonIndex == 1){
		[self resetToFalk];
	}
}

- (void)_inithelper {
	[self createAddressData];
	[self createSendSocket];
	I_values = [[[NSUserDefaults standardUserDefaults] objectForKey:@"GammaTable"] mutableCopy];
	if (!I_values) {
		[self resetToFalk];
	}
}

- (id)initWithStyle:(UITableViewStyle)style {
//	NSLog(@"%s",__FUNCTION__);
	if ((self = [super initWithStyle:style])) 
	{
		[self _inithelper];
	}
	return self;
}

- (id)initWithCoder:(NSCoder *)inCoder
{
	[self _inithelper];
//	NSLog(@"%s",__FUNCTION__);
	return [super initWithCoder:inCoder];
}
- (void)createAddressData {
	CFDataRef addressData = NULL;
	struct sockaddr_in socketAddress;

	bzero(&socketAddress, sizeof(struct sockaddr_in));
	socketAddress.sin_len = sizeof(struct sockaddr_in);
	socketAddress.sin_family = PF_INET;
	socketAddress.sin_port = htons(6942);
	socketAddress.sin_addr.s_addr = inet_addr([@"10.0.1.201" UTF8String]);
	
	addressData = CFDataCreate(kCFAllocatorDefault, (UInt8 *)&socketAddress, sizeof(struct sockaddr_in));
	
	_targetAddressData = addressData;
}

- (void)createSendSocket {
	I_sendSocket = CFSocketCreate(kCFAllocatorDefault, PF_INET, SOCK_DGRAM, IPPROTO_UDP, 
							   0, NULL, NULL);
	int yes = YES;
	if (I_sendSocket) {
//		NSLog(@"%s having my socket",__FUNCTION__);
		int result = setsockopt(CFSocketGetNative(I_sendSocket), SOL_SOCKET, 
								SO_REUSEADDR, &yes, sizeof(int));
		if (result == -1) {
			NSLog(@"Could not setsockopt to reuseaddr: %@ / %s", errno, strerror(errno));
		}
		
		result = setsockopt(CFSocketGetNative(I_sendSocket), SOL_SOCKET, 
								SO_REUSEPORT, &yes, sizeof(int));
		if (result == -1) {
			NSLog(@"Could not setsockopt to reuseport: %@ / %s", errno, strerror(errno));
		}
		
		CFRunLoopRef currentRunLoop = [[NSRunLoop currentRunLoop] getCFRunLoop];
		CFRunLoopSourceRef runLoopSource = CFSocketCreateRunLoopSource(kCFAllocatorDefault, I_sendSocket, 0);
		CFRunLoopAddSource(currentRunLoop, runLoopSource, kCFRunLoopCommonModes);
		CFRelease(runLoopSource);
	}
}

- (NSData *)configDataHigh:(BOOL)isHigh {
	NSMutableData *resultData = [NSMutableData dataWithLength:12*sizeof(uint32_t)];
	uint32_t *ints = (uint32_t *)[resultData bytes];
	*ints++ = OSSwapHostToBigInt32(0x23542667);
	*ints++ = OSSwapHostToBigInt32(2);
	*ints++ = OSSwapHostToBigInt32(0xFFFF);
	*ints++ = OSSwapHostToBigInt32(isHigh ? 0x1:0x0);
	if (!isHigh)
	{
		for (int i=0;i<8;i++) {
			*ints++ = OSSwapHostToBigInt32((uint32_t)[[I_values objectAtIndex:i] unsignedIntValue]);
		}
	} else {
		for (int i=8;i<16;i++) {
			*ints++ = OSSwapHostToBigInt32((uint32_t)[[I_values objectAtIndex:i] unsignedIntValue]);
		}
	}
//	NSLog(@"%s %@",__FUNCTION__,[resultData debugDescription]);
	return resultData;
}

- (void)sendConfig
{
	NSLog(@"%s",__FUNCTION__);
	if (!_targetAddressData) {
		[self createAddressData];
	}
	if (!I_sendSocket) {
		[self createSendSocket];
	}
	NSData *sendData = [self configDataHigh:NO];
    CFSocketError err = CFSocketSendData (
        I_sendSocket,
        _targetAddressData,
        	(CFDataRef)sendData,
        0.5
        );
    if (err != kCFSocketSuccess) {
//        NSLog(@"%s could not send data (%d, %@, %u bytes)",__FUNCTION__,err,[NSString stringWithAddressData:(NSData *)_targetAddressData],sendData);
        
    } else {
//        NSLog(@"%s did send data: (%@, %u bytes)",__FUNCTION__,[NSString stringWithAddressData:(NSData *)_targetAddressData],sendData);
    };
    sendData = [self configDataHigh:YES];
    err = CFSocketSendData (
        I_sendSocket,
        _targetAddressData,
        	(CFDataRef)sendData,
        0.5
        );
    if (err != kCFSocketSuccess) {
//        NSLog(@"%s could not send data (%d, %@, %u bytes)",__FUNCTION__,err,[NSString stringWithAddressData:(NSData *)_targetAddressData],sendData);
        
    } else {
//        NSLog(@"%s did send data: (%@, %u bytes)",__FUNCTION__,[NSString stringWithAddressData:(NSData *)_targetAddressData],sendData);
    };

	[[NSUserDefaults standardUserDefaults] setObject:I_values forKey:@"GammaTable"];
}


- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return NUMBER_OF_SECTIONS;
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section {
	if (section == SETTINGS_SECTION) {
		return @"Settings";
	} else 	if (section == ADDRESS_SECTION) {
		return @"Target Address";
	} else if (section == SLIDER_SECTION) {
	    return @"Gamma Table";
	} else {
	    return 0;
	}
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
	if (section == SETTINGS_SECTION) {
		return 2;
	} else if (section == ADDRESS_SECTION) {
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
	} else if (section == SETTINGS_SECTION) {
		CellIdentifier = @"SettingsCell";
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

	if (section == SETTINGS_SECTION) {
		if (indexPath.row == 0) {
			cell.text = @"Gamma Table: <default>";
		} else {
			cell.text = @"Addressing All";
		}
		cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
	} else if (section == ADDRESS_SECTION) {
		cell.text = @"10.0.1.201";
	} else if (section == SLIDER_SECTION) {
		MCUGammaAdjustSliderCell *adjustCell = (MCUGammaAdjustSliderCell *)cell;
		int shadeNumber = 0xf-indexPath.row;
		adjustCell.shadeNumber = shadeNumber;
		adjustCell.shadeValue  = [[I_values objectAtIndex:shadeNumber] intValue];
		adjustCell.delegate = self;
	}
    
    return cell;
}

- (void)sliderCellDidChangeValue:(MCUGammaAdjustSliderCell *)inCell
{
	[I_values replaceObjectAtIndex:inCell.shadeNumber withObject:[NSNumber numberWithInt:inCell.shadeValue]];
//	NSLog(@"%s %d->%d",__FUNCTION__,inCell.shadeNumber,inCell.shadeValue);
	[self sendConfig];
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

- (void)viewWillAppear:(BOOL)animated {
    [self.tableView reloadData];
	[super viewWillAppear:animated];
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	if (indexPath.section == SETTINGS_SECTION) {
		if (indexPath.row == 0) {
			[self.navigationController pushViewController:gammaTableSettingsController animated:YES];
		}
	}
}

- (NSIndexPath *)tableView:(UITableView *)tableView willSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	if (indexPath.section == SETTINGS_SECTION) {
		return indexPath;
	} else {
		return nil;
	}
}


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


@end

