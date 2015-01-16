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

#import "ServerViewController.h"
#import "VentafoneAppDelegate.h"
#import "User.h"
#import "Channel.h"
#import "Utility.h"
#import "TableOrder.h"
#import "TableItem.h"

//#define rowHeight 35.0 //45.0
#define indentWidth 20
#define statusTag 1
#define nameTag 2

@class User;
@class Channel;
@class TableOrder;
@class TableItem;

@implementation ServerViewController

@synthesize isRecording;
@synthesize connected;
@synthesize pushFinished;
@synthesize eventService;
@synthesize serverIndex;
@synthesize rowHeight;
@synthesize myTableView;
@synthesize channelList;
@synthesize userList;
@synthesize tableOrder;
@synthesize recordingBackground;
@synthesize recordingButton;
@synthesize myUserViewController;

uint16_t selectedChannel;

- (void)dealloc {
	[myTableView release];
	[recordingBackground release];
	[recordingButton release];
	[channelList release];
	[userList release];
	if (myUserViewController != nil)
		[self.myUserViewController release];
	if (tableOrder != nil)
	{
		[tableOrder release];
	}
	if (eventService != nil) {
		[eventService release];
	}
	
	//eventService = nil;
    [super dealloc];
}

- (id)initWithServerSettingsIndex:(NSInteger)aServerIndex
{
    if (self = [super initWithNibName:@"ServerViewController" bundle:nil])
    {
		pushFinished = NO;
		serverIndex = aServerIndex;
		connected = YES;
    }
    return self;
}

// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {

	appDelegate = (VentafoneAppDelegate *)[[UIApplication sharedApplication] delegate];
	
	if (serverIndex >= 0) {
		self.title = [[[appDelegate serverList] objectAtIndex:serverIndex] hostAlias];
	}
	else {
		self.title = @"Error";
	}
	self.navigationItem.hidesBackButton = YES;

//	UIImageView *backgroundImage = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"carbonfiber"]];
//	[self.view addSubview:backgroundImage];
//	[self.view sendSubviewToBack:backgroundImage];
//	[backgroundImage release];
//	self.view.backgroundColor = [UIColor colorWithPatternImage:[UIImage imageNamed:@"steel"]];
	
	self.channelList = [[NSMutableDictionary alloc] initWithCapacity:0];
	self.userList = [[NSMutableDictionary alloc] initWithCapacity:0];
	//UIImage *anImage = [UIImage imageNamed:@"ServerImage.png"];
	[recordingButton setHidden:YES];
	[recordingBackground setHidden:YES];
	isRecording = NO;
	//connected = YES;
	
	//[myTableView setRowHeight:rowHeight];
	if ([[appDelegate globalSettings] smallRowHeight]) {
		rowHeight = 35;
	}
	else {
		rowHeight = 45;
	}

	
//	UIButton *infoButton = [UIButton buttonWithType:UIButtonTypeInfoLight];
//	[infoButton addTarget:self action:@selector(showInfo) forControlEvents:UIControlEventTouchUpInside];
//	self.navigationItem.rightBarButtonItem = [[[UIBarButtonItem alloc] initWithCustomView:infoButton] autorelease];
	
	//NSLog(@"ServerViewController >>> (viewDidAppear) >>> entering");
	// The hud will dispable all input on the view
    HUD = [[MBProgressHUD alloc] initWithView:self.view];
	// Add HUD to screen
    [self.view addSubview:HUD];
	// Regisete for HUD callbacks so we can remove it from the window at the right time
    HUD.delegate = self;
	HUD.labelText = @"Connecting";
	recordingToggle = [[appDelegate globalSettings] toggle];
	// Show the HUD while the provided method executes in a new thread
    
	
    [super viewDidLoad];
}

- (void)viewWillAppear:(BOOL)animated
{
	[super viewWillAppear:animated];

	[self reloadData];
}

