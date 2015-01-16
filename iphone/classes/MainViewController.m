/*
 * Copyright 2010 Forwardcode, LLC
 *
 * This file is part of Ventafone.
 *
 * Ventafone is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Ventafone is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Ventafone.  If not, see <http://www.gnu.org/licenses/>.
 */

#import "MainViewController.h"

@implementation MainViewController

@synthesize myTableView;
@synthesize mySettingsViewController;
@synthesize myAddServerViewController;
@synthesize myServerViewController;
@synthesize myAboutViewController;

- (void)dealloc
{
    [myTableView release];
	if (mySettingsViewController != nil)
		[self.mySettingsViewController release];
	if (myAddServerViewController != nil)
		[self.myAddServerViewController release];
	if (myServerViewController != nil)
		[self.myServerViewController release];
	if (myAboutViewController != nil)
		[self.myAboutViewController release];
	[super dealloc];
}

- (void)viewDidLoad
{
	[super viewDidLoad];
	appDelegate = (VentafoneAppDelegate *)[[UIApplication sharedApplication] delegate];
	self.myTableView.allowsSelectionDuringEditing = YES;
}

- (void)viewDidAppear:(BOOL)animated
{
	[super viewDidAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated
{
	[super viewWillDisappear:animated];
}

- (void)viewDidDisappear:(BOOL)animated
{
	[super viewDidDisappear:animated];
}

- (void)viewDidUnload
{
	[super viewDidUnload];
	self.myTableView = nil;
}

- (void)viewWillAppear:(BOOL)animated
{	
	[super viewWillAppear:animated];
	
	self.navigationItem.title = @"Servers";
	
	UIBarButtonItem *editButton = [[UIBarButtonItem alloc] initWithTitle:@"Edit" style:UIBarButtonItemStyleBordered target:self action:@selector(editingAction)];
	self.navigationItem.leftBarButtonItem = editButton;
	[editButton release];
	
	UIBarButtonItem *addButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemAdd target:self action:@selector(addServerViewAction)];
	self.navigationItem.rightBarButtonItem = addButton;
	[addButton release];
	
	
	
	[self.myTableView deselectRowAtIndexPath:self.myTableView.indexPathForSelectedRow animated:NO];
	if ([self.myTableView isEditing]) {
		[self.myTableView setEditing:NO animated:NO];
		[self.navigationItem.leftBarButtonItem setTitle:@"Edit"];
		[self.navigationItem.leftBarButtonItem setStyle:UIBarButtonItemStyleBordered];
		
		UIBarButtonItem *addButton = [[UIBarButtonItem alloc]
									  initWithBarButtonSystemItem:UIBarButtonSystemItemAdd target:self action:@selector(addServerViewAction)];
		[self.navigationItem setRightBarButtonItem:addButton animated:NO];
		[addButton release];
		
	}
	
	[self.myTableView reloadData];
	
	if ([[appDelegate serverList] count] <= 0)
	{
		// No servers available.. slide up the add server window
		[self presentAddServerViewControllerWithServerIndex:-1];
	}
	
	//	// Create a final modal view controller
	//	UIButton* modalViewButton = [UIButton buttonWithType:UIButtonTypeInfoLight];
	//	[modalViewButton addTarget:self action:@selector(modalViewAction:) forControlEvents:UIControlEventTouchUpInside];
	//	UIBarButtonItem *modalBarButtonItem = [[UIBarButtonItem alloc] initWithCustomView:modalViewButton];
	//	self.navigationItem.rightBarButtonItem = modalBarButtonItem;
	//	[modalBarButtonItem release];
}

- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}

#pragma mark -
#pragma mark UITableViewDataSource

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
	return [[appDelegate serverList] count];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
	static NSString *kCellIdentifier = @"cellID";
	
	UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:kCellIdentifier];
	if (!cell)
	{
		cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:kCellIdentifier] autorelease];
        
		cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
		cell.showsReorderControl = YES;
        
        cell.textLabel.backgroundColor = [UIColor clearColor];
		cell.textLabel.opaque = NO;
		cell.textLabel.textColor = [UIColor blackColor];
		cell.textLabel.highlightedTextColor = [UIColor whiteColor];
		cell.textLabel.font = [UIFont boldSystemFontOfSize:18];
		
		cell.detailTextLabel.backgroundColor = [UIColor clearColor];
		cell.detailTextLabel.opaque = NO;
		cell.detailTextLabel.textColor = [UIColor grayColor];
		cell.detailTextLabel.highlightedTextColor = [UIColor whiteColor];
		cell.detailTextLabel.font = [UIFont systemFontOfSize:14];
    }
	
	// Get the server for this row
	ServerSettings *server = [[appDelegate serverList] objectAtIndex:indexPath.row];
	cell.textLabel.text = [server hostAlias];
	cell.detailTextLabel.text = [server userAlias];
	
	
	return cell;
}

