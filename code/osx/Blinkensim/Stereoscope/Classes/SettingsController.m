#import "SettingsController.h"
#import "TableSection.h"
#import "AppController.h"
#import <QuartzCore/QuartzCore.h>

#define SECTIONS_BEFORE_PROXY_LIST 1

static NSString * const proxyCellIdentifier = @"ProxyCell";
static NSString * const switchCellIdentifier = @"SwitchCell";
static NSString * const labelCellIdentifier = @"LabelCell";

@class AppController;

@implementation SettingsController
@synthesize projectTableSections;

- (NSDictionary *)manualProxy {
	NSArray *components = [[[NSUserDefaults standardUserDefaults] stringForKey:@"blinkenproxyAddress"] componentsSeparatedByString:@":"];
	NSString *address = [components count]>0 ? [components objectAtIndex:0] : @"";
	NSString *port    = [components count]>1 ? [components objectAtIndex:1] : nil;
	return [NSDictionary dictionaryWithObjectsAndKeys:
					address,@"address",
					port, @"port", // warning if no port port dictionary stops here
					nil];
}

- (BOOL)manualProxyIsSet {
	NSString *proxy = [[NSUserDefaults standardUserDefaults] stringForKey:@"blinkenproxyAddress"];
	return (proxy && [proxy length]);
}

- (void)innerInit
{
		projectTableSections = [NSMutableArray new];
		NSArray *savedStreams = [[NSUserDefaults standardUserDefaults] objectForKey:@"blinkenArray"];
		if (savedStreams) {
			[self updateWithBlinkenstreams:savedStreams];
		}
}

- (id)initWithStyle:(UITableViewStyle)style {
//	NSLog(@"%s",__FUNCTION__);
	if ((self = [super initWithStyle:style])) {
		[self innerInit];
	}
	return self;
}

- (id)initWithCoder:(NSCoder *)inCoder
{
	id result = [super initWithCoder:inCoder];
	if (result) {
		[self innerInit];
	}
	
	return result;
}

- (void)updateWithBlinkenstreams:(NSArray *)inBlinkenArray
{
	[[NSUserDefaults standardUserDefaults] setObject:inBlinkenArray forKey:@"blinkenArray"];
	[projectTableSections removeAllObjects];
	for (NSDictionary *project in inBlinkenArray)
	{
		NSString *name = [project objectForKey:@"name"];
		if ([[project objectForKey:@"building"] isEqualToString:@"stereoscope"]) {
			NSMutableArray *proxyArray = [NSMutableArray new];
			for (NSDictionary *proxy in [project objectForKey:@"proxies"])
			{
				if ([[proxy valueForKey:@"size"] isEqualToString:@"displayed"]) {
					NSMutableDictionary *proxyDict = [proxy mutableCopy];
					[proxyDict setValue:name forKey:@"projectName"];
					[proxyDict setValue:[project valueForKey:@"building"] forKey:@"projectBuilding"];
					[proxyArray addObject:proxy];
				}
			}
			if ([proxyArray count]) {
				TableSection *section = [TableSection sectionWithItems:proxyArray heading:name indexLabel:@""];
				section.representedObject = project;
				[projectTableSections addObject:section];
			}
			[proxyArray release];
		}
	}
	
	[self.tableView reloadData];
}


- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
	return [projectTableSections count] + 1 + SECTIONS_BEFORE_PROXY_LIST;
}


- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
	if (section == 0) {
		return 1;
	} else if (section < [projectTableSections count] + SECTIONS_BEFORE_PROXY_LIST) {
		return [[[projectTableSections objectAtIndex:section - SECTIONS_BEFORE_PROXY_LIST] items] count];
	} else {
		return ([self manualProxyIsSet] ? 2 : 1);
	}
}