- (void)viewDidAppear:(BOOL)animated
{
//	//NSLog(@"ServerViewController >>> (viewDidAppear) >>> entering");
//	// The hud will dispable all input on the view
//    HUD = [[MBProgressHUD alloc] initWithView:self.view];
//	// Add HUD to screen
//    [self.view addSubview:HUD];
//	// Regisete for HUD callbacks so we can remove it from the window at the right time
//    HUD.delegate = self;
//	HUD.labelText = @"Connecting";
//	recordingToggle = [[appDelegate globalSettings] toggle];
//	// Show the HUD while the provided method executes in a new thread
//    [HUD showWhileExecuting:@selector(connectTask) onTarget:self withObject:nil animated:YES];
	//[self connectToServer];
	
	if (!pushFinished) {
		pushFinished = YES;
		
		[HUD showWhileExecuting:@selector(connectTask) onTarget:self withObject:nil animated:YES];
	}
	
	[self.myTableView deselectRowAtIndexPath:self.myTableView.indexPathForSelectedRow animated:YES];
	
	[super viewDidAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated
{
	[super viewWillDisappear:animated];
}

- (void)viewDidDisappear:(BOOL)animated
{
	//NSLog(@"ServerViewController >>> (viewDidDisappear) >>> entering");

	[super viewDidDisappear:animated];
}

/*
// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    // Return YES for supported orientations
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}
*/

- (void)didReceiveMemoryWarning {
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}

#pragma mark -
#pragma mark UITableViewDataSource

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
	return ([userList count] + [channelList count] + 1);
}

- (CGFloat) tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
	return rowHeight;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
	static NSString *kCellIdentifier = @"cellID";
	
	UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:kCellIdentifier];
	if (!cell)
	{
		cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:kCellIdentifier] autorelease];
		//UILabel *statusLabel = [[UILabel alloc] initWithFrame:CGRectMake(150, 0, rowHeight, rowHeight)];
		UIImage *scaledImage = [Utility scale:[UIImage imageNamed:@"speaker_stop"] toSize:CGSizeMake(rowHeight, rowHeight)];
		UIImageView *imageView = [[UIImageView alloc] initWithImage:scaledImage];
		imageView.tag = statusTag;
		[cell.contentView addSubview:imageView];
		[imageView release];
		
		CGRect frame;
		frame.origin.x = rowHeight;
		frame.origin.y = 0;
		frame.size.height = rowHeight;
		frame.size.width = 150;
		
		UILabel *nameLabel = [[UILabel alloc] initWithFrame:frame];
		nameLabel.tag = nameTag;
		nameLabel.textAlignment =  UITextAlignmentLeft;
		nameLabel.backgroundColor = [UIColor clearColor];
		//nameLabel.font = [UIFont fontWithName:@"Arial Rounded MT Bold" size:(14.0)];
		nameLabel.font = [UIFont boldSystemFontOfSize:14];
		[cell.contentView addSubview:nameLabel];
		[nameLabel release];
    }
	
	// First row is the server lobby
	if (indexPath.row == 0) {

		cell.selectionStyle = UITableViewCellSelectionStyleBlue;
		cell.accessoryType = UITableViewCellAccessoryNone;
		CGRect newFrame;
		
		UIImage *scaledImage = [Utility scale:[UIImage imageNamed:@"globe"] toSize:CGSizeMake(rowHeight, rowHeight)];
		UIImageView *currentImageView = (UIImageView *)[cell.contentView viewWithTag:statusTag];
		newFrame = currentImageView.frame;
		newFrame.origin.x = 0;
		currentImageView.frame = newFrame;
		currentImageView.image = scaledImage;
		
		UILabel *currentNameLabel = (UILabel *)[cell.contentView viewWithTag:nameTag];
		newFrame = currentNameLabel.frame;
		newFrame.origin.x = rowHeight;
		currentNameLabel.frame = newFrame;
		currentNameLabel.textColor = [UIColor grayColor];
		currentNameLabel.text = [[[appDelegate serverList] objectAtIndex:serverIndex] hostAlias];
	
		return cell;
	}
	
	if (tableOrder != nil) {
		if ([self.tableOrder tableLayout] != nil) {
			if ( [[self.tableOrder tableLayout] count] > (indexPath.row-1) )
			{
				// Get the TableItem for this indexpath row
				TableItem *anItem = [[self.tableOrder tableLayout] objectAtIndex:(indexPath.row-1)];
				NSInteger indentLevel = [anItem level];
				if ([anItem typeFlag] == 0) {
					// User
					cell.selectionStyle = UITableViewCellSelectionStyleBlue;
					//cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
					cell.accessoryType = UITableViewCellAccessoryNone;
					User *currentUser = [anItem user];
					CGRect newFrame;
					
					UIImage *scaledImage;
					if ([currentUser muted]) {
						scaledImage = [Utility scale:[UIImage imageNamed:@"mute"] toSize:CGSizeMake(rowHeight, rowHeight)];
					}
					else {
						if ([currentUser transmitting]) {
							scaledImage = [Utility scale:[UIImage imageNamed:@"speaker_play"] toSize:CGSizeMake(rowHeight, rowHeight)];
						}
						else {
							scaledImage = [Utility scale:[UIImage imageNamed:@"speaker_stop"] toSize:CGSizeMake(rowHeight, rowHeight)];
						}
					}

					
					UIImageView *currentImageView = (UIImageView *)[cell.contentView viewWithTag:statusTag];
					newFrame = currentImageView.frame;
					newFrame.origin.x = indentWidth*indentLevel;
					currentImageView.frame = newFrame;
					currentImageView.image = scaledImage;
					
					UILabel *currentNameLabel = (UILabel *)[cell.contentView viewWithTag:nameTag];
					newFrame = currentNameLabel.frame;
					newFrame.origin.x = rowHeight + (indentWidth*indentLevel);
					currentNameLabel.frame = newFrame;
					currentNameLabel.textColor = [UIColor blackColor];
					currentNameLabel.text = [currentUser userAlias];
				}
				else {
					// Channel
					cell.selectionStyle = UITableViewCellSelectionStyleBlue;
					cell.accessoryType = UITableViewCellAccessoryNone;
					Channel *currentChannel = [anItem channel];
					CGRect newFrame;
					
					UIImage *scaledImage = [Utility scale:[UIImage imageNamed:@"channel_icon"] toSize:CGSizeMake(rowHeight, rowHeight)];
					UIImageView *currentImageView = (UIImageView *)[cell.contentView viewWithTag:statusTag];
					newFrame = currentImageView.frame;
					newFrame.origin.x = indentWidth*indentLevel;
					currentImageView.frame = newFrame;
					currentImageView.image = scaledImage;
					
					UILabel *currentNameLabel = (UILabel *)[cell.contentView viewWithTag:nameTag];
					newFrame = currentNameLabel.frame;
					newFrame.origin.x = rowHeight + (indentWidth*indentLevel);
					currentNameLabel.frame = newFrame;
					currentNameLabel.textColor = [UIColor lightGrayColor];
					currentNameLabel.text = [currentChannel channelName];
				}
			}
		}
	}
	
    
	//	// get the view controller's info dictionary based on the indexPath's row
	//    NSDictionary *dataDictionary = [serverList objectAtIndex:indexPath.row];
	//    cell.textLabel.text = [dataDictionary valueForKey:kTitleKey];
	//    cell.detailTextLabel.text = [dataDictionary valueForKey:kDetailKey];
	
	
	return cell;
}