- (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath toIndexPath:(NSIndexPath *)toIndexPath {
	if (toIndexPath.row != fromIndexPath.row) {
		id obj = [[appDelegate serverList] objectAtIndex:fromIndexPath.row];
        [obj retain];
		[[appDelegate serverList] removeObjectAtIndex:fromIndexPath.row];
        if (toIndexPath.row >= [[appDelegate serverList] count]) {
			[[appDelegate serverList] addObject:obj];
        } else {
			[[appDelegate serverList] insertObject:obj atIndex:toIndexPath.row];
        }
        [obj release];
		[appDelegate saveServersToDisk];
    }
	
}

- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath {
    if (editingStyle == UITableViewCellEditingStyleDelete) {
		[[appDelegate serverList] removeObjectAtIndex:indexPath.row];
		[appDelegate saveServersToDisk];
		[self.myTableView deleteRowsAtIndexPaths:[NSArray arrayWithObject:indexPath] withRowAnimation:UITableViewRowAnimationFade];
	}
}


#pragma mark -
#pragma mark UITableViewDelegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{	
	if (self.myTableView.editing)
	{
		[self presentAddServerViewControllerWithServerIndex:indexPath.row];
	}
	else
	{
		if ([appDelegate connectionAvailable] && [appDelegate micAvailable]) {
			if (myServerViewController != nil) {
				[self.myServerViewController release];
			}
			
			NSString *viewControllerName = @"ServerViewController";
			self.myServerViewController = [[NSClassFromString(viewControllerName) alloc] initWithServerSettingsIndex:indexPath.row];
//			self.myServerViewController = [[NSClassFromString(viewControllerName) alloc] initWithNibName:viewControllerName bundle:nil];
//			[self.myServerViewController setServerIndex:indexPath.row];
			
			[self.navigationController pushViewController:self.myServerViewController animated:YES];
		}
		else {
			[self.myTableView deselectRowAtIndexPath:self.myTableView.indexPathForSelectedRow animated:YES];
		}

		
	}
}

- (BOOL)tableView:(UITableView *)tableview canMoveRowAtIndexPath:(NSIndexPath *)indexPath {
	return YES;	
}

#pragma mark -
#pragma mark Execution code

- (void)disconnectTask
{
	//NSLog(@"MainViewController >>> (disconnectTask) >>> entering");
	[self.myServerViewController disconnectFromServer];
}
	 
#pragma mark -
#pragma mark MBProgressHUDDelegate methods
	 
- (void)hudWasHidden
{
	self.navigationItem.leftBarButtonItem.enabled = YES;
	self.navigationItem.rightBarButtonItem.enabled = YES;
	
	// Remove HUD from screen when the HUD was hidded
	[HUD removeFromSuperview];
	[HUD release];
}

#pragma mark -
#pragma mark API

- (void) presentAddServerViewControllerWithServerIndex:(NSInteger)aServerIndex
{
	if (myAddServerViewController == nil)
        self.myAddServerViewController = [[[AddServerViewController alloc] initWithNibName:
										   NSStringFromClass([AddServerViewController class]) bundle:nil] autorelease];


	[self.myAddServerViewController setServerIndex:aServerIndex];
	
	[self.navigationController presentModalViewController:self.myAddServerViewController animated:YES];
}

- (void) disconnectWithHUDDisplay
{
	//NSLog(@"MainViewController >>> (disconnectWithHUDDisplay) >>> entering");
	self.navigationItem.leftBarButtonItem.enabled = NO;
	self.navigationItem.rightBarButtonItem.enabled = NO;
	
	// The hud will dispable all input on the view
    HUD = [[MBProgressHUD alloc] initWithView:self.view];
	// Add HUD to screen
    [self.view addSubview:HUD];
	// Regisete for HUD callbacks so we can remove it from the window at the right time
    HUD.delegate = self;
	HUD.labelText = @"Disconnecting";
	// Show the HUD while the provided method executes in a new thread
    [HUD showWhileExecuting:@selector(disconnectTask) onTarget:self withObject:nil animated:YES];
	//[self connectToServer];
}

- (IBAction)addServerViewAction
{
    [self presentAddServerViewControllerWithServerIndex:-1];

	//AddServerViewController *addServerViewController = [[AddServerViewController alloc] initWithNibName:NSStringFromClass([AddServerViewController class]) bundle:nil];
	//UINavigationController *controller = [[UINavigationController alloc] initWithRootViewController:addServerViewController];
	//[[self navigationController] presentModalViewController:controller animated:YES];
	//[addServerViewController release];
	//[controller release];
	
	
//	UINavigationController *controller = [[UINavigationController alloc] initWithRootViewController:newTransactionViewController];
//	[[self navigationController] presentModalViewController:controller animated:YES];
}

- (IBAction)settingsViewAction
{
    if (mySettingsViewController == nil)
        self.mySettingsViewController = [[[SettingsViewController alloc] initWithNibName:
										   NSStringFromClass([SettingsViewController class]) bundle:nil] autorelease];
	
	[self.navigationController presentModalViewController:self.mySettingsViewController animated:YES];
	
}

- (IBAction)editingAction
{
	if ([self.myTableView isEditing]) {
		[self.myTableView setEditing:NO animated:YES];
		[self.navigationItem.leftBarButtonItem setTitle:@"Edit"];
		[self.navigationItem.leftBarButtonItem setStyle:UIBarButtonItemStyleBordered];
		
		UIBarButtonItem *addButton = [[UIBarButtonItem alloc]
									  initWithBarButtonSystemItem:UIBarButtonSystemItemAdd target:self action:@selector(addServerViewAction)];
		[self.navigationItem setRightBarButtonItem:addButton animated:YES];
		[addButton release];
		
	}
	else {
		[self.myTableView setEditing:YES animated:YES];
		[self.navigationItem.leftBarButtonItem setTitle:@"Done"];
		[self.navigationItem.leftBarButtonItem setStyle:UIBarButtonItemStyleDone];
		
		[self.navigationItem setRightBarButtonItem:nil animated:YES];
	}

}

- (void)reloadTable
{
	// Server was added or edited.. reload data
	[self.myTableView reloadData];
	return;
}

- (void)showInfo
{
	if (myAboutViewController == nil)
        self.myAboutViewController = [[[AboutViewController alloc] initWithNibName:
										  NSStringFromClass([AboutViewController class]) bundle:nil] autorelease];
	
	[self.navigationController presentModalViewController:self.myAboutViewController animated:YES];
}

@end

