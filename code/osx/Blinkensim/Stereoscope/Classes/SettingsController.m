#import "SettingsController.h"
#import "TableSection.h"

@implementation SettingsController

- (void)innerInit
{
		projectTableSections = [NSMutableArray new];
		NSDictionary *savedStreams = [[NSUserDefaults standardUserDefaults] objectForKey:@"blinkenDict"];
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

- (void)updateWithBlinkenstreams:(NSDictionary *)inBlinkenDict 
{
	[[NSUserDefaults standardUserDefaults] setObject:inBlinkenDict forKey:@"blinkenDict"];
	[projectTableSections removeAllObjects];
	for (NSString *projectKey in [[inBlinkenDict allKeys] sortedArrayUsingSelector: @selector(caseInsensitiveCompare:)])
	{
		NSDictionary *project = [inBlinkenDict objectForKey:projectKey];
		if ([[project objectForKey:@"building"] isEqualToString:@"stereoscope"]) {
			TableSection *section = [TableSection sectionWithItems:[project objectForKey:@"proxies"] heading:projectKey indexLabel:@""];
			section.representedObject = project;
			[projectTableSections addObject:section];
		}
	}
	
	[self.tableView reloadData];
}


- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
	return [projectTableSections count] + 1;
}


- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
	if (section < [projectTableSections count]) {
		return [[[projectTableSections objectAtIndex:section] items] count];
	} else {
		return 1;
	}
}

- (NSString *)tableView:(UITableView *)tv titleForHeaderInSection:(NSInteger)section {
	if (section < [projectTableSections count]) {
		return [[projectTableSections objectAtIndex:section] heading];
	} else {
		return @"Current Blinkenproxy Address";
	}
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
	
	static NSString *MyIdentifier = @"MyIdentifier";
	
	UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:MyIdentifier];
	if (cell == nil) {
		cell = [[[UITableViewCell alloc] initWithFrame:CGRectZero reuseIdentifier:MyIdentifier] autorelease];
	}
	
	if (indexPath.section < [projectTableSections count]) {
		NSDictionary *proxy = [[[projectTableSections objectAtIndex:indexPath.section] items] objectAtIndex:indexPath.row];
		cell.text = [NSString stringWithFormat:@"%@:%@",[proxy objectForKey:@"address"], [proxy objectForKey:@"port"]];
	} else {
		cell.text = [[NSUserDefaults standardUserDefaults] stringForKey:@"blinkenproxyAddress"];
	}
	return cell;
}

@synthesize editingViewController;

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {


	if (indexPath.section < [projectTableSections count]) {
		NSDictionary *proxy = [[[projectTableSections objectAtIndex:indexPath.section] items] objectAtIndex:indexPath.row];
		NSString *address = [NSString stringWithFormat:@"%@:%@",[proxy objectForKey:@"address"], [proxy objectForKey:@"port"]];
		[[NSUserDefaults standardUserDefaults] setObject:address forKey:@"blinkenproxyAddress"];
		[[NSNotificationCenter defaultCenter] postNotificationName:@"SettingChange" object:self];
//	    [self.navigationController popViewControllerAnimated:YES];
		UIBarButtonItem *item = self.navigationController.navigationBar.topItem.leftBarButtonItem;
		[item.target performSelector:item.action withObject:item];
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


@end