#pragma mark -
#pragma mark UITableViewDelegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
	UITableViewCell *selectedCell = [self.myTableView cellForRowAtIndexPath:indexPath];
	if (indexPath.row == 0) {
		// User wants to go to the main lobby
		const v3_codec *codec_info;
		if ((codec_info = v3_get_channel_codec(0)))
		{
			// Check the codec first
			if ( (codec_info->codec != 0) && (codec_info->codec != 3) )
			{
				NSString *codecName = [NSString stringWithFormat:@"%s",codec_info->name];
				[appDelegate performSelectorOnMainThread:@selector(channelUnsupportedCodec:) withObject:codecName waitUntilDone:NO];
				[self.myTableView deselectRowAtIndexPath:self.myTableView.indexPathForSelectedRow animated:YES];
				return;
			}
			char *pass = [[NSString stringWithFormat:@""] cStringUsingEncoding:NSUTF8StringEncoding];
			v3_change_channel(0, pass);
//			if ([[appDelegate globalSettings] soundEffects]) {
//				[appDelegate playSoundWithId:@"Channel"];
//			}
			[self.myTableView deselectRowAtIndexPath:self.myTableView.indexPathForSelectedRow animated:YES];
		}
		
	}
	
	if (tableOrder != nil) {
		if ([self.tableOrder tableLayout] != nil) {
			if ( [[self.tableOrder tableLayout] count] > (indexPath.row-1) )
			{
				// Get the TableItem for this indexpath row
				TableItem *anItem = [[self.tableOrder tableLayout] objectAtIndex:(indexPath.row-1)];
				NSInteger indentLevel = [anItem level];
				if ([anItem typeFlag] == 0) {
					// User
					if (myUserViewController == nil)
						self.myUserViewController = [[[UserViewController alloc] initWithNibName:
													   NSStringFromClass([UserViewController class]) bundle:nil] autorelease];
					
					[self.myUserViewController setUid:[[anItem user] userId]];
					
					[self.navigationController presentModalViewController:self.myUserViewController animated:YES];
				}
				else {
					// Channel
					Channel *newChannel = [[[self.tableOrder tableLayout] objectAtIndex:indexPath.row-1] channel];
					uint16_t cid = (uint16_t)[newChannel channelId];
					
					const v3_codec *codec_info;
					if ((codec_info = v3_get_channel_codec(cid)))
					{
						// Check to make sure the new channel uses a supported codec first
						if ( (codec_info->codec != 0) && (codec_info->codec != 3) )
						{
							NSString *codecName = [NSString stringWithFormat:@"%s",codec_info->name];
							[appDelegate performSelectorOnMainThread:@selector(channelUnsupportedCodec:) withObject:codecName waitUntilDone:NO];
							[self.myTableView deselectRowAtIndexPath:self.myTableView.indexPathForSelectedRow animated:YES];
							return;
						}
						
						if (v3_channel_requires_password(cid)) {
							// Channel is password protected
							selectedChannel = cid;
							// Prompt user for password
							//			AlertPrompt *prompt = [AlertPrompt alloc];
							//			prompt = [prompt initWithTitle:@"Channel is password protected" message:@"Please insert a password to join this channel" delegate:self cancelButtonTitle:@"Cancel" okButtonTitle:@"OK"];
							//			[prompt show];
							//			[prompt release];
							
							UIAlertView *passwordAlert = [[UIAlertView alloc] initWithTitle:@"Protected channel" message:@"\n\n\n"
																				   delegate:self cancelButtonTitle:NSLocalizedString(@"Cancel",nil) otherButtonTitles:NSLocalizedString(@"OK",nil), nil];
							
							UILabel *passwordLabel = [[UILabel alloc] initWithFrame:CGRectMake(12,40,260,25)];
							passwordLabel.font = [UIFont systemFontOfSize:16];
							passwordLabel.textColor = [UIColor whiteColor];
							passwordLabel.backgroundColor = [UIColor clearColor];
							passwordLabel.shadowColor = [UIColor blackColor];
							passwordLabel.shadowOffset = CGSizeMake(0,-1);
							passwordLabel.textAlignment = UITextAlignmentCenter;
							passwordLabel.text = @"Please insert the channel password";
							[passwordAlert addSubview:passwordLabel];
							
							UIImageView *passwordImage = [[UIImageView alloc] initWithImage:[UIImage imageWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"passwordfield" ofType:@"png"]]];
							passwordImage.frame = CGRectMake(11,79,262,31);
							[passwordAlert addSubview:passwordImage];
							
							passwordField = [[UITextField alloc] initWithFrame:CGRectMake(16,83,252,25)];
							passwordField.font = [UIFont systemFontOfSize:18];
							passwordField.backgroundColor = [UIColor whiteColor];
							passwordField.secureTextEntry = YES;
							passwordField.keyboardAppearance = UIKeyboardAppearanceAlert;
							passwordField.delegate = self;
							[passwordField becomeFirstResponder];
							[passwordAlert addSubview:passwordField];
							
							//[passwordAlert setTransform:CGAffineTransformMakeTranslation(0,109)];
							[passwordAlert show];
							[passwordAlert release];
							[passwordField release];
							[passwordImage release];
							[passwordLabel release];
							
							//			char *pass = [[NSString stringWithFormat:@""] cStringUsingEncoding:NSUTF8StringEncoding];
							//			v3_change_channel(cid, pass);
							//			[(VentafoneAppDelegate *)[[UIApplication sharedApplication] delegate] playSoundWithId:@"Channel"];
						}
						else {
							// No password required
							char *pass = [[NSString stringWithFormat:@""] cStringUsingEncoding:NSUTF8StringEncoding];
							v3_change_channel(cid, pass);
							//				if ([[appDelegate globalSettings] soundEffects]) {
							//					[appDelegate playSoundWithId:@"Channel"];
							//				}
							[self.myTableView deselectRowAtIndexPath:self.myTableView.indexPathForSelectedRow animated:YES];
						}
					}
				}
			}
		}
	}
	
