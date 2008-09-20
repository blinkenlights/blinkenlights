#import "SettingsController.h"
#import "TableSection.h"

#define SECTIONS_BEFORE_PROXY_LIST 1

static NSString * const proxyCellIdentifier = @"ProxyCell";
static NSString * const switchCellIdentifier = @"SwitchCell";
static NSString * const labelCellIdentifier = @"LabelCell";

@class AppController;

@implementation SettingsController
@synthesize projectTableSections;

- (void)innerInit
{
		projectTableSections = [NSMutableArray new];
		NSArray *savedStreams = [[NSUserDefaults standardUserDefaults] objectForKey:@"blinkenArray"];
		if (savedStreams) {
			[self updateWithBlinkenstreams:savedStreams];
		}
}

- (id)initWithStyle:(UITableViewStyle)style {
	NSLog(@"%s",__FUNCTION__);
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
		return 1;
	}
}

- (NSString *)tableView:(UITableView *)tv titleForHeaderInSection:(NSInteger)section {
	if (section == 0) {
		return @"";
	} else if (section < [projectTableSections count] + SECTIONS_BEFORE_PROXY_LIST) {
		return [[projectTableSections objectAtIndex:section - SECTIONS_BEFORE_PROXY_LIST] heading];
	} else {
		return @"Current Blinkenproxy Address";
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
		cell.text = @"Autoselect Stream";
		((UISwitch *)cell.accessoryView).isOn = [[[NSUserDefaults standardUserDefaults] objectForKey:@"autoselectProxy"] boolValue];
	} else if (indexPath.section < [projectTableSections count] + SECTIONS_BEFORE_PROXY_LIST) {
		NSDictionary *proxy = [[[projectTableSections objectAtIndex:indexPath.section - SECTIONS_BEFORE_PROXY_LIST] items] objectAtIndex:indexPath.row];
		cell.text = [NSString stringWithFormat:@"%@:%@",[proxy objectForKey:@"address"], [proxy objectForKey:@"port"]];
	} else {
		cell.text = [[NSUserDefaults standardUserDefaults] stringForKey:@"blinkenproxyAddress"];
	}
	return cell;
}

@synthesize editingViewController;

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {


	if (indexPath.section < [projectTableSections count] + SECTIONS_BEFORE_PROXY_LIST &&
		indexPath.section >= SECTIONS_BEFORE_PROXY_LIST) {
		NSDictionary *proxy = [[[projectTableSections objectAtIndex:indexPath.section - SECTIONS_BEFORE_PROXY_LIST] items] objectAtIndex:indexPath.row];
		NSString *address = [NSString stringWithFormat:@"%@:%@",[proxy objectForKey:@"address"], [proxy objectForKey:@"port"]];
		[[NSUserDefaults standardUserDefaults] setObject:address forKey:@"blinkenproxyAddress"];
		[[AppController sharedAppController] connectToProxy:proxy];
//	    [self.navigationController popViewControllerAnimated:YES];
		UIBarButtonItem *item = self.navigationController.navigationBar.topItem.leftBarButtonItem;
		[item.target performSelector:item.action withObject:item];
	} else if (indexPath.section == [projectTableSections count] + SECTIONS_BEFORE_PROXY_LIST) {
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
		editingViewController.titleString = @"Blinkenproxy Address";
		editingViewController.dateEditing = NO;
	
		[self.navigationController pushViewController:editingViewController animated:YES];
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
	[[UIApplication sharedApplication] openURL:[NSURL URLWithString:@"http://www.blinkenlights.net/"]];
}

- (IBAction)gotoBlinkenlightsWebsite:(id)inSender {
	[[UIApplication sharedApplication] openURL:[NSURL URLWithString:@"http://www.blinkenlights.net/"]];
}

- (IBAction)gotoCodingMonkeysWebsite:(id)inSender {
	[[UIApplication sharedApplication] openURL:[NSURL URLWithString:@"http://www.codingmonkeys.de/"]];
}

- (IBAction)dismissAbout:(id)inSender
{
	[self dismissModalViewControllerAnimated:YES];
}

- (IBAction)showAbout:(id)inSender 
{
	[self presentModalViewController:_ibAboutViewController animated:YES];
}


@end