- (NSString *)tableView:(UITableView *)tv titleForHeaderInSection:(NSInteger)section {
	if (section == 0) {
		return @"";
	} else if (section < [projectTableSections count] + SECTIONS_BEFORE_PROXY_LIST) {
		return [[projectTableSections objectAtIndex:section - SECTIONS_BEFORE_PROXY_LIST] heading];
	} else {
		return @"Manual Feed Address";
	}
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
	
	NSString *identifier = proxyCellIdentifier;
	if (indexPath.section < SECTIONS_BEFORE_PROXY_LIST) {
		if (indexPath.row == 0) {
			identifier = switchCellIdentifier;
		} else {
			identifier = labelCellIdentifier;
		}
	}
	
	UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:identifier];
	if (cell == nil) {
		cell = [[[UITableViewCell alloc] initWithFrame:CGRectZero reuseIdentifier:identifier] autorelease];
		if (identifier == switchCellIdentifier) {
			UISwitch *mySwitch = [[UISwitch alloc] initWithFrame:CGRectMake(0.0,0.0,10.0,10.0)];
			[cell setAccessoryView:mySwitch];
		}
	}
	if (indexPath.section == 0) {
		cell.text = @"Autoselect Feed";
		UISwitch *mySwitch = ((UISwitch *)cell.accessoryView);
		[mySwitch setOn:[[[NSUserDefaults standardUserDefaults] objectForKey:@"autoselectProxy"] boolValue] animated:NO];
		[mySwitch addTarget:self action:@selector(takeValueForAutoselect:) forControlEvents:UIControlEventValueChanged];
	} else if (indexPath.section < [projectTableSections count] + SECTIONS_BEFORE_PROXY_LIST) {
		NSDictionary *proxy = [[[projectTableSections objectAtIndex:indexPath.section - SECTIONS_BEFORE_PROXY_LIST] items] objectAtIndex:indexPath.row];
		NSString *baseString = [proxy objectForKey:@"name"] ? [proxy objectForKey:@"name"] : [NSString stringWithFormat:@"%@:%@",[proxy objectForKey:@"address"], [proxy objectForKey:@"port"]];
		if ([proxy objectForKey:@"kind"]) {
			baseString = [baseString stringByAppendingFormat:@" (%@)",[proxy objectForKey:@"kind"]];
		}
		cell.text = baseString;
		cell.accessoryType = [proxy isEqual:[[AppController sharedAppController] currentProxy]] ? UITableViewCellAccessoryCheckmark : UITableViewCellAccessoryNone;
	} else {
		if (indexPath.row == 0 && [self manualProxyIsSet]) {
			NSString *address = [[NSUserDefaults standardUserDefaults] stringForKey:@"blinkenproxyAddress"];
			cell.text = address;
			cell.accessoryType = [[self manualProxy] isEqual:[[AppController sharedAppController] currentProxy]] ? UITableViewCellAccessoryCheckmark : UITableViewCellAccessoryNone;
		} else {
			cell.text = @"Other...";
			cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
		}
	}
	return cell;
}

- (void)takeValueForAutoselect:(id)inSender
{
	[[NSUserDefaults standardUserDefaults] setBool:[inSender isOn] forKey:@"autoselectProxy"];
	if ([inSender isOn]) {
		[[AppController sharedAppController] connectionDidBecomeAvailable];
	}
}

@synthesize editingViewController;

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {


	if (indexPath.section < [projectTableSections count] + SECTIONS_BEFORE_PROXY_LIST &&
		indexPath.section >= SECTIONS_BEFORE_PROXY_LIST) {
		NSDictionary *proxy = [[[projectTableSections objectAtIndex:indexPath.section - SECTIONS_BEFORE_PROXY_LIST] items] objectAtIndex:indexPath.row];
		[[AppController sharedAppController] connectToProxy:proxy];
		[[AppController sharedAppController] doneWithSettings:nil];
	} else if (indexPath.section == [projectTableSections count] + SECTIONS_BEFORE_PROXY_LIST) {
		if (indexPath.row == 0 && [self manualProxyIsSet]) {
			[[AppController sharedAppController] connectToManualProxy];
			[[AppController sharedAppController] doneWithSettings:nil];
		} else {
			// Create the editing view controller if necessary.
			if (editingViewController == nil) {
				EditingViewController *viewController = [[EditingViewController alloc] initWithNibName:@"EditingView" bundle:nil];
				self.editingViewController = viewController;
				[viewController release];
			}
			
			editingViewController.editedObject = [NSUserDefaults standardUserDefaults];
			NSString *key = @"blinkenproxyAddress";
			editingViewController.editedFieldKey = key;
			editingViewController.textValue = [[NSUserDefaults standardUserDefaults] stringForKey:key];
			editingViewController.titleString = @"Feed Address";
			editingViewController.dateEditing = NO;
		
			[self.navigationController pushViewController:editingViewController animated:YES];
		}
	}
}