//	ServerSettings *server = [self.serverList objectAtIndex:indexPath.row];
//	if (self.myServerViewController != nil) {
//		[myServerViewController release];
//	}
//	
//	NSLog(@"Creating the view controller for server: %@\n",[server hostAlias]);
//	NSString *viewControllerName = @"ServerViewController";
//	myServerViewController = [[NSClassFromString(viewControllerName) alloc] initWithNibName:viewControllerName bundle:nil];
//	[myServerViewController setServerSettings:server];
//	
//    [self.navigationController pushViewController:self.myServerViewController animated:YES];
}

#pragma mark -
#pragma mark UIAlertViewDelegate methods

- (void)alertView:(UIAlertView *)alertView willDismissWithButtonIndex:(NSInteger)buttonIndex
{
	[passwordField resignFirstResponder];
	if (buttonIndex != [alertView cancelButtonIndex])
	{
		// User wants to join
		char *pass = [[NSString stringWithFormat:[passwordField text]] cStringUsingEncoding:NSUTF8StringEncoding];
		v3_change_channel(selectedChannel, pass);
	}
	else {
		// User cancelled
	}
	[self.myTableView deselectRowAtIndexPath:self.myTableView.indexPathForSelectedRow animated:YES];

}

#pragma mark -
#pragma mark MBProgressHUDDelegate methods

- (void)hudWasHidden {
    // Remove HUD from screen when HUD is hidden
    [HUD removeFromSuperview];
    [HUD release];
	
	[recordingButton setHidden:NO];
	self.navigationItem.leftBarButtonItem = [[UIBarButtonItem alloc] initWithTitle:@"Disconnect" style:UIBarButtonItemStylePlain target:self action:@selector(disconnectAction)];
	//self.navigationItem.leftBarButtonItem = [[UIBarButtonItem alloc] initWithImage:[UIImage imageNamed:@"disconnect"]  style:UIBarButtonItemStylePlain target:self action:@selector(disconnectAction)];
	self.navigationItem.hidesBackButton = NO;
}

#pragma mark -
#pragma mark Execution code

- (void)connectTask
{
	[self connectToServer];
	while (![eventService finished]) {
		if ([eventService loginComplete]) {
			return;
		}
	}
}

#pragma mark -
#pragma mark API

- (void)connectToServer
{
	if (serverIndex >= 0) {
		EventService *anEventService = [[EventService alloc] initWithServerSettingsIndex:serverIndex];
		[self setEventService:anEventService];
		[anEventService release];
		
		[eventService startService];
	}
//	else {
//		NSLog(@"ServerViewController (connectToServer) >>> No server configured.. please configure a server first.\n");
//	}

	//encDecExample([host cString]);
	//testMain();
}