- (NSIndexPath *)tableView:(UITableView *)tableView willSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	if (indexPath.section < SECTIONS_BEFORE_PROXY_LIST) {
		return nil;
	} else {
		return indexPath;
	}
}

/*
- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
}
*/
/*
- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath {
	
	if (editingStyle == UITableViewCellEditingStyleDelete) {
	}
	if (editingStyle == UITableViewCellEditingStyleInsert) {
	}
}
*/
/*
- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath {
	return YES;
}
*/
/*
- (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath toIndexPath:(NSIndexPath *)toIndexPath {
}
*/
/*
- (BOOL)tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath {
	return YES;
}
*/


- (void)dealloc {
	[super dealloc];
}


- (void)viewDidLoad {
	[super viewDidLoad];
	self.tableView.tableHeaderView = _ibTopView;
//	CGPoint oldCenter = _ibCodingMonkeysButton.center;
//	_ibCodingMonkeysButton.center = CGPointMake(CGRectGetMaxX(_ibCodingMonkeysButton.frame),_ibCodingMonkeysButton.center.y);
//	[_ibCodingMonkeysButton.layer setAnchorPoint:CGPointMake(0.896,0.6)];
//	oldCenter.x += 55.;
//	_ibCodingMonkeysButton.center = oldCenter;
}


- (void)viewWillAppear:(BOOL)animated {
	[self.tableView reloadData];
	[super viewWillAppear:animated];
}

- (void)viewDidAppear:(BOOL)animated {
	[super viewDidAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated {
}

- (void)viewDidDisappear:(BOOL)animated {
}

- (void)didReceiveMemoryWarning {
	[super didReceiveMemoryWarning];
}

- (IBAction)gotoWebsite:(id)inSender {
	[[UIApplication sharedApplication] openURL:[NSURL URLWithString:@"http://www.blinkenlights.net/stereoscope"]];
}

- (IBAction)gotoBlinkenlightsWebsite:(id)inSender {
	[[UIApplication sharedApplication] openURL:[NSURL URLWithString:@"http://www.blinkenlights.net/stereoscope"]];
}

- (IBAction)gotoCodingMonkeysWebsite:(id)inSender {
	[[UIApplication sharedApplication] openURL:[NSURL URLWithString:@"http://www.codingmonkeys.de/"]];
}

- (IBAction)dismissAbout:(id)inSender
{
	[self dismissModalViewControllerAnimated:YES];
}

#define MONKEY_ROTATION (M_PI / 16.0)

- (IBAction)showAbout:(id)inSender 
{
	[self presentModalViewController:_ibAboutViewController animated:YES];
	
	_animationState = 0;
	
	[UIView beginAnimations:@"MonkeyAnimation" context:NULL];
	[UIView setAnimationDelay:2.0];
	[UIView setAnimationDuration:0.3];
	[UIView setAnimationDelegate:self];
	[UIView setAnimationDidStopSelector:@selector(monkeyAnimationDidEnd:context:)];
	[_ibCodingMonkeysButton setTransform:CGAffineTransformMakeRotation(-MONKEY_ROTATION)];
	[UIView commitAnimations];
}

- (void)monkeyAnimationDidEnd:(id)animationId context:(void *)inContext
{
	CGFloat _targetRotation = _animationState++ % 2 ? MONKEY_ROTATION : MONKEY_ROTATION;
	[UIView beginAnimations:@"MonkeyAnimation" context:NULL];
	
	if (_animationState < 3) {
		[UIView setAnimationDuration:0.6];
		[UIView setAnimationDelegate:self];
		[UIView setAnimationDidStopSelector:@selector(monkeyAnimationDidEnd:context:)];
		[_ibCodingMonkeysButton setTransform:CGAffineTransformMakeRotation(_targetRotation)];
	} else {
		[UIView setAnimationDuration:0.3];
		[_ibCodingMonkeysButton setTransform:CGAffineTransformMakeRotation(0)];
	}
	[UIView commitAnimations];
}


@end