- (void)disconnectFromServer
{
	//NSLog(@"ServerViewController >>> (disconnectFromServer) >>> entering");
	if (tableOrder != nil) {
		[tableOrder release];
		tableOrder = nil;
	}
    [eventService stopService];
	[recordingButton setHidden:YES];
	while (![eventService finished]) {
		//NSLog(@"ServerViewController >>> (disconnectFromServer) >>> waiting for eventservice to finish");
		[NSThread sleepForTimeInterval:0.1];
	}

	if (eventService != nil) {
		[eventService release];
		eventService = nil;
		//eventService = nil;
	}
	//eventService = nil;
	
}

- (void)addUserWithId:(NSInteger)uid alias:(NSString *)alias channelId:(NSInteger)cid
{
	NSString *userId = [NSString stringWithFormat:@"%d",uid];
	if ([self.userList objectForKey:userId] == nil) {
		
		// User doesn't exist.. so we can add
		if (tableOrder != nil) {
			[self.tableOrder release];
		}
		
		User *user = [[User alloc] initWithUserId:uid alias:alias channelId:cid];
		[self.userList setObject:user forKey:userId];
		//[user release];
		
		
		TableOrder *aTableOrder = [[TableOrder alloc] init];
		self.tableOrder = aTableOrder;
		//[aTableOrder release];
		
		[self.tableOrder createLayoutWithUsers:self.userList Channels:self.channelList];
		
		[myTableView performSelectorOnMainThread:@selector(reloadData) withObject:nil waitUntilDone:NO];
	}
}

- (void)addChannelWithId:(NSInteger)cid name:(NSString *)name parentId:(NSInteger)pid
{
	NSString *channelId = [NSString stringWithFormat:@"%d",cid];
	if ([self.channelList objectForKey:channelId] == nil) {
		// Channel doesn't exist.. so we can add
		if (tableOrder != nil) {
			[self.tableOrder release];
		}
		
		Channel *channel = [[Channel alloc] initWithChannelId:cid name:name parentId:pid];
		[self.channelList setObject:channel forKey:[NSString stringWithFormat:@"%d",cid]];
		//[channel release];
		
		TableOrder *aTableOrder = [[TableOrder alloc] init];
		self.tableOrder = aTableOrder;
		//[aTableOrder release];
		
		[self.tableOrder createLayoutWithUsers:self.userList Channels:self.channelList];
		
		
		[myTableView performSelectorOnMainThread:@selector(reloadData) withObject:nil waitUntilDone:NO];
	}
}

- (void)removeUserWithId:(NSInteger)uid
{
	NSString *userId = [NSString stringWithFormat:@"%d",uid];
	if ([self.userList objectForKey:userId] != nil)
	{
		// Valid user to remove
		if (tableOrder != nil) {
			[self.tableOrder release];
		}
		
		[self.userList removeObjectForKey:userId];
		
		TableOrder *aTableOrder = [[TableOrder alloc] init];
		self.tableOrder = aTableOrder;
		//[aTableOrder release];
		
		[self.tableOrder createLayoutWithUsers:self.userList Channels:self.channelList];
		
		
		[myTableView performSelectorOnMainThread:@selector(reloadData) withObject:nil waitUntilDone:NO];
	}
}

- (void)removeChannelWithId:(NSInteger)cid
{
	NSString *channelId = [NSString stringWithFormat:@"%d",cid];
	if ([self.channelList objectForKey:channelId] != nil)
	{
		// Valid channel to remove
		if (tableOrder != nil) {
			[self.tableOrder release];
		}
		
		[self.channelList removeObjectForKey:channelId];
		
		TableOrder *aTableOrder = [[TableOrder alloc] init];
		self.tableOrder = aTableOrder;
		//[aTableOrder release];
		
		[self.tableOrder createLayoutWithUsers:self.userList Channels:self.channelList];
		
		
		[myTableView performSelectorOnMainThread:@selector(reloadData) withObject:nil waitUntilDone:NO];
	}
}

- (void)moveUser:(NSInteger)uid toChannel:(NSInteger)cid
{
	NSString *userId = [NSString stringWithFormat:@"%d",uid];
	NSString *channelId = [NSString stringWithFormat:@"%d",cid];
	if (cid != 0) {
		if ([self.channelList objectForKey:channelId] == nil) {
			// Channel doesn't exist and should..
			return;
		}
	}
	
	if ( [self.userList objectForKey:userId] != nil )
	{
		// Valid user and channel to move to
		if (tableOrder != nil) {
			[self.tableOrder release];
		}
		
		User *currentUser = [self.userList objectForKey:userId];
		[currentUser setChannelId:cid];

		TableOrder *aTableOrder = [[TableOrder alloc] init];
		self.tableOrder = aTableOrder;
		//[aTableOrder release];
		
		[self.tableOrder createLayoutWithUsers:self.userList Channels:self.channelList];
		
		
		[myTableView performSelectorOnMainThread:@selector(reloadData) withObject:nil waitUntilDone:NO];
	}
}

- (uint16_t)channelIdForUser:(uint16_t)uid
{
	uint16_t cid = -1;
	
	NSString *userId = [NSString stringWithFormat:@"%u",uid];
	User *theUser = (User *)[self.userList objectForKey:userId];
	if (theUser != nil) {
		// User exists
		cid = (uint16_t)[theUser channelId];
	}
	
	return cid;
	
}

- (User *)userForUserId:(NSInteger)uid
{
	NSString *userId = [NSString stringWithFormat:@"%u",uid];
	return (User *)[self.userList objectForKey:userId];
}

- (void)userTransmitting:(NSInteger)uid Status:(BOOL)isTransmit
{
	NSString *userId = [NSString stringWithFormat:@"%d",uid];
	User *currentUser = [self.userList objectForKey:userId];
	if (currentUser != nil)
	{
		[currentUser setTransmitting:isTransmit];
		
		[myTableView performSelectorOnMainThread:@selector(reloadData) withObject:nil waitUntilDone:NO];
	}
}

- (IBAction)startRecording:(id)sender
{
	if (recordingToggle) {
		if (isRecording) {
			UIButton *button = (UIButton *)sender;
			button.selected = NO;
			isRecording = NO;
			[recordingBackground setHidden:YES];
			[eventService stopRecording];
		}
		else {
			isRecording = YES;
			[recordingBackground setHidden:NO];
			[eventService startRecording];
		}

	}
	else {
		[recordingBackground setHidden:NO];
		[eventService startRecording];
	}
}

- (IBAction)stopRecording:(id)sender
{
	if (recordingToggle) {
		UIButton *button = (UIButton *)sender;
		button.selected = isRecording;
		button.highlighted = !isRecording;
	}
	else {
		[recordingBackground setHidden:YES];
		[eventService stopRecording];
	}
}

- (IBAction)showInfo
{
//	//NSLog(@"User wants to see info about this server...\n");
//	[(VentafoneAppDelegate *)[[UIApplication sharedApplication] delegate] playSoundWithId:@"connect"];
//	if (tableOrder != nil) {
//		[self.tableOrder printLayout];
//	}
	
	NSLog(@"Muting user 100");
	uint16_t theId = 135;
	int theLevel = 100;
	v3_set_volume_user(theId, theLevel);
}

- (IBAction)disconnectAction
{
	if (connected) {
		// User presses "back" button
		[appDelegate disconnectWithHUDDisplay];
		connected = NO;
	}
	[self.navigationController popToRootViewControllerAnimated:YES];
}

- (void)reloadData
{
	if (tableOrder != nil) {
		[self.tableOrder release];
	}
	TableOrder *aTableOrder = [[TableOrder alloc] init];
	self.tableOrder = aTableOrder;
	[self.tableOrder createLayoutWithUsers:self.userList Channels:self.channelList];
	
	[self.myTableView reloadData];
}

@end
